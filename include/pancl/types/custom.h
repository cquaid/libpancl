/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_CUSTOM
#define H_PANCL_TYPES_CUSTOM

#include "pancl/types/location.h"
#include "pancl/types/tuple.h"

struct pancl_utf8_string;

/**
 * Represents a custom type: name( ... )
 */
struct pancl_custom {
	/**
	 * Where this was found in the input.
	 */
	struct pancl_location loc;
	/**
	 * Name of the custom type.
	 *
	 * Guaranteed to be non-NULL and non-empty.
	 */
	struct pancl_utf8_string *name;
	/**
	 * Tuple containing the type parameters.  This tuple may be empty.
	 *
	 * This member is guaranteeed to be non-NULL.
	 */
	struct pancl_tuple tuple;
};

#endif /* H_PANCL_TYPES_CUSTOM */
// vim:ts=4:sw=4:autoindent
