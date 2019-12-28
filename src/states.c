#include "states.h"
#include "template.h"

#include <stdlib.h>
#include <string.h>

#define ALLOC_SIZE (16)
#define ALLOC_SIZE_HM (16)
#define ALLOC_SIZE_SUB (320)
#define ALLOC_SIZE_LINE (1024)
#define ALLOC_SIZE_FILE (16)

//#define DEBUG

// states
int state_copy(state *s, const char *c);
int state_det_spec(state *s, const char *c);
int state_spec(state *s, const char *c);
int state_var(state *s, const char *c);
int state_aft_spec(state *s, const char *c);
int state_block(state *s, const char *c);

void
reset_keywords_list(state *s)
{
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->keywords_list[i][0] = '\0';
	}

	s->keyword_i = 0;
	s->kci = 0;
	s->prev = '%';
	s->fpsc_l[s->fp_l_level].sc.spec_state = OUT;
}

/* state struct initialiser
 *
 * params:
 * 	"state *" Given state struct
 */
state *
state_new(void)
{
	state *s = calloc(1, sizeof(state));

	s->err_int = calloc(1, sizeof(int));
	*s->err_int = 1;
	s->new_err = 0;
	s->fname_line = calloc(1024, sizeof(char));
	s->fpsc_l = calloc(ALLOC_SIZE, sizeof(fp_sc));
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->fpsc_l[i].sc.current_state = COPY;
		s->fpsc_l[i].sc.spec_state = OUT;
		s->fpsc_l[i].sc.previous_state = NONE;
		s->fpsc_l[i].fp = NULL;
		s->fpsc_l[i].type = -1;
		s->fpsc_l[i].filename = NULL;
		s->fpsc_l[i].line = 1;
	}

	s->keywords_list = calloc(ALLOC_SIZE, sizeof(char *));
	s->kci_alloc = calloc(ALLOC_SIZE, sizeof(int));
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->keywords_list[i] = calloc(ALLOC_SIZE_SUB, sizeof(char));
		s->kci_alloc[i] = ALLOC_SIZE;
	}
	s->keyword_i = 0;
	s->kci = 0;
	s->prev = '%';
	s->variable = calloc(ALLOC_SIZE_SUB, sizeof(char));
	s->var_i = 0;
	s->variables_hm = hashmap_new(ALLOC_SIZE_HM);
	s->errors_hm = hashmap_new(ALLOC_SIZE_HM);
	s->li_max = ALLOC_SIZE_LINE;
	s->line = calloc(s->li_max, sizeof(char));
	s->li = 0;
	s->fp_l_level = -1;
	s->fp_o = NULL;
	s->fp_l_level_max = -1;

	return s;
}

