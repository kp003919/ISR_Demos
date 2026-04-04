#include "state_machine_logic.h"

sm_state_t sm_process_event(sm_state_t state, sm_event_t event)
{
    // Global transition: RESET always returns to IDLE
    if (event == EV_RESET)
        return SM_IDLE;

    switch (state) {

        case SM_IDLE:
            if (event == EV_X)
                return SM_WAIT_Y;
            break;

        case SM_WAIT_Y:
            if (event == EV_Y)
                return SM_DONE;
            break;

        case SM_DONE:
            // No transitions except RESET (handled above)
            break;
    }

    return state;
}
