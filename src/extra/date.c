#include "date.h"

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

date *
date_new(const char *date_string)
{
	const unsigned int months[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned int feb_max = 28;

	date *d = calloc(1, sizeof(date));

	// Use current date
	if (date_string[0] == '\0') {
		time_t t = time(NULL);
		struct tm current = *localtime(&t);

		d->year = current.tm_year + 1900;
		d->month = current.tm_mon + 1;
		d->day = current.tm_mday;
	} else {
		// Try YYYY-MM-DD format
		int n = sscanf(date_string, "%4u-%2u-%2u", &d->year, &d->month, &d->day);
		if (n < 3) {
			// Try alternative format: YYYY/MM/DD
			n = sscanf(date_string, "%4u/%2u/%2u", &d->year, &d->month, &d->day);
		}

		if (n < 3) {
			// Formatting fail
			fprintf(stderr, "Date format must be YYYY-MM-DD or YYYY/MM/DD\n");
			goto ret_null;
		}

		// Check maximum month boundaries
		if (d->month < 1 || d->month > 12) {
			fprintf(stderr, "Month cannot be less than 1 or more than 12\n");
			goto ret_null;
		}

		// Check day boundaries
		if (d->day < 1 || d->day > months[d->month]) {
			fprintf(stderr, "Day cannot be less than 1 or more than the month (%u) last day (%u)\n", d->month, months[d->month]);
			goto ret_null;
		}

		// Check leap year
		if (d->month == 2) {
			if (d->year % 400 == 0) {
				feb_max = 29;
			} else if (d->year % 100 == 0) {
				feb_max = 28;
			} else if (d->year % 4 == 0) {
				feb_max = 29;
			} else {
				feb_max = 28;
			}

			if (d->day > feb_max) {
				fprintf(stderr, "The year %d does not have a leap year\n", d->year);
				goto ret_null;
			}
		}
	}

	return d;
ret_null:
	if (d != NULL) {
		free(d);
	}
	return NULL;
}

void
date_destroy(date *d)
{
	free(d);
}

