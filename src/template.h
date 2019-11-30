#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "states.h"

int template_base(state *s, int argc, char **argv, int flag);
int template_string(state *s, int argc, char **argv, int flag);
int template_sub_content(state *s, int argc, char **argv, int flag);

#endif /* TEMPLATE_H */
