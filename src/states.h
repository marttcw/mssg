#ifndef STATES_H
#define STATES_H

enum e_states{ COPY, DET_SPEC, SPEC, AFT_SPEC };

int state_copy(enum e_states *state, const char *c);
int state_det_spec(enum e_states *state, const char *c);
int state_spec(enum e_states *state, const char *c);
int state_aft_spec(enum e_states *state, const char *c);

#endif /* STATES_H */
