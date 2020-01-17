/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"
#include "internal.h"

void
pancl_array_init(struct pancl_array *array)
{
	if (array != NULL)
		memset(array, 0, sizeof(*array));
}

void
pancl_array_fini(struct pancl_array *array)
{
	if (array == NULL)
		return;

	if (array->values != NULL) {
		size_t i;

		for (i = 0; i < array->count; ++i)
			pancl_value_destroy(&(array->values[i]));

		pancl_free(array->values);
	}

	pancl_array_init(array);
}

int
pancl_array_append(struct pancl_array *array, struct pancl_value *value)
{
	int err;

	if (array == NULL || value == NULL)
		return PANCL_ERROR_ARG_INVALID;

	/* Arrays have to validate that they only hold one type of data. */
	if (array->count != 0) {
		if (array->values[0]->type != value->type)
			return PANCL_ERROR_ARRAY_MEMBER_TYPE;
	}

	if (!can_inc(array->count))
		return PANCL_ERROR_OVERFLOW;

	err = pancl_resize((void **)&(array->values), sizeof(*(array->values)),
			array->count + 1);

	if (err == PANCL_SUCCESS) {
		array->values[array->count] = value;
		array->count += 1;
	}

	return err;
}

// vim:ts=4:sw=4:autoindent
