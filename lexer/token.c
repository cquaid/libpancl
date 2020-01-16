/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pancl_error.h"
#include "lexer/token.h"

void
token_init(struct token *t)
{
	t->type = TT_UNSET;
	t->subtype = TST_NONE;
	t->value = NULL;
	t->pos.line = 0;
	t->pos.column = 0;
}

void
token_fini(struct token *t)
{
	if (t == NULL)
		return;

	free(t->value);
	token_init(t);
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
token_append(struct token *t, const char *value)
{
	void *tmp;
	size_t len;
	size_t add;

	if (value == NULL)
		return PANCL_SUCCESS;

	if (t->value == NULL)
		len = 0;
	else
		len = strlen(t->value);

	add = strlen(value);

	/* XXX: overflow check. */
	tmp = realloc(t->value, len + add + 1);

	if (tmp == NULL)
		return PANCL_ERROR_ALLOC;

	t->value = tmp;

	memcpy(&(t->value[len]), value, add + 1);
	return PANCL_SUCCESS;
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
