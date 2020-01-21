/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_TABLE_DATA
#define H_PANCL_TYPES_TABLE_DATA

#include <stddef.h>

#include "pancl/types/location.h"

struct pancl_entry;

/**
 * Represents table data:
 *   [table_name]
 *   ...
 * Or
 *   { ... }
 */
struct pancl_table_data {
	/**
	 * Where this data was found in the input.
	 */
	struct pancl_location loc;
	/**
	 * Number of entries in the table.
	 */
	size_t count;
	/**
	 * Entries in the table.
	 *
	 * If count != 0, then this is guaranteed to be non-NULL.
	 *
	 * If this is non-NULL then each pointer pointed to is guaranteed to also
	 * be non-NULL.
	 */
	struct pancl_entry **entries;
};

#endif /* H_PANCL_TYPES_TABLE_DATA */
// vim:ts=4:sw=4:autoindent
