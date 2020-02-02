/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_VALUE
#define H_PANCL_TYPES_VALUE

#include <stdint.h>

#include "pancl/types/array.h"
#include "pancl/types/custom.h"
#include "pancl/types/location.h"
#include "pancl/types/table_data.h"
#include "pancl/types/tuple.h"
#include "pancl/types/utf8_string.h"

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
	struct pancl_array array;         /**< PANCL_TYPE_ARRAY */
	struct pancl_custom custom;       /**< PANCL_TYPE_CUSTOM */
	int boolean;                      /**< PANCL_TYPE_BOOLEAN */
	double floating;                  /**< PANCL_TYPE_FLOATING */
	int_least32_t integer;            /**< PANCL_TYPE_INTEGER */
	struct pancl_utf8_string *string; /**< PANCL_TYPE_STRING (non-NULL) */
	struct pancl_table_data table;    /**< PANCL_TYPE_TABLE */
	struct pancl_tuple tuple;         /**< PANCL_TYPE_TUPLE */
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
	struct pancl_location loc; /**< Where found in the input */
	union pancl_type_union data; /**< The actual data */
	enum pancl_type type; /**< Type of data this value represents */
};

/**
 * Cleans up the memory associated with a pancl_value.
 *
 * @param[in,out] value   Pointer to the pointer to the value to clean up
 *
 * @note The dereference of @p value will be set to NULL on return.
 */
void pancl_value_destroy(struct pancl_value **value);

#endif /* H_PANCL_TYPES_VALUE */
// vim:ts=4:sw=4:autoindent
