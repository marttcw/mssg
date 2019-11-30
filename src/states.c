#include "states.h"

#include <stdio.h>

/* Simple copy over state
 *
 * returns int error codes
 * params:
 * 	"enum e_states *" Current state
 * 	"const char *" Character given
 */
int
state_copy(enum e_states *state, const char *c)
{
	switch (*c) {
	case '{':
		*state = DET_SPEC;
		break;
	default:
		printf("%c", *c);
	}

	return 0;
}

/* Start of special section state with '{%'
 *
 * returns int error codes
 * params:
 * 	"enum e_states *" Current state
 * 	"const char *" Character given
 */
int
state_det_spec(enum e_states *state, const char *c)
{
	switch (*c) {
	case '%':
		*state = SPEC;
		break;
	default:
		*state = COPY;
		printf("{%c", *c);
	}

	return 0;
}

/* At the special section state
 *
 * returns int error codes
 * params:
 * 	"enum e_states *" Current state
 * 	"const char *" Character given
 */
int
state_spec(enum e_states *state, const char *c)
{
	switch (*c) {
	case ' ':
		break;
	case '%':
		*state = AFT_SPEC;
		break;
	default:
		printf(">%c", *c);
	}

	return 0;
}

/* End of special section with '%}' state
 *
 * returns int error codes
 * params:
 * 	"enum e_states *" Current state
 * 	"const char *" Character given
 */
int
state_aft_spec(enum e_states *state, const char *c)
{
	switch (*c) {
	case '}':
		*state = COPY;
		break;
	default:
		*state = SPEC;
	}

	return 0;
}

