#include "ACF_State.h"

// #define DEBUG_STATE

/*
 * ABSTRACT STATE
 */      
EventSet AbstractState::acceptedUserEvents() {
  // default implementation: delegate to containing state (if any):
  if(containingState != NULL) { 
    return containingState->acceptedUserEvents();
  } else {
    return EVENT_SET_NONE;
  }
}

EventSet AbstractState::eval() {
  // override this to add more non-user-triggered events:
  return acceptedUserEvents();
}

StateID AbstractState::transAction(Event) {
  // default implementation: no transition target defined
  return STATE_UNDEFINED;
}

void AbstractState::entryAction() {
  // default implementation: empty
}

void AbstractState::exitAction(){
  // default implementation: empty
}


/*
 * SIMPLE STATE
 */
StateID AbstractSimpleState::enter() {
  entryAction();
  return id();
}
        
StateID AbstractSimpleState::trans(const Event event) {
  StateID next = transAction(event);
  if (next == STATE_UNDEFINED && containingState != NULL) {
    next = containingState->trans(event);
  }
  if (next != STATE_SAME && next != this->id() && next != STATE_UNDEFINED) {
    exit(event, next);
  }
  return next;
}

void AbstractSimpleState::exit(const Event event, const StateID next) {
  exitAction();
  if(containingState != NULL) { 
    containingState->exit(event, next);
  }
}

/*
 * COMPOSITE STATE
 */
void AbstractCompositeState::setSubstates(AbstractState **substates, uint8_t numSubstates) {
  this->substates = substates;
  this->numSubstates = numSubstates;
  // set containingStates of the subtates to "this":
  for(uint16_t i=0; i< this->numSubstates; i++) {
    this->substates[i]->containingState = this;
  }
}

AbstractState *AbstractCompositeState::initialSubstate() {
  return substates[0];
}
      
StateID AbstractCompositeState::enter() {
  entryAction();
  return initialSubstate()->enter();
}

StateID AbstractCompositeState::trans(const Event event) {
  StateID next = transAction(event);
  if (next == STATE_UNDEFINED && containingState != NULL) {
    next = containingState->trans(event);
  }
  return next;
  // Do not invoke the exit action here: exits are performed "up" the containment hiearchy.
}

void AbstractCompositeState::exit(const Event event, const StateID next) {
  // if the transition occurs within the substates of this containing state, then we will not exit this containing state
  // and neither its containing states:
  for(uint16_t i=0; i< numSubstates; i++) {
    if (substates[i]->id() == next) {
      return;
    }
  }
  exitAction();
  if(containingState != NULL) { 
    containingState->exit(event, next);
  }
}


/*
 * STATE AUTOMATON
 */
 
void AbstractStateAutomaton::setStates(AbstractState **states, uint8_t numStates) {
  this->states = states;
  this->numStates = numStates;
  if (numStates > 0) {
    currentState = states[0];
  }
}

EventSet AbstractStateAutomaton::acceptedUserEvents() {
  return currentState->acceptedUserEvents();
}
  
EventSet AbstractStateAutomaton::evaluate() {
  EventSet candidates = currentState->eval();
  #ifdef DEBUG_STATE
    Serial.print(F("DEBUG_STATE: eval in state "));
    Serial.print(currentState->id().id());
    Serial.print(F(": candidates = "));
    Serial.println(candidates.events(), HEX);
  #endif
  return candidates;
}

void AbstractStateAutomaton::transition(Event event) {
  #ifdef DEBUG_STATE
    Serial.print(F("DEBUG_STATE: State "));
    Serial.print(currentState->id().id());
    Serial.print(F(": process event 0x"));
    Serial.println(event.id(), HEX);
  #endif
  StateID oldStateID = currentState->id();
  StateID newStateID = currentState->trans(event);
  if (newStateID == STATE_UNDEFINED) {
    if (! (currentState->illegalTransitionLogged & event)) { // this illegal event has not been logged at this state
      #ifdef DEBUG_STATE
        Serial.print(F("DEBUG_STATE: State "));
        Serial.print(currentState->id().id());
        Serial.print(F(": log invalid event 0x"));
        Serial.println(event.id(), HEX);
      #endif

      if (log != NULL) {
        log->logMessage(static_cast<uint8_t>(StateMessageEnum::MSG_ILLEGAL_TRANS), currentState->id().id(), event.id());
      }
      currentState->illegalTransitionLogged |= event;
    }
    
  } else if (newStateID != STATE_SAME && oldStateID != newStateID) { // we are in a different new state!
    currentState = state(newStateID);
    
    // Enter the new state (which can be a composite state but will always end up in a simple state):
    newStateID = currentState->enter();
    currentState = state(newStateID);
    
    stateChanged(oldStateID, event, newStateID);
    
    #ifdef DEBUG_STATE
      Serial.print(("DEBUG_STATE: New state: "));
      Serial.println(currentState->id().id());
    #endif
  } else {
    #ifdef DEBUG_STATE
      Serial.print(("DEBUG_STATE: Same state: "));
      Serial.println(currentState->id().id());
    #endif
  }
}

AbstractState *AbstractStateAutomaton::state(StateID id) {
  for(uint8_t i=0; i<numStates; i++) {
    if (states[i]->id() == id) {
      return states[i];
    }
  }
  log->log_S_O_S(static_cast<uint8_t>(StateMessageEnum::MSG_UNKNOWN_STATE), id.id(), 0);  // function NEVER RETURNS
  abort();
}

void AbstractStateAutomaton::stateChanged(StateID, Event, StateID) {
  // do nothing
}
