/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_TABLE
#define H_PANCL_TYPES_TABLE

#include "pancl/types/location.h"
#include "pancl/types/table_data.h"

/**
 * Top-level table:
 *  [ name ]
 *  key = value
 *  ...
 */
struct pancl_table {
	/**
	 * Location (column and line number) of where the table was found in the
	 * input.
	 */
	struct pancl_location loc;
	/**
	 * Name of the table.  Note that the very first table in a file may be NULL
	 * which represents values in the root table instead of those under a table
	 * header ([...])
	 */
	char *name;
	/**
	 * Data associated with the table.
	 * May be NULL.
	 */
	struct pancl_table_data data;
};

/**
 * Initialize a pancl_table structure to a state that may be safely passed
 * to pancl_table_fini().
 *
 * @param[in] table   Table to initialize
 */
void pancl_table_init(struct pancl_table *table);
/**
 * Cleans up all the resources associated with a pancl_table.
 *
 * @param[in] table   Table to clean up
 *
 * @note It is safe to call this function multiple times with the same table.
 */
void pancl_table_fini(struct pancl_table *table);

#endif /* H_PANCL_TYPES_TABLE */
// vim:ts=4:sw=4:autoindent
