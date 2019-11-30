#ifndef STATES_H
#define STATES_H

enum e_states{ COPY, DET_SPEC, SPEC, AFT_SPEC };
enum e_states_spec{ IN, OUT };

typedef struct {
	enum e_states current_state;
	enum e_states_spec spec_state;
	char **keywords_list;
	unsigned int keyword_i;
	unsigned int kci;
	char prev;
	short in_spec_char;
} state;

// Initialise/destroyer
int state_init(state *s);
int state_destroy(state *s);

int state_determine_state(state *s, const char *c);

#endif /* STATES_H */
