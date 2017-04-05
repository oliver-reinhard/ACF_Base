#ifndef ACF_STATE_H_INCLUDED
  #define ACF_STATE_H_INCLUDED

  #include <Arduino.h>
  #include <ACF_Types.h>
  #include <ACF_Logging.h>

  /* Base type for state serialisation. */
  typedef int8_t T_State_ID;

  /* Base type for state serialisation. */
  typedef uint32_t T_Event_ID;

  static const char UNNAMED[] = "";


  /*
   * Immutable ans serializable object to identify states independent of actual state-implementation classes.
   */
  class StateID {
    public:
	  
      /*
       * Constructor.
       *
       * @param id unique identifier. Negative values are reserved by framework.
       * @param name optional (pass NULL).
       */
      StateID(const T_State_ID id, const char *name) {
        this->value = id;
        this->str = name;
      }
      
      StateID(const T_State_ID id) : StateID(id, NULL) {}
      
      T_State_ID id() const { return value; }
      const char *name() const { return (str == NULL) ? UNNAMED : str; }
	  
      bool operator ==(const StateID other) const { return value == other.value; }
      bool operator !=(const StateID other) const { return value != other.value; }
      
    protected:
      T_State_ID value;
      const char *str;
  };

  /* Pseudo state: used to identify illegal transition events. */
  static const StateID STATE_UNDEFINED = StateID(-2, "Undefined");
  
  /* Pseudo state: use this in AbstractState::transAction() implementations to make transition definitions more intuitive to read. */
  static const StateID STATE_SAME = StateID(-1, "(same)");


  /*
   * Immutable object to identify transition events between states.
   * 
   * Note: Event IDs are represented as single bits, i.e. powers of 2: 0, 1, 2, 4, 8 (0x0, 0x1, 0x2, 0x4, 0x8, 0x10, etc.)
   */
  class Event {
    public:
	  /*
	   * Constructor.
	   *
	   * @param id ids of events are powers of 2, i.e. 0, 1, 2, 3, 4
	   * @param name optional (pass NULL)
	   */
	  Event(const T_Event_ID id, const char *name) {
        this->value = id;
        this->str = name;
	  }
	  
	  Event(const T_Event_ID id) : Event(id, NULL) {}
	  
	  T_Event_ID id() const { return value; }
	  const char *name() const { return (str == NULL) ? UNNAMED : str; }
	  
      bool operator ==(const Event other) const { return value == other.value; }
      bool operator !=(const Event other) const { return value != other.value; }
      
    protected:
	  T_Event_ID value;
      const char *str;
  };

  /* Pseudo event: used to indicate that no (user-triggered) events are available from a certain state. */
  static const Event EVENT_NONE = Event(0L, "None");


  /*
   * Mutable object to collect supported (user-triggered) event options.
   */
  class EventSet {
    public:
      EventSet() : EventSet(EVENT_NONE) { }
      EventSet(const Event event) { this->value = event.id(); }
      EventSet(T_Event_ID events) { this->value = events; }
	  
	  /* Returns the events in the set or'ed together ("|"). */
      T_Event_ID events() const { return value; }
      
      void clear() { value = EVENT_NONE.id(); }
      
      /* Add an event to the set. */
      void operator |=(const Event event) { value |= event.id(); }
      
      /* Add an event set to the set. */
      void operator |=(const EventSet events) { value |= events.events(); }
      
      /* Check whether an event is part of the set. */
      bool operator &(const Event event) { return value & event.id(); } 
      
      bool operator ==(const EventSet other) const { return value == other.value; }
      bool operator !=(const EventSet other) const { return value != other.value; }

      /*
       * Enables chaining of expressions like:
       * EventSet es; Event a; Event b;
       *   return es | a | b;
       */
      EventSet operator |(const Event event) { return EventSet(value |= event.id()); }
      
     private:
	   /* Stores the events in the set or'ed together ("|"). */
       T_Event_ID value;
  };
  
  static const EventSet EVENT_SET_NONE = EventSet(EVENT_NONE);

  
  /*
   * The mother of all states. 
   *
   * Note 1: Subclass AbstractSimpleState and AbstractComplexState rather than this class.
   * Note 2: All state classes are stateless in that they do not store any values during or after state changes or as a result of executing actions.
   */
  class AbstractState {
    public:
    
      /* The containing state, or NULL if none.  */
      AbstractState *containingState = NULL;
      
      /*
       * Records whether an illegal transition has been logged at this state in order to avoid excessive logging.
       * Note: the presence of an event bit in the set EventCandidates means the attempt has been logged and will not be logged again
       * at this state for the lifetime of the state object.
       */
      EventSet illegalTransitionLogged = EventSet(EVENT_NONE);

      /*
       * Constructor.
       */
      AbstractState() { }

      /* @return ID of the state. */
      virtual StateID id() = 0;

      /*
       * Calculates and returns the set of user-initiated events supported by the current state at the time of invocation of this method.
       * 
       * The default implementation invokes the containing state's acceptedUserEvents() method first, then adds its own events to the result.
       *
       * Override this method to support (i.e. return) user-initiated events.
       * 
       * @return EVENT_SET_NONE if there should be no user-triggered events available at the time.
       */
      virtual EventSet acceptedUserEvents();
      
      /*
       * Evaluates which event should be the next to trigger a transition (see trans(event)).
       *
       * If a user-triggered event is provided, then it will be matched against the acceptedUserEvents() at this time. The method also
       * checks all the event conditions for automaton-triggered ("automatic") transitions (non-user-triggered) and it may favour the
       * user request over automatic transitions or vice versa.
       * If no user-triggered event is provided, only automatic transitions are considered. However, the default implementation does
       * not check for any automatic transitions.
       *
       * Note : this method does not actually perform a transition, it merely calculates the options for transitioning. It is  left to the caller
       * to pick the event with the highest priority by his own definition and trigger the transition.
       *
       * Override this method to consider automatic (i.e. non-user triggered) events.
       * 
       * @param  timeInState time [ms] spent so far in this state; use to trigger timeout events
       * @param  userRequest optional. If a userRequest is passed then it will be matched against currently available user-triggered events.
       * @return EVENT_SET_NONE if there should not be a state change.
       */
      virtual EventSet eval(const TimeMillis timeInState, const Event userRequest = EVENT_NONE);
    
      /*
       * Handels the event and executes the transistion. If this state can't handle the event it delegates to the containing state's trans() method.
       * Handling the event consists of invoking its transAction() method (which may execute commands and computes the next state).
       * Note: this method stays at the same state or transitions *out* of it but does *not yet enter* the next state.
       * 
       * @return STATE_UNDEFINED if event wasn't handled, returns id() or STATE_SAME if current state does not change
       */
      virtual StateID trans(const Event event) = 0;
    
      /*
       * Enters a state by invoking its entryAction() method, then triggers the enter() method of its initial substate (if any).
       * Note 1: Enter methods are always invoked *down* the containment chain, i.e. containing state first, then its substates until a simple state is reached.
       * Note 2: You would normally override entryAction() rather than enter().
       * Note 3: This method is typically invoked by the containing state automaton, not by the programmer.
       *
       * @return the actual new state, which is always a simple state; if "this" is a composite state, then the new state is its initial substate and so on.
       */
      virtual StateID enter() = 0;
    
      /*
       * Performes the exit tasks of "this", then invokes the containing state's exit method.
       * Note 1: exit methods are always invoked *up* the containment chain, i.e. simple state first, then its containing state(s).
       * Note 2: You would normally override exitAction() rather than exit().
       * Note 3: This method is typically invoked by the containing state itself or by the state automaton, not by the programmer.
       */
      virtual void exit(const Event event, const StateID next) = 0;
  
    protected:
          
      /*
       * Executes the transition actions for this event (if any), calculates and returns the next state.
       * 
       * @return STATE_UNDEFINED if event couldn't be handled at this level.
       */
      virtual StateID transAction(const Event event);

      /* Executes the action(s) that are always performed when this state is entered. */
      virtual void entryAction();
      
      /* Executes the action(s) that are always performed when this state is truly exited. */
      virtual void exitAction();
  };


  /*
   * Simple states do not have substates.
   */
  class AbstractSimpleState : public AbstractState {
    public:
      AbstractSimpleState() : AbstractState() { };
      
      virtual StateID enter();
      
      /* If the next state is different from "this" then the exit() method is invoked, which may directly trigger exit() at containing states. */
      virtual StateID trans(const Event event);

    protected:
      virtual void exit(const Event event, const StateID next);
  };

  
  /*
   * Complex states have substates. Nesting can be arbitrarily deep, mixing simple and complex states as childern is possible.
   */
  class AbstractCompositeState : public AbstractState {
    public:
      AbstractCompositeState()  : AbstractState() { }
    
      /*
       * @param substates an array of size numSubstates. The first element is the initial substate.
       * @param numSubstates must be > 0
       */
      void setSubstates(AbstractState **substates, uint8_t numSubstates);
    
      /* Only invoke after invocation of setSubstates(). */
      AbstractState *initialSubstate() { return substates[0]; }
    
      /* Override. */
      virtual StateID enter();
      
      /* Does not invoke the exit action (exits are triggered by simple states and are then performed "up" the containment hiearchy). */
      virtual StateID trans(const Event event);
      
    protected:
    
      AbstractState **substates;
      uint16_t numSubstates;
      
      /* If the next state is a simple state not contained by "this" then the exitAction() method is invoked (else we won't leave "this"). */      
      virtual void exit(const Event event, const StateID next);
         
  };
  
  /*
   * IDs for log messages posted by AbstractStateAutomaton.
   */
  enum class StateMessageEnum : uint8_t {
    MSG_ILLEGAL_TRANS = 30,   // State [state]: illegal transition attemt (event [event])
    MSG_UNKNOWN_STATE = 31    // State [state] has not been defined
  };
  
  /*
   * STATE AUTOMATON
   */
  class AbstractStateAutomaton {
    public:
      
      /* Returns the current state. */
      AbstractState *state() { return currentState; }
    
      /* Returns the timepoint ([ms] as returned by millis()) when the automaton transitioned to the current state. */
      TimeMillis stateStartMillis() { return currentStateStartMillis; }
    
    
      /* Returns the time [ms] spent so far at the current state. */
      TimeMillis inStateMillis() { return millis() - currentStateStartMillis; }

      /*
       * Adds the states to the automaton.
       * @param states an array of size numStates
       * @param numStates must be > 0
       * @param initial defines the initial state of the automaton. If NULL is passed, then the first element of passed states becomes the initial state.
       */
      virtual void setStates(AbstractState **states, uint8_t numStates, AbstractState *initial = NULL);

      /* Optional invocation. Automaton can handle NULL log. */
      void setLog(AbstractLog *log) { this->log = log; }

      /* Returns the result of acceptedUserEvents() of the current state. */
      EventSet acceptedUserEvents();
    
      
      /* Returns the result of eval() of the current state. */
      virtual EventSet evaluate(const Event userRequest = EVENT_NONE);
    
      /*
       * Execute trans(event) on the current state and enters the new state, if there is a transition to a new state at all.
       * Note: executes all entry, exit and transition actions as defined and appropriate.
       */
      virtual void transition(const Event event);
    
    protected:
      AbstractState **states;
      uint8_t numStates;
      AbstractState *currentState;
      /* Timepoint [ms] of most recent transition to current state.  */
      TimeMillis currentStateStartMillis = 0L;
      AbstractLog *log = NULL;

      /* Maps ids to real states. */
      virtual AbstractState *state(const StateID id);

      /* Override: use for logging, time-tracking, notifications, etc. */
      virtual void stateChanged(const StateID fromState, const Event event, const StateID toState);
  };
#endif
