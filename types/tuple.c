/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"
#include "internal.h"

void
pancl_tuple_init(struct pancl_tuple *tuple)
{
	if (tuple != NULL)
		memset(tuple, 0, sizeof(*tuple));
}

void
pancl_tuple_fini(struct pancl_tuple *tuple)
{
	if (tuple == NULL)
		return;

	if (tuple->values != NULL) {
		size_t i;

		for (i = 0; i < tuple->count; ++i)
			pancl_value_destroy(&(tuple->values[i]));

		pancl_free(tuple->values);
	}

	pancl_tuple_init(tuple);
}

int
pancl_tuple_append(struct pancl_tuple *tuple, struct pancl_value *value)
{
	void *d;
	size_t new_size;

	if (tuple == NULL || value == NULL)
		return PANCL_ERROR_ARG_INVALID;

	/* XXX: overflow */
	new_size = sizeof(*(tuple->values)) * (tuple->count + 1);

	d = pancl_realloc(tuple->values, new_size);

	if (d == NULL)
		return PANCL_ERROR_ALLOC;

	tuple->values = d;
	tuple->values[tuple->count] = value;
	tuple->count += 1;

	return PANCL_SUCCESS;
}

// vim:ts=4:sw=4:autoindent
