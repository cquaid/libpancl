/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_INTERNAL
#define H_PANCL_INTERNAL

#include <stddef.h>

#include "pancl.h"

struct pancl_parse_operations {
	/**
	 * Called when constructing the pancl_context.  Used if the operations need
	 * any initialization to work correctly.
	 *
	 * @param[in] ops_data   Arbitrary user data
	 *
	 * @return Returns 0 on success and non-zero on failure.
	 */
	int (*init)(void *ops_data);

	/**
	 * Get more data from whatever the backend is.
	 *
	 * @param[in] ops_data   Arbitrary user data
	 * @param[in] store      Buffer to store data in
	 * @param[in,out] size   Size of @p store and storage for the amount read
	 *
	 * @return Returns 0 on success and non-zero on failure.
	 *
	 * @note
	 *   Returning 0 in @p size with a return value of 0 means no more data
	 *   remains to be read (i.e. EOF).
	 */
	int (*next)(void *ops_data, void *store, size_t *size);

	/**
	 * Called during pancl_context cleanup.  Used to clean up anything related
	 * to the operations.
	 *
	 * @param[in] ops_data   Arbitrary user data
	 */
	void (*fini)(void *ops_data);
};

void pancl_array_init(struct pancl_array *array);
void pancl_array_fini(struct pancl_array *array);
int pancl_array_append(struct pancl_array *array, struct pancl_value *item);

void pancl_tuple_init(struct pancl_tuple *tuple);
void pancl_tuple_fini(struct pancl_tuple *tuple);
int pancl_tuple_append(struct pancl_tuple *tuple, struct pancl_value *item);

void pancl_custom_init(struct pancl_custom *custom);
void pancl_custom_fini(struct pancl_custom *custom);

void pancl_table_data_init(struct pancl_table_data *td);
void pancl_table_data_fini(struct pancl_table_data *td);
int pancl_table_data_append(struct pancl_table_data *td,
		struct pancl_entry *entry);

void pancl_value_init(struct pancl_value *value, enum pancl_type type);
int pancl_value_new(struct pancl_value **value, enum pancl_type type);
void pancl_value_destroy(struct pancl_value **value);

void pancl_entry_init(struct pancl_entry *entry);
int pancl_entry_new(struct pancl_entry **entry);
void pancl_entry_destroy(struct pancl_entry **entry);

#endif /* H_PANCL_INTERNAL */
// vim:ts=4:sw=4:autoindent
