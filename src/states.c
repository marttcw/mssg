#include "states.h"

#include <stdio.h>
#include <stdlib.h>

#define ALLOC_SIZE (16)
#define ALLOC_SIZE_SUB (320)

// states
int state_copy(state *s, const char *c);
int state_det_spec(state *s, const char *c);
int state_spec(state *s, const char *c);
int state_aft_spec(state *s, const char *c);

void
reset_keywords_list(state *s)
{
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->keywords_list[i][0] = '\0';
	}

	s->keyword_i = 0;
	s->kci = 0;
	s->prev = '%';
	s->spec_state = OUT;
}

void
print_keywords_list(state *s)
{
	for (unsigned int i=0; i < s->keyword_i; ++i) {
		printf("%d: %s\n", i, s->keywords_list[i]);
	}
}

/* state struct initialiser
 *
 * params:
 * 	"state *" Given state struct
 */
int
state_init(state *s)
{
	s->current_state = COPY;
	s->spec_state = OUT;
	s->keywords_list = (char **) calloc(ALLOC_SIZE, sizeof(char *));
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->keywords_list[i] = (char *) calloc(ALLOC_SIZE_SUB, sizeof(char));
	}
	s->keyword_i = 0;
	s->kci = 0;
	s->prev = '%';

	return 0;
}

int
state_destroy(state *s)
{
	for (int i=0; i < ALLOC_SIZE; ++i) {
		free(s->keywords_list[i]);
	}
	free(s->keywords_list);

	return 0;
}

/* Simple copy over state
 *
 * returns int error codes
 * params:
 *   	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_copy(state *s, const char *c)
{
	switch (*c) {
	case '{':
		s->current_state = DET_SPEC;
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
 *  	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_det_spec(state *s, const char *c)
{
	switch (*c) {
	case '%':
		s->current_state = SPEC;
		break;
	default:
		s->current_state = COPY;
		printf("{%c", *c);
	}

	return 0;
}

/* Inside the implicit string state
 *
 * returns int error codes
 * params:
 * 	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_spec_in(state *s, const char *c)
{
	switch (*c) {
	case '\\':
		if (!s->in_spec_char) {
			s->in_spec_char = 1;
			break;
		}
		// Implicitly tells the compiler that a fallthrough is expected
		__attribute__ ((fallthrough));
	case '"':
		if (!s->in_spec_char) {
			// Move to OUT state and set prev 
			s->spec_state = OUT;
			s->prev = *c;
			break;
		}
		s->in_spec_char = 0;	// Put character out of special (for next character)
		// Implicitly tells the compiler that a fallthrough is expected
		__attribute__ ((fallthrough));
	default:
		s->keywords_list[s->keyword_i][s->kci++] = *c;
	}

	return 0;
}

/* Outside the implicit string state
 *
 * returns int error codes
 * params:
 * 	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_spec_out(state *s, const char *c)
{
	switch (*c) {
	case '"':
		// Move to IN state and set special char default
		s->spec_state = IN;
		s->in_spec_char = 0;
		break;
	case ' ':
		if (s->prev != '%') {
			++s->keyword_i;
			s->kci = 0;
		}
		break;
	case '%':
		s->current_state = AFT_SPEC;
		break;
	default:
		s->keywords_list[s->keyword_i][s->kci++] = *c;
		s->prev = *c;
	}

	return 0;
}

/* At the special section state
 *
 * returns int error codes
 * params:
 * 	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_spec(state *s, const char *c)
{
	switch (s->spec_state) {
	case IN:	state_spec_in(s, c);	break;
	case OUT:	state_spec_out(s, c);	break;
	}

	return 0;
}

/* End of special section with '%}' state
 *
 * returns int error codes
 * params:
 * 	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_aft_spec(state *s, const char *c)
{
	switch (*c) {
	case '}':
		print_keywords_list(s);
		reset_keywords_list(s);
		s->current_state = COPY;
		break;
	default:
		s->current_state = SPEC;
	}

	return 0;
}

int
state_determine_state(state *s, const char *c)
{
	switch (s->current_state) {
	case COPY:	state_copy(s, c); 	break;
	case SPEC:	state_spec(s, c); 	break;
	case DET_SPEC:	state_det_spec(s, c);	break;
	case AFT_SPEC:	state_aft_spec(s, c);	break;
	default:
		fprintf(stderr, "State error\n");
		return -2;
	}

	return 0;
}

