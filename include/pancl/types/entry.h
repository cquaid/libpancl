/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_ENTRY
#define H_PANCL_TYPES_ENTRY

#include "pancl/types/location.h"
#include "pancl/types/value.h"

struct pancl_utf8_string;

/**
 * Entry in a table (key-value pair).
 */
struct pancl_entry {
	struct pancl_location loc;      /**< Where found in the input */
	struct pancl_utf8_string *name; /**< Name (key) of the entry (non-NULL) */
	struct pancl_value value;       /**< Associated value (non-NULL) */
};

/**
 * Cleans up the memory associated with a pancl_entry.
 *
 * @param[in,out] entry   Pointer to the pointer to the entry to clean up
 *
 * @note The dereference of @p entry will be set to NULL on return.
 */
void pancl_entry_destroy(struct pancl_entry **entry);

#endif /* H_PANCL_TYPES_ENTRY */
// vim:ts=4:sw=4:autoindent
