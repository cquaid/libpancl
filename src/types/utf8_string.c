/* SPDX-License-Identifier: MIT */
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl/pancl.h"
#include "internal.h"

int
pancl_utf8_string_new(struct pancl_utf8_string **string,
	size_t bytes)
{
	int err;
	size_t total_size;

	if (string == NULL)
		return PANCL_ERROR_ARG_INVALID;

	err = safe_add(sizeof(**string), bytes, &total_size);

	if (err != 0)
		return err;

	*string = pancl_zalloc(total_size);

	if (*string == NULL)
		return PANCL_ERROR_ALLOC;

	/* No +1 on the byte count since we don't include the NUL terminator. */
	(*string)->bytes = bytes;
	return PANCL_SUCCESS;
}

void
pancl_utf8_string_destroy(struct pancl_utf8_string **string)
{
	if (string == NULL || *string == NULL)
		return;

	pancl_free(*string);
	*string = NULL;
}

// vim:ts=4:sw=4:autoindent
