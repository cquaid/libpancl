/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL
#define H_PANCL

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>

#include "pancl/pancl_error.h"
#include "pancl/types/array.h"
#include "pancl/types/custom.h"
#include "pancl/types/entry.h"
#include "pancl/types/location.h"
#include "pancl/types/table.h"
#include "pancl/types/table_data.h"
#include "pancl/types/tuple.h"
#include "pancl/types/utf8_string.h"
#include "pancl/types/value.h"

struct pancl_parse_operations;

struct pancl_context {
	void *ops_data; /**< Operations user data */
	const struct pancl_parse_operations *ops; /**< Parsing operations */

	size_t buffer_size; /**< Size of the allocated buffer */
	void *allocated_buffer; /**< If the buffer was allocated, this is set */

	const char *cursor; /**< Current position in the buffer. */
	const char *end; /**< End pointer. */

	struct pancl_location loc; /**< Column/line number */
	/* Note: Error data is not exact. */
	struct pancl_location error_loc; /**< Error column/line number */
	struct pancl_utf8_string *error_token; /**< Error token (can be NULL) */

	int end_of_input; /**< No more input data available */
	void *token1; /**< Internal use */
};

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
int pancl_get_next_table(struct pancl_context *ctx, struct pancl_table *table);

#if defined(__cplusplus)
}
#endif

#endif /* H_PANCL */
// vim:ts=4:sw=4:autoindent
