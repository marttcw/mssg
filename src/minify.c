#include "minify.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum {
	CHUNK_SIZE = 512,
	MAX_LEVELS = 64
};

enum minify_state {
	MINIFY_STATE_OUT = 0,
	MINIFY_STATE_TAG_READ,
	MINIFY_STATE_TAG_LOOSE,
	MINIFY_STATE_TAG_DIRECT,
	MINIFY_STATE_TAG_CLOSE,

	MINIFY_STATE_TOTAL
};

struct minify {
	uint64_t 		current_file_position;
	enum minify_state	state;
	char			tag[64];
	uint32_t		tag_length;
	char			prev;
	uint32_t		level;
	enum minify_state	level_state[MAX_LEVELS];
};

static bool
minify__rc_out(struct minify *minify,
		const char ch)
{
	switch (ch)
	{
	case '<':
		minify->tag[0] = '\0';
		minify->tag_length = 0;

		minify->state = MINIFY_STATE_TAG_READ;
		return true;
	default:
		return false;
	}
}

static bool
minify__rc_tread(struct minify *minify,
		const char ch)
{
	switch (ch)
	{
	case '>':
		minify->tag[minify->tag_length] = '\0';
		++minify->level;

		if (!strcmp(minify->tag, "pre"))
		{
			minify->state = MINIFY_STATE_TAG_DIRECT;
		}
		else
		{
			minify->state = MINIFY_STATE_TAG_LOOSE;
		}
		minify->level_state[minify->level] = minify->state;
		break;
	default:
		minify->tag[minify->tag_length++] = ch;
		break;
	}

	return true;
}

static bool
minify__rc_tloose(struct minify *minify,
		const char ch)
{
	bool show = false;
	switch (ch)
	{
	case '/':
		if (minify->prev == '<')
		{
			minify->state = MINIFY_STATE_TAG_CLOSE;
			show = true;
		}
		break;
	case ' ':
		if (minify->prev != ch)
		{
			show = true;
		}
		break;
	case '\t':
	case '\n':
		break;
	default:
		show = true;
	}

	return show;
}

static bool
minify__rc_tdirect(struct minify *minify,
		const char ch)
{
	switch (ch)
	{
	case '/':
		if (minify->prev == '<')
		{
			minify->state = MINIFY_STATE_TAG_CLOSE;
		}
		break;
	default:
		break;
	}

	return true;
}

static bool
minify__rc_tclose(struct minify *minify,
		const char ch)
{
	switch (ch)
	{
	case '>':
		--minify->level;
		minify->state = minify->level_state[minify->level];
		break;
	}

	return true;
}

static bool
minify__rc(struct minify *minify,
		const char ch)
{
	static bool (*func[MINIFY_STATE_TOTAL])
		(struct minify *, const char) = {
			[MINIFY_STATE_OUT] = minify__rc_out,
			[MINIFY_STATE_TAG_READ] = minify__rc_tread,
			[MINIFY_STATE_TAG_LOOSE] = minify__rc_tloose,
			[MINIFY_STATE_TAG_DIRECT] = minify__rc_tdirect,
			[MINIFY_STATE_TAG_CLOSE] = minify__rc_tclose,
	};
	bool retval =  func[minify->state](minify, ch);
	minify->prev = ch;
	return retval;
}

void
minify(FILE *out_stream, FILE *in_stream)
{
	size_t read_size = CHUNK_SIZE;
	char chunk[CHUNK_SIZE + 1] = { 0 };
	struct minify minify = {
		.current_file_position = 0,
		.level = 0,
		.level_state[0] = MINIFY_STATE_OUT
	};

	rewind(in_stream);

	while (read_size == CHUNK_SIZE)
	{
		uint64_t file_position = ftell(in_stream);
		read_size = fread(chunk, sizeof(char), CHUNK_SIZE, in_stream);
		chunk[read_size] = '\0';

		if (ferror(in_stream))
		{
			break;
		}

		uint32_t j = 0;
		char nchunk[CHUNK_SIZE + 1] = { 0 };
		for (uint32_t i = 0; i < read_size; ++i, ++file_position)
		{
			// Parse the character
			minify.current_file_position = file_position;
			if (minify__rc(&minify, chunk[i]))
			{
				nchunk[j++] = chunk[i];
			}
		}

		nchunk[j] = '\0';
		fwrite(nchunk, sizeof(char), j, out_stream);
	}

	fwrite("\n", sizeof(char), 1, out_stream);
}

