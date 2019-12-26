#ifndef EXTRA_DATE_H
#define EXTRA_DATE_H

typedef struct {
	unsigned int year;
	unsigned int month;
	unsigned int day;
} date;

date *date_new(const char *date_string);
void date_destroy(date *d);

#endif /* EXTRA_DATE_H */
