#include "blog.h"

#include "extra.h"

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>

void
blog_new(const char *date, const char *title)
{
	char src_path[36], src_file[52];
	const unsigned int months[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned int feb_max = 28;
	unsigned int year, month, day;

	// Use current date
	if (date[0] == '\0') {
		time_t t = time(NULL);
		struct tm current = *localtime(&t);

		year = current.tm_year + 1900;
		month = current.tm_mon + 1;
		day = current.tm_mday;
	} else {
		// Try YYYY-MM-DD format
		int n = sscanf(date, "%4u-%2u-%2u", &year, &month, &day);
		if (n < 3) {
			// Try alternative format: YYYY/MM/DD
			n = sscanf(date, "%4u/%2u/%2u", &year, &month, &day);
		}

		if (n < 3) {
			// Formatting fail
			fprintf(stderr, "Date format must be YYYY-MM-DD or YYYY/MM/DD\n");
			return;
		}

		// Check maximum month boundaries
		if (month < 1 || month > 12) {
			fprintf(stderr, "Month cannot be less than 1 or more than 12\n");
			return;
		}

		// Check day boundaries
		if (day < 1 || day > months[month]) {
			fprintf(stderr, "Day cannot be less than 1 or more than the month (%u) last day (%u)\n", month, months[month]);
			return;
		}

		// Check leap year
		if (month == 2) {
			if (year % 400 == 0) {
				feb_max = 29;
			} else if (year % 100 == 0) {
				feb_max = 28;
			} else if (year % 4 == 0) {
				feb_max = 29;
			} else {
				feb_max = 28;
			}

			if (day > feb_max) {
				fprintf(stderr, "The year %d does not have a leap year\n", year);
				return;
			}
		}
	}

	// Boundaries checking passed, create source file from it
	sprintf(src_path, "src/blog/%4d/%02d/%02d", year, month, day);
	sprintf(src_file, "%s/index.html", src_path);

	if (m_mkdir(src_path, 0777) < 0) {
		// If the error is not directory already exists
		if (errno != 17) {
			perror("Error: Directory creation error: ");
		}
	} else {
		FILE *fp = fopen(src_file, "w");

		fprintf(fp, "{%% base src/base.html %%}\n"
				"<h1>%s</h1>\n"
				"<p>%d-%d-%d</p>\n"
				"Blog template\n"
				, title, year, month, day);

		fclose(fp);
	}

	printf("Blog html file created at: '%s'\n", src_file);
}

