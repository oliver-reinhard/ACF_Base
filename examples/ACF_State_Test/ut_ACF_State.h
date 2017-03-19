#ifndef UT_ACF_STATE_H_INCLUDED
  #define UT_ACF_STATE_H_INCLUDED
  
  #include <ACF_State.h>

  class MockExecutionContext {
    public:
      bool cond = false;
      uint16_t entryA = 0;
      uint16_t exitA  = 0;
      uint16_t entryB = 0;
      uint16_t exitB  = 0;
      uint16_t entryC = 0;
      uint16_t exitC  = 0;
      uint16_t entryD = 0;
      uint16_t exitD  = 0;
      uint16_t entryE = 0;
      uint16_t exitE  = 0;
      uint16_t transAction_A_B  = 0;
      uint16_t transAction_C_C  = 0;
  
      /* Example of a transition action executing a function in the context. */
      void trans_A_B() { transAction_A_B++; }
      void trans_C_C() { transAction_C_C++; }
      
      void reset() {
        cond = false;
        entryA = 0;
        exitA  = 0;
        entryB = 0;
        exitB  = 0;
        entryC = 0;
        exitC  = 0;
        entryD = 0;
        exitD  = 0;
        entryE = 0;
        exitE  = 0;
        transAction_A_B = 0;
        transAction_C_C = 0;
      }
  
      uint16_t totalInvocations() {
        return 
          entryA +
          exitA  +
          entryB +
          exitB  +
          entryC +
          exitC  +
          entryD +
          exitD  +
          entryE +
          exitE  +
          transAction_A_B +
          transAction_C_C;
      }
  };
  
  class ContextAware {
    public:
      void setContext(MockExecutionContext *context) { this->context = context; }
    protected:
      MockExecutionContext *context;
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
  
  static const StateID STATE_A = StateID(1);
  static const StateID STATE_B = StateID(2);
  static const StateID STATE_C = StateID(3);
  static const StateID STATE_D = StateID(4);
  static const StateID STATE_E = StateID(5);
  
  static const uint8_t NUM_STATES = 5;  
  
  static const Event EVENT_A_B = Event(0x1);
  static const Event EVENT_B_A = Event(0x2);
  static const Event EVENT_C_C = Event(0x4);
  static const Event EVENT_C_D = Event(0x8);
  static const Event EVENT_D_E = Event(0x10);
  
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
      void entryAction() { context->entryA++; }
      void exitAction()  { context->exitA++; }
  };
  
  class StateB : public AbstractCompositeState, public ContextAware {
    public:
      StateID id() { return STATE_B; }
      EventSet acceptedUserEvents() { return EventSet(EVENT_B_A); }
      StateID transAction(const Event event) {
        if (event == EVENT_B_A) { return STATE_A; }
        return AbstractState::transAction(event);
      }
      void entryAction() { context->entryB++; }
      void exitAction()  { context->exitB++; }
  };
  
  class StateC : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return STATE_C; }
      EventSet acceptedUserEvents() { return AbstractState::acceptedUserEvents() | EVENT_C_C | EVENT_C_D; }
      StateID transAction(const Event event) {
        if (event == EVENT_C_C) {
          context->trans_C_C();  // example of a transition action
          return STATE_C;
        }
        if (event == EVENT_C_D) { return STATE_D; }
        return AbstractState::transAction(event);
      }
      void entryAction() { context->entryC++; }
      void exitAction()  { context->exitC++; }
  };
  
  class StateD : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return STATE_D; }
      EventSet acceptedUserEvents() { return AbstractState::acceptedUserEvents() | EVENT_D_E; }
      StateID transAction(const Event event) {
        if (event == EVENT_D_E) { return STATE_E; }
        return AbstractState::transAction(event);
      }
      void entryAction() { context->entryD++; }
      void exitAction()  { context->exitD++; }
  };
  
  class StateE : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return STATE_E; }
      EventSet acceptedUserEvents() { return EventSet(EVENT_NONE); }
      StateID transAction(const Event event) {
        return AbstractState::transAction(event);
      }
      void entryAction() { context->entryE++; }
      void exitAction()  { context->exitE++; }
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
      void init(MockExecutionContext *context) {
        setStates(ALL_STATES, NUM_STATES);
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
