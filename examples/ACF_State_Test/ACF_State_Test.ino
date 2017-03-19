#include <ArduinoUnitX.h>
#include "ut_ACF_State.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
  // Test::exclude("*");
  // Test::include("e*");
  //Test::exclude("z_s_o_s");
}

void loop() {
  Test::run();
}


// ------ Unit Tests --------

test(a_state) {
  assertTrue (STATE_A == STATE_A);
  assertFalse(STATE_A == STATE_B);
  assertTrue (STATE_A != STATE_B);
}

test(b_event) {
  assertTrue (EVENT_A_B == EVENT_A_B);
  assertFalse(EVENT_A_B == EVENT_B_A);
  assertTrue (EVENT_A_B != EVENT_B_A);
}

test(c_event_set) {
  EventSet es = EventSet();
  es |= EVENT_A_B; // add event
  assertTrue (es.events() == EVENT_A_B.id());
  es |= EVENT_B_A; // add event
  assertEqual(es.events(), EVENT_A_B.id() | EVENT_B_A.id());

  assertTrue (es & EVENT_A_B);
  assertFalse(es & EVENT_C_D);
  
  EventSet es1 = EventSet();
  es1 |= EVENT_A_B; // add event
  assertFalse(es == es1);
  assertTrue (es != es1);
  es1 |= EVENT_B_A; // add event
  assertTrue (es == es1);
  
  EventSet es2 = EventSet(EVENT_A_B);
  EventSet res = es2 | EVENT_C_D | EVENT_D_E;
  assertEqual(res.events(), EVENT_A_B.id() | EVENT_C_D.id() | EVENT_D_E.id());

  EventSet es3 = EventSet(EVENT_A_B);
  EventSet es4 = EventSet(EVENT_C_D);
  es3 |= es4; // add event set
  assertEqual(es3.events(), EVENT_A_B.id() | EVENT_C_D.id());
}

test(d_states_events) {
  MockExecutionContext context = MockExecutionContext();
  StateA a = StateA();
  assertEqual(a.id().id(), STATE_A.id());
  
  StateB b = StateB();  // composite
  assertEqual(b.id().id(), STATE_B.id());
  
  StateC c = StateC();
  assertEqual(c.id().id(), STATE_C.id());
  
  StateD d = StateD();
  assertEqual(d.id().id(), STATE_D.id());
  
  StateE e = StateE();
  assertEqual(e.id().id(), STATE_E.id());

  // Set context:
  a.setContext(&context);
  b.setContext(&context);
  c.setContext(&context);
  d.setContext(&context);
  e.setContext(&context);
  
  // Create composite state:
  AbstractState *B_SUBSTATES[2] = {&c, &d};
  b.setSubstates(B_SUBSTATES, 2);

  // Check accepted events:
  EventSet aes = a.acceptedUserEvents();
  assertEqual(aes.events(), EVENT_A_B.id());
  aes = a.eval(EVENT_A_B);
  assertEqual(aes.events(), EVENT_A_B.id());
  
  EventSet bes = b.acceptedUserEvents();
  assertEqual(bes.events(), EVENT_B_A.id());
  
  EventSet ces = c.acceptedUserEvents();
  assertEqual(ces.events(), EVENT_B_A.id() | EVENT_C_C.id() | EVENT_C_D.id());
  ces = c.eval(EVENT_C_C);
  assertEqual(ces.events(), EVENT_C_C.id());
  
  EventSet des = d.acceptedUserEvents();
  assertEqual(des.events(), EVENT_B_A.id() | EVENT_D_E.id());

  EventSet ees = e.acceptedUserEvents();
  assertEqual(ees.events(), EVENT_NONE.id());
}

test(e_entry_exit) {
  // Check invocation of entry and exit actions during transitions
  MockExecutionContext context = MockExecutionContext();
  TestAutomaton automaton = TestAutomaton();
  automaton.init(&context);

  // check initial state:
  assertEqual(automaton.state()->id().id(), STATE_A.id());

  // A -> B (-> ends up at initial substate C )
  context.reset();
  assertEqual(automaton.state()->illegalTransitionLogged.events(), 0);
  automaton.transition(EVENT_A_B); 
  assertEqual(automaton.state()->id().id(), STATE_C.id());
  assertEqual(automaton.state()->illegalTransitionLogged.events(), 0);
  assertEqual(context.exitA, 1);
  assertEqual(context.transAction_A_B, 1);
  assertEqual(context.entryB, 1);
  assertEqual(context.entryC, 1);
  assertEqual(context.totalInvocations(), 4);
  
  // B -> A (actually C -> A)
  context.reset();
  automaton.transition(EVENT_B_A); 
  assertEqual(automaton.state()->id().id(), STATE_A.id());
  assertEqual(context.exitC, 1);;
  assertEqual(context.exitB, 1);
  assertEqual(context.entryA, 1);
  assertEqual(context.totalInvocations(), 3);

  // walk along A -> B again (-> ends up at initial substate C )
  automaton.transition(EVENT_A_B);
  assertEqual(automaton.state()->id().id(), STATE_C.id());
  
  // C -> C
  context.reset();
  automaton.transition(EVENT_C_C); 
  assertEqual(automaton.state()->id().id(), STATE_C.id());
  assertEqual(context.transAction_C_C, 1);
  assertEqual(context.totalInvocations(), 1);
  
  // C -> D
  context.reset();
  automaton.transition(EVENT_C_D); 
  assertEqual(automaton.state()->id().id(), STATE_D.id());
  assertEqual(context.exitC, 1);
  assertEqual(context.entryD, 1);
  assertEqual(context.totalInvocations(), 2);
  
  // D -> E (-> also leaving super state B)
  context.reset();
  automaton.transition(EVENT_D_E); 
  assertEqual(automaton.state()->id().id(), STATE_E.id());
  assertEqual(context.exitD, 1);;
  assertEqual(context.exitB, 1);
  assertEqual(context.entryE, 1);
  assertEqual(context.totalInvocations(), 3);
}

test(f_illegal_trans) {
  // Check invocation of entry and exit actions during transitions
  MockExecutionContext context = MockExecutionContext();
  TestAutomaton automaton = TestAutomaton();
  automaton.init(&context);
  assertEqual(automaton.state()->id().id(), STATE_A.id());  // initial state
  
  assertEqual(automaton.state()->illegalTransitionLogged.events(), 0);
  automaton.transition(EVENT_C_D);
  assertEqual(automaton.state()->id().id(), STATE_A.id());
  assertEqual(automaton.state()->illegalTransitionLogged.events(), EVENT_C_D.id());
}
