/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL
#define H_PANCL

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct pancl_parse_operations;

/**
 * Storage for the line/column position of the start of any given data.
 */
struct pancl_pos {
	unsigned long column; /**< Column number (0-based) of the current line */
	unsigned long line; /**< Line number (0-based) */
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

/**
 * Static initializer for a pancl_context structure.
 */
#define PANCL_CONTEXT_INIT \
	{ \
		.ops_data = NULL, \
		.ops = NULL, \
		.buffer_size = 0, \
		.allocated_buffer = NULL, \
		.cursor = NULL, \
		.end = NULL, \
		.pos = { .column = 0, .line = 0 }, \
		.error_pos = { .column = 0, .line = 0 }, \
		.end_of_input = 0, \
		.token1 = NULL \
	}

/**
 * Initialize a pancl_context structure to a state that may be safely passed to
 * pancl_context_fini() and all other functions that accept a pancl_context.
 *
 * @param[in] ctx  Context to initialize
 */
void pancl_context_init(struct pancl_context *ctx);
/**
 * Cleans up all resources associated with a pancl_context.
 *
 * @param[in] ctx  Context to clean up
 *
 * @note It is safe to call this function multiple times on the same context.
 */
void pancl_context_fini(struct pancl_context *ctx);

/**
 * Start parsing a PanCL file.
 *
 * @param[in] ctx    Context to initialize and store parsing state in
 * @param[in] file   File to parse
 *
 * @retval PANCL_SUCCESS   Setup successful
 * @retval PANCL_ERROR_*   Something went wrong
 *
 * @note
 *   @p file should remain open until pancl_context_fini() has been called
 *   on @p ctx.
 */
int pancl_parse_file(struct pancl_context *ctx, FILE *file);
/**
 * Start parsing PanCL data from a string.
 *
 * @param[in] ctx      Context to initialize and store parsing state in
 * @param[in] string   String to parse
 *
 * @retval PANCL_SUCCESS   Setup successful
 * @retval PANCL_ERROR_*   Something went wrong
 *
 * @note
 *   @p string should remain available until pancl_context_fini() has been
 *   called on @p ctx.
 */
int pancl_parse_string(struct pancl_context *ctx, const char *string);
/**
 * Start parsing PanCL data from an arbitrary buffer.
 *
 * @param[in] ctx      Context to initialize and store parsing state in
 * @param[in] buffer   Buffer to parse
 * @param[in] size     Size of @p buffer in bytes
 *
 * @retval PANCL_SUCCESS   Setup successful
 * @retval PANCL_ERROR_*   Something went wrong
 *
 * @note
 *   @p buffer should remain available until pancl_context_fini() has been
 *   called on @p ctx.
 */
int pancl_parse_buffer(struct pancl_context *ctx, const void *buffer,
		size_t size);

/**
 * Retrieves a constant string equivalent of the given error code.
 *
 * @param[in] pancl_error_code   Error code to decode
 *
 * @return Returns an error string.
 * @note This function never returns NULL.
 */
const char *pancl_strerror(int pancl_error_code);

/**
 * Set the allocation functions for the entire PanCL library.
 *
 * @param[in] alloc_fn     Basic allocation function (malloc)
 * @param[in] realloc_fn   Specialized reallocation function (realloc)
 * @param[in] free_fn      Cleanup function (free)
 *
 * @note All functions must be specified.
 * @note DEFINITELY NOT THREAD SAFE
 *
 * @retval PANCL_SUCCESS             Success
 * @retval PANCL_ERROR_ARG_INVALID   A NULL parameter was given
 */
int pancl_lib_set_allocators(void *(*alloc_fn)(size_t),
		void *(*realloc_fn)(void *, size_t),
		void(*free_fn)(void *));

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
	int boolean;                   /**< PANCL_TYPE_BOOLEAN */
	double floating;               /**< PANCL_TYPE_FLOATING */
	int_least32_t integer;         /**< PANCL_TYPE_INTEGER */
	char *string;                  /**< PANCL_TYPE_STRING (non-NULL) */
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
 * Parse the next table from a pancl_context and return its data.
 *
 * @param[in] ctx      Context attached to some form of input (pancl_parse_*)
 * @param[out] table   Location to store the parsed table data.
 *
 * @retval PANCL_SUCCESS       Successful parse, @p table contains valid data
 * @retval PANCL_ERROR_*       Something failed
 * @retval PANCL_END_OF_INPUT  No more tables to return; parsing complete
 */
int pancl_get_table(struct pancl_context *ctx, struct pancl_table *table);

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

/**
 * Cleans up the memory associated with a pancl_entry.
 *
 * @param[in,out] entry   Pointer to the pointer to the entry to clean up
 *
 * @note The dereference of @p entry will be set to NULL on return.
 */
void pancl_entry_destroy(struct pancl_entry **entry);
/**
 * Cleans up the memory associated with a pancl_value.
 *
 * @param[in,out] value   Pointer to the pointer to the value to clean up
 *
 * @note The dereference of @p value will be set to NULL on return.
 */
void pancl_value_destroy(struct pancl_value **value);


#if defined(__cplusplus)
}
#endif

#endif /* H_PANCL */
// vim:ts=4:sw=4:autoindent
