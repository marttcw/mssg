#ifndef STATES_H
#define STATES_H

#include <stdio.h>

enum e_states{ COPY, DET_SPEC, SPEC, VAR, AFT_SPEC };
enum e_states_spec{ IN, OUT };
enum e_var_type{ NONE, INT, STR, CONTENT };

typedef struct {
	char *name;
	char *value;
	enum e_var_type type;
} var_info;

typedef struct {
	// States
	enum e_states current_state;
	enum e_states_spec spec_state;
	enum e_states previous_state;

	// Copy
	char *line;
	unsigned int li;
	unsigned int li_max;

	// Templating
	char **keywords_list;
	unsigned int keyword_i;
	unsigned int kci;
	char prev;
	short in_spec_char;

	// Variable
	char *variable;
	unsigned int var_i;

	// Variables list
	var_info *variables_list;
	unsigned int var_l_m;

	// Input Files list
	FILE **fp_l;
	unsigned int fp_l_level;

	// Output file
	FILE *fp_o;
} state;

// Initialise/destroyer
int state_init(state *s);
int state_destroy(state *s);

int state_determine_state(state *s, const char *c);
int state_set_level_file(state *s, const char **filepath);
int state_set_output_file(state *s, const char **filepath);

#endif /* STATES_H */
