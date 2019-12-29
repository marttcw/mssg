#ifndef STATES_H
#define STATES_H

#include <stdio.h>

#include "hashmap.h"

enum e_states{ STATE_NONE, COPY, DET_SPEC, SPEC, VAR, AFT_SPEC, BLOCK };
enum e_states_spec{ IN, OUT };
enum e_var_type{ NONE = 0, INT = 1, STR = 2, CONTENT = 3, LIST =4, DICT = 5 };
enum e_var_flag{ GLOBAL, LOCAL };

typedef struct {
	char *value;
	enum e_var_type type;
	enum e_var_flag flag;
} var_string;

typedef struct {
	char **list;
	unsigned int size;
	enum e_var_type type;
	enum e_var_flag flag;
} var_list;

typedef struct {
	hashmap *hashmap;
	enum e_var_type type;
	enum e_var_flag flag;
} var_dict;

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

	// Variables hashmap
	hashmap *variables_hm;

	// List of files and its associated configuration
	fp_sc *fpsc_l;
	int fp_l_level;
	int fp_l_level_max;

	// Output file
	FILE *fp_o;

	// Errors hashmap
	hashmap *errors_hm;
	char *fname_line;

	int *err_int;
	int new_err;
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

#endif /* STATES_H */
