#include "fsm.h"
#define DEBUG 1

static uint32_t fsm_mutex;
static state_t state = STATE_RESET;
static int id = 20;


void fsm_init(void)
{
	mutex_init(&fsm_mutex);
	while( fsm_lock() != FSM_LOCK_ACQUIRED );
	fsm_set_state(STATE_RESET);
	fsm_unlock();
}

uint8_t fsm_lock(void)
{
	uint8_t retval = FSM_LOCK_FAILED;

	if( mutex_lock(&fsm_mutex) == MUTEX_LOCKED )
		retval = FSM_LOCK_ACQUIRED;

	return retval;
}

void fsm_unlock(void)
{
	mutex_unlock(&fsm_mutex);
}

state_t fsm_get_state(void)
{
	return state;
}

int fsm_getID()
{
	return id;
}

void fsm_set_state(state_t new_state)
{
	/* only do this if the FSM has been locked! */
	if( fsm_mutex == MUTEX_LOCKED )
	{
		state = new_state;

		switch( state )
		{
		case STATE_RESET:
		default:
			break;

		case STATE_ECHO_BYTES:
			break;

		case STATE_PING:
			break;

		case STATE_PING_RECV:
			break;

		case STATE_UPDATE:
			break;

		case STATE_UPDATE_RECV:
			break;

		case STATE_GET_ID:
			break;

		case STATE_GOT_ID:
			break;

		case STATE_HAVE_ID:
			break;

		case STATE_ID_RECV:
			break;
		}
	}
}
