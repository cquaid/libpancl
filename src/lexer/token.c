/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pancl_error.h"
#include "internal.h"
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

	pancl_free(t->value);
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

	t->value = pancl_strdup(value);

	if (t->value == NULL)
		return PANCL_ERROR_ALLOC;

	return PANCL_SUCCESS;
}

int
token_append(struct token *t, const char *value)
{
	int err;
	void *tmp;
	size_t len;
	size_t add;
	size_t total;

	if (value == NULL)
		return PANCL_SUCCESS;

	if (t->value == NULL)
		len = 0;
	else
		len = strlen(t->value);

	add = strlen(value);

	/* Overflow checks. */
	err = safe_add(len, add, &total);

	if (err != PANCL_SUCCESS)
		return err;

	if (!can_inc(total))
		return PANCL_ERROR_OVERFLOW;

	tmp = pancl_realloc(t->value, total + 1);

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
	int err = token_buffer_append(tb, '\0');

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
