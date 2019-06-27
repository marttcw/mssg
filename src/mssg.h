#ifndef MSSG_H
#define MSSG_H

/* Pre-processors defines */
#define VERSION (20190627)
#define CONF_NAME "config"
#define MIN_COMPAT (20190627)
#define FGEN_LEN (1024)

/* generator.c */
int generate(int argc, char **argv, int flag);
int new_generate(int argc, char **argv, int flag);

/* mdtohtml.c */
char *mdline_to_html(const char *line, int *inpara);

/* blog.c */
int new_post(int argc, char **argv, int flag);

#endif /* MSSG_H */

