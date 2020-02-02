/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_INTERNAL
#define H_PANCL_INTERNAL

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#include "pancl/pancl.h"

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

/* types/array.c */
void pancl_array_init(struct pancl_array *array);
void pancl_array_fini(struct pancl_array *array);
int pancl_array_append(struct pancl_array *array, struct pancl_value *value);

/* types/tuple.c */
void pancl_tuple_init(struct pancl_tuple *tuple);
void pancl_tuple_fini(struct pancl_tuple *tuple);
int pancl_tuple_append(struct pancl_tuple *tuple, struct pancl_value *value);

/* types/custom.c */
void pancl_custom_init(struct pancl_custom *custom);
void pancl_custom_fini(struct pancl_custom *custom);

/* types/table_data.c */
void pancl_table_data_init(struct pancl_table_data *td);
void pancl_table_data_fini(struct pancl_table_data *td);
int pancl_table_data_append(struct pancl_table_data *td,
		struct pancl_entry *entry);

/* types/utf8_string.c */
int pancl_utf8_string_new(struct pancl_utf8_string **string, size_t bytes);

/* types/value.c */
int pancl_value_new(struct pancl_value **value, enum pancl_type type);
void pancl_value_init(struct pancl_value *value, enum pancl_type type);
void pancl_value_fini(struct pancl_value *value);

/* types/entry.c */
int pancl_entry_new(struct pancl_entry **entry);
void pancl_entry_init(struct pancl_entry *entry);
void pancl_entry_fini(struct pancl_entry *entry);

/* overflow.c */
int safe_add(size_t a, size_t b, size_t *r);
int safe_mul(size_t a, size_t b, size_t *r);

static inline bool
can_inc(size_t v)
{
	return (v != SIZE_MAX);
}

/* memory.c */
void pancl_free(void *p);
void *pancl_alloc(size_t n);
void *pancl_zalloc(size_t n);
void *pancl_realloc(void *p, size_t n);
int pancl_resize(void **p, size_t n, size_t count);

#endif /* H_PANCL_INTERNAL */
// vim:ts=4:sw=4:autoindent
