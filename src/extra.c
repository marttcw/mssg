#include "extra.h"

#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int
m_mkdir(char *dirpath, mode_t mode)
{
	char temp[strlen(dirpath)];
	size_t i, len;
	int j=0;
	int ret;

	len = strlen(dirpath);

	if (dirpath[len - 1] == '/') {
		dirpath[len - 1] = '\0';
	}

	for (i=0; i < len; ++i) {
		if (dirpath[i] == '/') {
			temp[j] = '\0';

			if ((ret = mkdir(temp, mode)) < 0) {
				if (errno != 17) {
					return ret;
				}
			}
		}
		temp[j++] = dirpath[i];
	}

	temp[j] = '\0';
	if ((ret = mkdir(temp, mode)) < 0) {
		if (errno != 17) {
			return ret;
		}
	}

	return 0;
}