int
state_destroy(state *s)
{
#ifdef DEBUG
	printf("state_destroy called\n");
#endif
	if (s->fp_o != NULL) {
		fclose(s->fp_o);
	}
	free(s->line);
	hashmap_destroy(s->errors_hm);
	hashmap_destroy(s->variables_hm);
	free(s->variable);
	for (unsigned int i=0; i < ALLOC_SIZE; ++i) {
		free(s->keywords_list[i]);
	}
	free(s->kci_alloc);
	free(s->keywords_list);
	free(s->fpsc_l);
	free(s->fname_line);
	free(s->err_int);
	free(s);

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
	case '\n': case '{':
		// Null terminate string
		s->line[s->li] = '\0';

		if (s->line[0] != '\0') {
			if (*c == '\n') {
				fprintf(s->fp_o, "%s\n", s->line);
			} else {
				fprintf(s->fp_o, "%s", s->line);
			}
		}

		// Reset string
		s->li = 0;
		s->line[0] = '\0';

		switch (*c) {
		case '{':
			// Scope out of copy
			s->fpsc_l[s->fp_l_level].sc.current_state = DET_SPEC;
			break;
		case '\n':
			// Increment line counter
			++s->fpsc_l[s->fp_l_level].line;
		}
		break;
	default:
		// Reallocate more space if soon to be not enough
		if (s->li >= ((unsigned int) s->li_max * 0.95)) {
			s->li_max += (unsigned int) s->li_max * 0.5;
			s->line = realloc(s->line, s->li_max * sizeof(char));
		}
		s->line[s->li++] = *c;
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
		s->fpsc_l[s->fp_l_level].sc.current_state = SPEC;
		break;
	case '{':
		s->fpsc_l[s->fp_l_level].sc.current_state = VAR;
		break;
	default:
		s->fpsc_l[s->fp_l_level].sc.current_state = COPY;
		fprintf(s->fp_o, "{%c", *c);
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
			s->fpsc_l[s->fp_l_level].sc.spec_state = OUT;
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
		s->fpsc_l[s->fp_l_level].sc.spec_state = IN;
		s->in_spec_char = 0;
		break;
	case ' ':
		if (s->prev != '%') {
			// Null terminate string and refresh values
			s->keywords_list[s->keyword_i][s->kci] = '\0';
			++s->keyword_i;
			s->kci = 0;
		}
		break;
	case '\n':	// Only for configuration file
		if (s->fpsc_l[s->fp_l_level].type == 1) {
			// Use line
			s->keywords_list[s->keyword_i][s->kci] = '\0';
			++s->keyword_i;	// Offsets
			sprintf(s->fname_line, "%s_%d",
					s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
			int template_flag = template_keywords_list(s);
			if (template_flag != 1) {
				reset_keywords_list(s);
			}
			// Error occured, add to hashmap to prevent repeated messages
			if (template_flag < 0 && s->new_err) {
				hashmap_setValue(s->errors_hm, s->fname_line, s->err_int, sizeof(int), 0);
				s->new_err = 0;
			}
			break;
		}
		break;
	case '%':
		s->fpsc_l[s->fp_l_level].sc.previous_state = SPEC;
		s->fpsc_l[s->fp_l_level].sc.current_state = AFT_SPEC;
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
	switch (s->fpsc_l[s->fp_l_level].sc.spec_state) {
	case IN:	state_spec_in(s, c);	break;
	case OUT:	state_spec_out(s, c);	break;
	}

	return 0;
}

// TODO
/* At a variable block state
 * 	EX:
 * 		{% block foo %}
 * 			<p>
 * 			This is a block sections.
 * 			</p>
 * 		{% endblock %}
 *
 * returns int error codes
 * params:
 * 	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_block(state *s, const char *c)
{
	switch (*c) {
	case '{':
		s->prev = '{';
		break;
	case '%':
		if (s->prev == '{') {
			s->fpsc_l[s->fp_l_level].sc.previous_state = BLOCK;
			s->fpsc_l[s->fp_l_level].sc.current_state = SPEC;
			s->keywords_list[s->keyword_i][s->kci] = '\0';
			break;
		}
		s->keywords_list[s->keyword_i][s->kci++] = '{';
		// Implicitly tells the compiler that a fallthrough is expected
		__attribute__ ((fallthrough));
	default:
		if (s->kci >= (s->kci_alloc[s->keyword_i] - 1)) {
			s->kci_alloc[s->keyword_i] = (unsigned int) (1.5 * s->kci_alloc[s->keyword_i]);
			s->keywords_list[s->keyword_i] = realloc(s->keywords_list[s->keyword_i],
					s->kci_alloc[s->keyword_i] * sizeof(char));
		}
		s->keywords_list[s->keyword_i][s->kci++] = *c;
		s->prev = *c;
	}
	return 0;
}

/* At the variable use state
 *
 * returns int error codes
 * params:
 * 	"state *" Given state struct
 * 	"const char *" Character given
 */
int
state_var(state *s, const char *c)
{
	switch (*c) {
	case ' ':
		break;
	case '}':
		s->fpsc_l[s->fp_l_level].sc.previous_state = VAR;
		s->fpsc_l[s->fp_l_level].sc.current_state = AFT_SPEC;
		break;
	default:
		s->variable[s->var_i++] = *c;
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
	int template_flag = 0;


	switch (*c) {
	case '}':
		switch (s->fpsc_l[s->fp_l_level].sc.previous_state) {
		case SPEC:	// HTML file
			s->keywords_list[s->keyword_i][s->kci] = '\0';

			sprintf(s->fname_line, "%s_%d",
					s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
			template_flag = template_keywords_list(s);
			if (template_flag != 1) {
				reset_keywords_list(s);
			}
			// Error occured, add to hashmap to prevent repeated messages
			if (template_flag < 0 && s->new_err) {
				hashmap_setValue(s->errors_hm, s->fname_line, s->err_int, sizeof(int), 0);
				s->new_err = 0;
			}
			break;
		case VAR:
			// Null terminate string
			s->variable[s->var_i] = '\0';

			sprintf(s->fname_line, "%s_%d",
					s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
			template_flag = template_variable(s);
			if (template_flag < 0 && s->new_err) {
				hashmap_setValue(s->errors_hm, s->fname_line, s->err_int, sizeof(int), 0);
				s->new_err = 0;
			}

			// Clear variable
			s->variable[0] = '\0';
			s->var_i = 0;
			break;
		default:
			fprintf(stderr, "Error: State: AFT: Previous state not SPEC or VAR.\n");
			break;
		}
		s->fpsc_l[s->fp_l_level].sc.current_state = COPY;
		break;
	default:
		s->fpsc_l[s->fp_l_level].sc.current_state = SPEC;
	}

	return 0;
}

int
state_determine_state(state *s, const char *c)
{
	switch (s->fpsc_l[s->fp_l_level].sc.current_state) {
	case COPY:	state_copy(s, c); 	break;
	case SPEC:	state_spec(s, c); 	break;
	case DET_SPEC:	state_det_spec(s, c);	break;
	case AFT_SPEC:	state_aft_spec(s, c);	break;
	case VAR:	state_var(s, c);	break;
	case BLOCK:	state_block(s, c);	break;
	default:
		fprintf(stderr, "State error\n");
		return -2;
	}

	return 0;
}

int
state_config_state(state *s, const char *c)
{
	state_spec(s, c);
	return 0;
}

#ifdef DEBUG
int
state_debug_print(state *s)
{
	printf("States:\n");
	for (int i=0; i <= s->fp_l_level_max; ++i) {
		if (i == s->fp_l_level) {
			printf("=> %d\n", i);
		} else {
			printf("   %d\n", i);
		}
	}
	putchar('\n');

	return 0;
}
#endif

int
state_set_level_file(state *s, const char *filepath, int type)
{
	// -1: File not found/read error
	if ((s->fpsc_l[(s->fp_l_level + 1)].fp = fopen(filepath, "r")) == NULL) {
		fprintf(stderr, "Error occured, cannot read file: '%s'\n", filepath);
		return -1;
	}

	// Copy over the filename and type
	s->fpsc_l[(s->fp_l_level + 1)].filename = calloc(strlen(filepath), sizeof(char));
	strcpy(s->fpsc_l[(s->fp_l_level + 1)].filename, filepath);
	s->fpsc_l[(s->fp_l_level + 1)].type = type;

	++s->fp_l_level;
	++s->fp_l_level_max;

#ifdef DEBUG
	printf("level: %d/%d\n", s->fp_l_level, s->fp_l_level_max);
	printf("NEW: %s\n", filepath);
	state_debug_print(s);
#endif

	return 0;
}

int
fpsc_swap(fp_sc *p1, fp_sc *p2)
{
	fp_sc temp = *p1;
	*p1 = *p2;
	*p2 = temp;

	return 0;
}

int
state_set_bef_level_file(state *s, const char *filepath, int type)
{
	if (state_set_level_file(s, filepath, type) == -1) {
		return -1;
	}

	fpsc_swap(&s->fpsc_l[s->fp_l_level - 1], &s->fpsc_l[s->fp_l_level]);

#ifdef DEBUG
	printf("%d <-> %d\n", s->fp_l_level-1, s->fp_l_level);
	state_debug_print(s);
#endif
	return 0;
}

int
state_level_up(state *s)
{
	// -1: Cannot go above max level
	if (s->fp_l_level == s->fp_l_level_max) {
		return -1;
	}
	s->fpsc_l[++s->fp_l_level].sc.current_state = COPY;
#ifdef DEBUG
	printf("DEBUG: state_level_up\n");
	state_debug_print(s);
#endif
	return 0;
}

int
state_level_down(state *s)
{
	// -1: Cannot go below min level zero
	if (s->fp_l_level == 0) {
		--s->fp_l_level;
		return -1;
	}
	s->fpsc_l[--s->fp_l_level].sc.current_state = COPY;
#ifdef DEBUG
	printf("DEBUG: state_level_down\n");
	state_debug_print(s);
#endif
	return 0;
}

int
state_level_down_close(state *s)
{
	s->fpsc_l[s->fp_l_level].line = 1;

	// Free up resources
	if (s->fpsc_l[s->fp_l_level].fp != NULL) {
		fclose(s->fpsc_l[s->fp_l_level].fp);
	}
#ifdef DEBUG
	printf("Down: %d: %s\n", s->fp_l_level, s->fpsc_l[s->fp_l_level].filename);
#endif
	if (s->fpsc_l[s->fp_l_level].filename != NULL) {
		free(s->fpsc_l[s->fp_l_level].filename);
	}

	if (state_level_down(s) == -1) {
		s->fp_l_level = -1;
		s->fp_l_level_max = -1;
		return -1;
	}
	--s->fp_l_level_max;
#ifdef DEBUG
	printf("DEBUG: state_level_down_close\n");
	state_debug_print(s);
#endif
	return 0;
}

int
state_set_output_file(state *s, const char *filepath)
{
	// -1: File not found/read error
	if ((s->fp_o = fopen(filepath, "w")) == NULL) {
		fprintf(stderr, "Error occured, cannot set output file: '%s'\n", filepath);
		perror("    Output error");
		return -1;
	}
	return 0;
}

int
state_generate(state *s)
{
	char c = '\0';

	while (s->fp_l_level >= 0) {
		// Read the file
		while ((c = fgetc(s->fpsc_l[s->fp_l_level].fp)) != EOF) {
			switch (s->fpsc_l[s->fp_l_level].type) {
			case 0:		// HTML file
				if (state_determine_state(s, &c) < 0) {
					fprintf(stderr, "ERROR: File generation error has occured\n");
					fclose(s->fpsc_l[s->fp_l_level].fp);
					return -2;
				}
				break;
			case 1:		// Configuration file
				if (state_config_state(s, &c) < 0) {
					fprintf(stderr, "ERROR: File configuration read error has occured\n");
					fclose(s->fpsc_l[s->fp_l_level].fp);
					return -3;
				}
				break;
			}
		}
#ifdef DEBUG
		printf("End of file read\n");
#endif

		// Shift back down if possible, otherwise stop loop
		if (state_level_down_close(s) < 0) {
#ifdef DEBUG
			printf("state_level_down_close return < 0\n");
#endif
			break;
		}
	}

	return 0;
}

