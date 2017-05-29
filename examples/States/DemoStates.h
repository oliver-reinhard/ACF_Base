#ifndef DEMO_STATES_H_INCLUDED
#define DEMO_STATES_H_INCLUDED

#include <ACF_State.h>

class ExecutionContext {
public:
	bool cond = false;
	
	/* Example of a transition action executing a function in the context. */
	void trans_A_B() { Serial.println("transAction: A->B"); }
	void trans_C_C() { Serial.println("transAction: C->C"); }
	void enter(AbstractState *s) {
		Serial.print("entering ");
		Serial.println(s->id().name());
	}
	void exit(AbstractState *s) {
		Serial.print("exiting ");
		Serial.println(s->id().name());
	}
	
	
};

class ContextAware {
public:
	void setContext(ExecutionContext *context) { this->context = context; }
protected:
	ExecutionContext *context;
};

//                +------------------------+
//     +---+  ta  |      +---+   B  +---+  |    +---+
//     |   +----->|      |   |      |   |  |    |   |
// o-->| A |      |  o-->| C +----->| D +------>| E |
//     |   |<-----+      |   |<-+   |   |  |    |   |
//     +---+   *  |      +-+-+  |   +---+  |    +---+
//                |        |    |          |
//                |        +----+          |
//                |          ta            |
//                +------------------------+
//
// o--> Initial state
// ta = transition action
// *  = semantics of this transition are that every stated contained by B implicitly includes this transition option
// Note: the transition C-->C is timeout-triggered rather than user-triggered

static const StateID STATE_A = StateID(1, "A");
static const StateID STATE_B = StateID(2, "B");
static const StateID STATE_C = StateID(3, "C");
static const StateID STATE_D = StateID(4, "D");
static const StateID STATE_E = StateID(5, "E");

static const uint8_t NUM_STATES = 5;

static const Event EVENT_A_B = Event(0x1);
static const Event EVENT_B_A = Event(0x2);
static const Event EVENT_C_C = Event(0x4); // not a user-triggerable event (timout-triggered)
static const Event EVENT_C_D = Event(0x8);
static const Event EVENT_D_E = Event(0x10);

static const TimeMillis C_C_TIMEOUT = 2000L;

class StateA : public AbstractSimpleState, public ContextAware {
public:
	StateID id() { return STATE_A; }
	EventSet acceptedUserEvents() { return EventSet(EVENT_A_B); }
	StateID transAction(const Event event) {
		if (event == EVENT_A_B) {
			context->trans_A_B();  // example of a transition action
			return STATE_B;
		}
		return AbstractState::transAction(event);
	}
	void entryAction() { context->enter(this); }
	void exitAction()  { context->exit(this); }
};

class StateB : public AbstractCompositeState, public ContextAware {
public:
	StateID id() { return STATE_B; }
	EventSet acceptedUserEvents() { return EventSet(EVENT_B_A); }
	StateID transAction(const Event event) {
		if (event == EVENT_B_A) { return STATE_A; }
		return AbstractState::transAction(event);
	}
	void entryAction() { context->enter(this); }
	void exitAction()  { context->exit(this); }
};


class StateC : public AbstractSimpleState, public ContextAware {
public:
	StateID id() { return STATE_C; }
	EventSet acceptedUserEvents() { return AbstractState::acceptedUserEvents() | EVENT_C_D; }
	
	EventSet eval(const TimeMillis timeInState, const Event userRequest = EVENT_NONE) {
		EventSet result = AbstractState::eval(timeInState, userRequest); // handles user-requested events
		if (timeInState >= C_C_TIMEOUT) {
			result |= EVENT_C_C;
		}
		return result;
	}
	
	StateID transAction(const Event event) {
		if (event == EVENT_C_C) {
			context->trans_C_C();  // example of a transition action
			return STATE_C;
		}
		if (event == EVENT_C_D) { return STATE_D; }
		return AbstractState::transAction(event);
	}
	
	void entryAction() { context->enter(this); }
	void exitAction()  { context->exit(this); }
};

class StateD : public AbstractSimpleState, public ContextAware {
public:
	StateID id() { return STATE_D; }
	EventSet acceptedUserEvents() { return AbstractState::acceptedUserEvents() | EVENT_D_E; }
	StateID transAction(const Event event) {
		if (event == EVENT_D_E) { return STATE_E; }
		return AbstractState::transAction(event);
	}
	void entryAction() { context->enter(this); }
	void exitAction()  { context->exit(this); }
};

class StateE : public AbstractSimpleState, public ContextAware {
public:
	StateID id() { return STATE_E; }
	EventSet acceptedUserEvents() { return EventSet(EVENT_NONE); }
	StateID transAction(const Event event) {
		return AbstractState::transAction(event);
	}
	void entryAction() { context->enter(this); }
	void exitAction()  { context->exit(this); }
};


class TestAutomaton : public AbstractStateAutomaton,  public ContextAware {
private:
	StateA a = StateA();
	StateB b = StateB();  // composite
	StateC c = StateC();
	StateD d = StateD();
	StateE e = StateE();
	AbstractState *B_SUBSTATES[2] = {&c, &d};
	AbstractState *ALL_STATES[NUM_STATES] = {&a, &b, &c, &d, &e};
	
public:
	void init(ExecutionContext *context) {
		setStates(ALL_STATES, NUM_STATES, &a);
		// no log set
		
		setContext(context);
		a.setContext(context);
		b.setContext(context);
		c.setContext(context);
		d.setContext(context);
		e.setContext(context);
		
		b.setSubstates(B_SUBSTATES, 2);
		currentState = &a;  // just a example: is the default anyway
	}
};
#endif
