/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pancl_error.h"
#include "lexer/token.h"

void
token_fini(struct token *t)
{
	if (t == NULL)
		return;

	free(t->value);
	t->value = NULL;
}

int
token_set(struct token *t, int type, int subtype, const char *value)
{
	t->type = type;
	t->subtype = subtype;

	if (value == NULL) {
		t->value = NULL;
		return PANCL_SUCCESS;
	}

	t->value = strdup(value);

	if (t->value == NULL)
		return PANCL_ERROR_ALLOC;

	return PANCL_SUCCESS;
}

int
token_buffer_append_c(struct token_buffer *tb, char c)
{
	void *d;

	if (tb->pos < tb->size)
		goto append;

	/* Realloc. XXX overflow check. XXX define for step */
	d = realloc(tb->buffer, tb->size + TOKEN_BUFFER_STEP);

	if (d == NULL)
		return PANCL_ERROR_ALLOC;

	tb->size += TOKEN_BUFFER_STEP;
	tb->buffer = d;

append:
	tb->buffer[tb->pos++] = c;
	return PANCL_SUCCESS;
}

void
token_buffer_fini(struct token_buffer *tb)
{
	if (tb == NULL)
		return;

	free(tb->buffer);
	tb->buffer = NULL;
}

// vim:ts=4:sw=4:autoindent
