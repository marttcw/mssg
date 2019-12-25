#ifndef STATES_H
#define STATES_H

#include <stdio.h>

enum e_states{ STATE_NONE, COPY, DET_SPEC, SPEC, VAR, AFT_SPEC, BLOCK };
enum e_states_spec{ IN, OUT };
enum e_var_type{ NONE, INT, STR, CONTENT };
enum e_var_flag{ GLOBAL, LOCAL };

typedef struct {
	char *name;
	char *value;
	enum e_var_type type;
	enum e_var_flag flag;
} var_info;

typedef struct {
	enum e_states current_state;
	enum e_states_spec spec_state;
	enum e_states previous_state;
} states_collection;

typedef struct {
	states_collection sc;
	FILE *fp;
	int type;
	char *filename;
	unsigned int line;
} fp_sc;

typedef struct {
	// Copy
	char *line;
	unsigned int li;
	unsigned int li_max;

	// Templating
	char **keywords_list;
	unsigned int *kci_alloc;
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

	// List of files and its associated configuration
	fp_sc *fpsc_l;
	int fp_l_level;
	int fp_l_level_max;

	// Output file
	FILE *fp_o;
} state;

// Initialise/destroyer
state *state_new(void);
int state_destroy(state *s);

int state_determine_state(state *s, const char *c);
int state_set_level_file(state *s, const char *filepath, int type);
int state_set_bef_level_file(state *s, const char *filepath, int type);
int state_set_output_file(state *s, const char *filepath);
int state_generate(state *s);
int state_level_up(state *s);
int state_level_down(state *s);
int state_config(state *s, const char *filepath);
int state_generate_cleanup(state *s);

#endif /* STATES_H */
