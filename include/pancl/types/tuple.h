/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_TUPLE
#define H_PANCL_TYPES_TUPLE

#include <stddef.h>

#include "pancl/types/location.h"

struct pancl_value;

/**
 * Represents a tuple: ( ... )
 */
struct pancl_tuple {
	/**
	 * Where this was found in the input.
	 */
	struct pancl_location loc;
	/**
	 * Number of entries in the values array.
	 */
	size_t count;
	/**
	 * Entries in the tuple.  Each entry may be of a different type.
	 *
	 * If count != 0, then this is guaranteed to be non-NULL.
	 *
	 * If this is non-NULL then each pointer pointed to is guaranteed to also
	 * be non-NULL.
	 */
	struct pancl_value **values;
};

#endif /* H_PANCL_TYPES_TUPLE */
// vim:ts=4:sw=4:autoindent
