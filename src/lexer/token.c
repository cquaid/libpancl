/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pancl/pancl.h"
#include "internal.h"
#include "lexer/token.h"

void
token_init(struct token *t)
{
	t->type = TT_UNSET;
	t->subtype = TST_NONE;
	t->string = NULL;
	t->loc.line = 0;
	t->loc.column = 0;
}

void
token_fini(struct token *t)
{
	if (t == NULL)
		return;

	pancl_utf8_string_destroy(&(t->string));
	token_init(t);
}


int
token_set_string(struct token *t, int type, int subtype,
	struct pancl_utf8_string *string)
{
	t->type = type;
	t->subtype = subtype;
	t->string = string;

	return PANCL_SUCCESS;
}

static int
token_buffer_to_utf8_string(struct token_buffer *tb,
	struct pancl_utf8_string **string)
{
	/* tb->pos is the byte count. */
	int err = pancl_utf8_string_new(string, tb->pos);

	if (err != 0)
		return err;

	/* Copy in the string data. */
	memcpy((*string)->data, tb->buffer, tb->pos + 1);
	/* Set the code point count (bytes was already set). */
	(*string)->codepoints = tb->codepoints;

	return PANCL_SUCCESS;
}


int
token_set(struct token *t, int type, int subtype, struct token_buffer *tb)
{
	int err;
	struct pancl_utf8_string *string = NULL;

	err = token_buffer_to_utf8_string(tb, &string);

	if (err == PANCL_SUCCESS)
		err = token_set_string(t, type, subtype, string);

	if (err != PANCL_SUCCESS)
		pancl_utf8_string_destroy(&string);

	return err;
}

void
token_move(struct token *dest, struct token *src)
{
	*dest = *src;
	token_init(src);
}

int
token_buffer_append_c(struct token_buffer *tb, char c)
{
	int err;
	void *d;
	size_t new_size;

	if (tb->pos < tb->size)
		goto append;

	err = safe_add(tb->size, TOKEN_BUFFER_STEP, &new_size);

	if (err != PANCL_SUCCESS)
		return err;

	d = pancl_realloc(tb->buffer, new_size);

	if (d == NULL)
		return PANCL_ERROR_ALLOC;

	tb->size = new_size;
	tb->buffer = d;

append:
	tb->buffer[tb->pos++] = c;
	return PANCL_SUCCESS;
}

int
token_buffer_end(struct token_buffer *tb)
{
	/* Using append_c since we know this is a single-byte and we don't want the
	 * codepoint count to increase.
	 */
	int err = token_buffer_append_c(tb, '\0');

	/* Decrement the position when we add the '\0' in case someone attempts to
	 * append later (legal).
	 */
	if (err == PANCL_SUCCESS)
		tb->pos -= 1;

	return err;
}

void
token_buffer_fini(struct token_buffer *tb)
{
	if (tb == NULL)
		return;

	pancl_free(tb->buffer);
	tb->buffer = NULL;
}

// vim:ts=4:sw=4:autoindent
