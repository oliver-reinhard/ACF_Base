#include "DemoStates.h”

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  // Print invocation of entry and exit actions during transitions
  ExecutionContext context = ExecutionContext();
  TestAutomaton automaton = TestAutomaton();
  automaton.init(&context);

  // Print initial state:
  Serial.print("Initial state: ");
  Serial.println(automaton.state()->id().name());

  // A -> B (-> ends up at initial substate C )
  automaton.transition(EVENT_A_B); 
  
  // B -> A (actually C -> A)
  automaton.transition(EVENT_B_A); 

  // walk along A -> B again (-> ends up at initial substate C )
  automaton.transition(EVENT_A_B);
  
  // C -> C
  automaton.transition(EVENT_C_C); 
  
  // C -> D
  automaton.transition(EVENT_C_D); 
  
  // D -> E (-> also leaving super state B)
  automaton.transition(EVENT_D_E); 

}

void loop() {
  // do nothing
}
