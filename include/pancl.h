/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL
#define H_PANCL

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct pancl_parse_operations;

struct pancl_pos {
	unsigned long column;
	unsigned long line;
};

struct pancl_context {
	void *ops_data; /**< Operations user data */
	const struct pancl_parse_operations *ops; /**< Parsing operations */

	size_t buffer_size; /**< Size of the allocated buffer */
	void *allocated_buffer; /**< If the buffer was allocated, this is set */

	const char *cursor; /**< Current position in the buffer. */
	const char *end; /**< End pointer. */

	struct pancl_pos pos; /**< Column/line number */
	struct pancl_pos error_pos; /**< Error column/line number */

	int end_of_input; /**< No more input data available */
	void *token1; /**< Internal use */
};

void pancl_context_init(struct pancl_context *ctx);
void pancl_context_fini(struct pancl_context *ctx);

int pancl_parse_file(struct pancl_context *ctx, FILE *file);
int pancl_parse_string(struct pancl_context *ctx, const char *string);
int pancl_parse_buffer(struct pancl_context *ctx, const void *buffer,
		size_t size);


struct pancl_value;
struct pancl_entry;

/**
 * Represents an array: [ ... ]
 */
struct pancl_array {
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

/**
 * Represents a tuple: ( ... )
 */
struct pancl_tuple {
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

/**
 * Represents a custom type: name( ... )
 */
struct pancl_custom {
	/**
	 * Name of the custom type.
	 *
	 * Guaranteed to be non-NULL and non-empty.
	 */
	char *name;
	/**
	 * Tuple containing the type parameters.  This tuple may be empty.
	 *
	 * This member is guaranteeed to be non-NULL.
	 */
	struct pancl_tuple tuple;
};

/**
 * Represents table data:
 *   [table_name]
 *   ...
 * Or
 *   { ... }
 */
struct pancl_table_data {
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

enum pancl_type {
	/**
	 * Type is an array.
	 * Uses pancl_type_union::array
	 */
	PANCL_TYPE_ARRAY,
	/**
	 * Type is a boolean.
	 * Uses pancl_type_union::boolean.
	 *
	 * 0 == false; 1 == true
	 */
	PANCL_TYPE_BOOLEAN,
	/**
	 * Type is custom.
	 * Uses pancl_type_union::custom.
	 */
	PANCL_TYPE_CUSTOM,
	/**
	 * Type is a floating point number.
	 * Uses pancl_type_union::floating.
	 */
	PANCL_TYPE_FLOATING,
	/**
	 * Type is an integer (signed 32-bit value).
	 * Uses pancl_type_union::integer.
	 */
	PANCL_TYPE_INTEGER,
	/**
	 * Type is a string.
	 * Uses pancl_type_union::string.
	 */
	PANCL_TYPE_STRING,
	/**
	 * Type is a table.
	 * Uses pancl_type_union::table.
	 */
	PANCL_TYPE_TABLE,
	/**
	 * Type is a tuple.
	 * Uses pancl_type_union::tuple.
	 */
	PANCL_TYPE_TUPLE,
	/**
	 * Optional int8 type.
	 * Uses pancl_type_union::opt::int8.
	 */
	PANCL_TYPE_OPT_INT8,
	/**
	 * Optional uint8 type.
	 * Uses pancl_type_union::opt::uint8.
	 */
	PANCL_TYPE_OPT_UINT8,
	/**
	 * Optional int16 type.
	 * Uses pancl_type_union::opt::int16.
	 */
	PANCL_TYPE_OPT_INT16,
	/**
	 * Optional uint16 type.
	 * Uses pancl_type_union::opt::uint16.
	 */
	PANCL_TYPE_OPT_UINT16,
	/**
	 * Optional int32 type.
	 * Uses pancl_type_union::opt::int32.
	 */
	PANCL_TYPE_OPT_INT32,
	/**
	 * Optional uint32 type.
	 * Uses pancl_type_union::opt::uint32.
	 */
	PANCL_TYPE_OPT_UINT32,
	/**
	 * Optional int64 type.
	 * Uses pancl_type_union::opt::int64.
	 */
	PANCL_TYPE_OPT_INT64,
	/**
	 * Optional uint64 type.
	 * Uses pancl_type_union::opt::uint64.
	 */
	PANCL_TYPE_OPT_UINT64
};

/**
 * Union of data types.
 */
union pancl_type_union {
	struct pancl_array array;      /**< PANCL_TYPE_ARRAY */
	struct pancl_custom custom;    /**< PANCL_TYPE_CUSTOM */
	int boolean;                    /**< PANCL_TYPE_BOOLEAN */
	double floating;                /**< PANCL_TYPE_FLOATING */
	int_least32_t integer;          /**< PANCL_TYPE_INTEGER */
	char *string;                   /**< PANCL_TYPE_STRING (non-NULL) */
	struct pancl_table_data table; /**< PANCL_TYPE_TABLE */
	struct pancl_tuple tuple;      /**< PANCL_TYPE_TUPLE */
	union {
		int_least8_t int8;     /**< PANCL_TYPE_OPT_INT8 */
		uint_least8_t uint8;   /**< PANCL_TYPE_OPT_UINT8 */
		int_least16_t int16;   /**< PANCL_TYPE_OPT_INT16 */
		uint_least16_t uint16; /**< PANCL_TYPE_OPT_UINT16 */
		int_least32_t int32;   /**< PANCL_TYPE_OPT_INT32 */
		uint_least32_t uint32; /**< PANCL_TYPE_OPT_UINT32 */
		int_least64_t int64;   /**< PANCL_TYPE_OPT_INT64 */
		uint_least64_t uint64; /**< PANCL_TYPE_OPT_UINT64 */
	} opt; /**< Optional/extended types. */
};

/**
 * Represents a value (type and data).
 */
struct pancl_value {
	enum pancl_type type; /**< Type of data this value represents */
	union pancl_type_union data; /**< The actual data */
};

/**
 * Entry in a table (key-value pair).
 */
struct pancl_entry {
	char *name; /**< Name (key) of the entry (non-NULL) */
	struct pancl_value value; /**< Associated value (non-NULL) */
};

/**
 * Top-level table:
 *  [ name ]
 *  key = value
 *  ...
 */
struct pancl_table {
	/**
	 * Name of the table.  Note that the very first table in a file may be NULL
	 * which represents values in the global table instead of those under a
	 * table header ([...])
	 */
	char *name;
	/**
	 * Data associated with the table.
	 * May be NULL.
	 */
	struct pancl_table_data data;
};

/**
 * @param[out] table
 */
int pancl_get_table(struct pancl_context *ctx, struct pancl_table *table);
void pancl_table_init(struct pancl_table *table);
void pancl_table_fini(struct pancl_table *table);

void pancl_entry_destroy(struct pancl_entry **entry);
void pancl_value_destroy(struct pancl_value **value);

#endif /* H_PANCL */
// vim:ts=4:sw=4:autoindent
