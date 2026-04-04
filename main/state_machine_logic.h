#ifndef STATE_MACHINE_LOGIC_H
#define STATE_MACHINE_LOGIC_H

typedef enum {
    SM_IDLE = 0,
    SM_WAIT_Y,
    SM_DONE
} sm_state_t;

typedef enum {
    EV_X = 1,
    EV_Y = 2,
    EV_RESET = 3
} sm_event_t;


sm_state_t sm_process_event(sm_state_t state, sm_event_t event);

#endif
