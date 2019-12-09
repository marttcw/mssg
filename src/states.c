#include "states.h"
#include "template.h"

#include <stdlib.h>
#include <string.h>

#define ALLOC_SIZE (16)
#define ALLOC_SIZE_VAR (128)
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
int
state_init(state *s)
{
	s->fpsc_l = malloc(ALLOC_SIZE * (sizeof(fp_sc)));
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->fpsc_l[i].sc.current_state = COPY;
		s->fpsc_l[i].sc.spec_state = OUT;
		s->fpsc_l[i].sc.previous_state = NONE;
		s->fpsc_l[i].fp = NULL;
	}

	s->keywords_list = calloc(ALLOC_SIZE, sizeof(char *));
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->keywords_list[i] = calloc(ALLOC_SIZE_SUB, sizeof(char));
	}
	s->keyword_i = 0;
	s->kci = 0;
	s->prev = '%';
	s->variable = calloc(ALLOC_SIZE_SUB, sizeof(char));
	s->var_i = 0;
	s->var_l_m = 0;
	s->variables_list = calloc(ALLOC_SIZE_VAR, sizeof(var_info));
	for (int i=0; i < ALLOC_SIZE; ++i) {
		s->variables_list[i].name = calloc(ALLOC_SIZE_SUB, sizeof(char));
		s->variables_list[i].value = calloc(ALLOC_SIZE_SUB, sizeof(char));
		s->variables_list[i].type = NONE;
	}
	s->li_max = ALLOC_SIZE_LINE;
	s->line = calloc(s->li_max, sizeof(char));
	s->li = 0;
	s->fp_l_level = -1;
	s->fp_o = NULL;
	s->fp_l_level_max = -1;

	return 0;
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
	for (unsigned int i=0; i < ALLOC_SIZE_VAR; ++i) {
		free(s->variables_list[i].name);
		free(s->variables_list[i].value);
	}
	free(s->variables_list);
	free(s->variable);
	for (unsigned int i=0; i < ALLOC_SIZE; ++i) {
		free(s->keywords_list[i]);
	}
	free(s->keywords_list);
	free(s->fpsc_l);

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

		// Scope out of copy
		if (*c == '{') {
			s->fpsc_l[s->fp_l_level].sc.current_state = DET_SPEC;
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
	switch (*c) {
	case '}':
		switch (s->fpsc_l[s->fp_l_level].sc.previous_state) {
		case SPEC:
			s->keywords_list[s->keyword_i][s->kci] = '\0';
			template_keywords_list(s);
			reset_keywords_list(s);
			break;
		case VAR:
			// Null terminate string
			s->variable[s->var_i] = '\0';

			template_variable(s);

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
	default:
		fprintf(stderr, "State error\n");
		return -2;
	}

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
state_set_level_file(state *s, const char *filepath)
{
	// -1: File not found/read error
	if ((s->fpsc_l[(s->fp_l_level + 1)].fp = fopen(filepath, "r")) == NULL) {
		fprintf(stderr, "Error occured, cannot read file: '%s'\n", filepath);
		return -1;
	}

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
state_set_bef_level_file(state *s, const char *filepath)
{
	if (state_set_level_file(s, filepath) == -1) {
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
	if (s->fpsc_l[s->fp_l_level].fp != NULL) {
		fclose(s->fpsc_l[s->fp_l_level].fp);
	}
	if (state_level_down(s) == -1) {
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
			if (state_determine_state(s, &c) < 0) {
				fprintf(stderr, "ERROR: File generation error has occured\n");
				fclose(s->fpsc_l[s->fp_l_level].fp);
				return -2;
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

/* Configuration file
 * TODO
 */
int
state_config(state *s, const char *filepath)
{
	(void)(s);

	FILE *fp = NULL;
	//char line[1024];

	fp = fopen(filepath, "r");

/* TODO
	while ((fgets(line, 1024, fp)) != NULL) {

	}
*/

	if (fp != NULL) {
		fclose(fp);
	}

	return 0;
}

