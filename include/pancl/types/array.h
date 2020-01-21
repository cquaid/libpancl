/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_ARRAY
#define H_PANCL_TYPES_ARRAY

#include <stddef.h>

#include "pancl/types/location.h"

struct pancl_value;

/**
 * Represents an array: [ ... ]
 */
struct pancl_array {
	/**
	 * Where this was found in the input.
	 */
	struct pancl_location loc;
	/**
	 * Number of entries in the values array.
	 */
	size_t count;
	/**
	 * Entries in the array.  Each entry is guaranteed by the parser to be of
	 * the same type.  Defreference values[0]->type for the array type.
	 *
	 * If count != 0, then this is guaranteed to be non-NULL.
	 *
	 * If this is non-NULL then each pointer pointed to is guaranteed to also
	 * be non-NULL.
	 */
	struct pancl_value **values;
};

#endif /* H_PANCL_TYPES_ARRAY */
// vim:ts=4:sw=4:autoindent
