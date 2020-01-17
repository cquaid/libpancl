/* SPDX-License-Identifier: MIT */
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"
#include "internal.h"

static void(*pancl_free_fn)(void *p) = free;
static void *(*pancl_alloc_fn)(size_t n) = malloc;
static void *(*pancl_realloc_fn)(void *p, size_t n) = realloc;

int
pancl_lib_set_allocators(void *(*alloc_fn)(size_t),
	void *(*realloc_fn)(void *, size_t),
	void(*free_fn)(void *))
{
	if (alloc_fn == NULL || realloc_fn == NULL || free_fn == NULL)
		return PANCL_ERROR_ARG_INVALID;

	pancl_alloc_fn = alloc_fn;
	pancl_realloc_fn = realloc_fn;
	pancl_free_fn = free_fn;

	return PANCL_SUCCESS;
}

void *
pancl_alloc(size_t n)
{
	return pancl_alloc_fn(n);
}

void
pancl_free(void *p)
{
	pancl_free_fn(p);
}

void *
pancl_realloc(void *p, size_t n)
{
	return pancl_realloc_fn(p, n);
}

int
pancl_resize(void **p, size_t n, size_t count)
{
	int err;
	size_t size;
	void *d;

	if (p == NULL || n == 0 || count == 0)
		return PANCL_ERROR_ALLOC;

	err = safe_mul(n, count, &size);

	if (err != 0)
		return err;

	d = pancl_realloc(*p, size);

	if (d == NULL)
		return PANCL_ERROR_ALLOC;

	*p = d;
	return PANCL_SUCCESS;
}

char *
pancl_strdup(const char *str)
{
	void *r;
	size_t len;

	if (str == NULL)
		return NULL;

	len = strlen(str);

	if (!can_inc(len))
		return NULL;

	r = pancl_alloc(len + 1);

	if (r != NULL)
		memcpy(r, str, len + 1);

	return r;
}

// vim:ts=4:sw=4:autoindent
