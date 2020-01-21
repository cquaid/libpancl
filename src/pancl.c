/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "pancl/pancl.h"
#include "lexer/token.h"

/**
 * @file pancl.c
 * @brief Contains setup/cleanup functions for the pancl_context.
 */


/**
 * Size of buffer to allocate for reading.
 */
#define DEFAULT_BUFFER_SIZE  (8192)


/**
 * A do-nothing read operation for cases where the input is a single buffer
 * and there is no more data to read.
 *
 * @param[in] ops_data   Operation-specific data (Unused)
 * @param[out] store     Buffer to read into (Unused)
 * @param[in,out] size   Size of @p store and the returned read size
 *
 * @return Always returns 0 and sets @p size to 0.
 *
 * @see buffer_parse_ops
 */
static int
buffer_next(void *ops_data, void *store, size_t *size)
{
	(void)ops_data;
	(void)store;

	/* Definition states that setting size to 0 and returning 0 means there's
	 * no more data to process.
	 */
	*size = 0;
	return 0;
}

/**
 * Parsing operations for buffers.
 */
static const struct pancl_parse_operations buffer_parse_ops = {
	.init = NULL,
	.next = buffer_next,
	.fini = NULL
};


/**
 * Initialization operation for reading from a FILE pointer.
 *
 * @param[in] ops_data   Opened FILE*
 *
 * @return Returns 0 on success and non-zero on failure.
 *
 * @see file_parse_ops
 */
static int
file_init(void *ops_data)
{
	FILE *f = ops_data;

	rewind(f); /* Also clears the stream error indicator (clearerr()). */
	return 0;
}


/**
 * FILE read operation.
 *
 * @param[in] ops_data   Opened FILE* to read from
 * @param[out] store     Buffer to read into
 * @param[in,out] size   Size of @p store and the returned read size
 *
 * @return
 *   Returns 0 on success and non-zero on failure.  Upon successful return,
 *   The value stored in @p size is updated to reflect the number of bytes
 *   read into @p store.
 *
 * @see file_parse_ops
 */
static int
file_next(void *ops_data, void *store, size_t *size)
{
	size_t ret;
	FILE *f = ops_data;

	ret = fread(store, 1, *size, f);

	if (ferror(f))
		return -EIO;

	/* On feof() ret will be 0 or < *size.  So we need to wait until the next
	 * request if we get a short read which will end up being 0 and thus the
	 * EOF for this function.
	 */
	*size = ret;
	return 0;
}

/**
 * Parse operations for reading from a FILE pointer.
 */
static const struct pancl_parse_operations file_parse_ops = {
	.init = file_init,
	.next = file_next,
	.fini = NULL
};


/**
 * Common backend setup function for a pancl_context.
 *
 * @param[out] ctx           The context to initialize
 * @param[in] ops            Parse operations to use
 * @param[in] ops_data       Operation-specific data
 * @param[in] alloc_buffer   If a read buffer needs to be allocated
 *
 * @retval PANCL_SUCCESS             Success
 * @retval PANCL_ERROR_ARG_INVALID   Invalid parameter
 * @retval PANCL_ERROR_ALLOC         Allocation failure
 * @retval PANCL_ERROR_CTX_INIT      ops->init failed
 */
static int
pancl_context_setup(struct pancl_context *ctx,
	const struct pancl_parse_operations *ops, void *ops_data,
	bool alloc_buffer)
{
	int err;

	if (ctx == NULL || ops == NULL || ops->next == NULL)
		return PANCL_ERROR_ARG_INVALID;

	pancl_context_init(ctx);

	ctx->ops = ops;
	ctx->ops_data = ops_data;

	if (alloc_buffer) {
		ctx->allocated_buffer = pancl_alloc(DEFAULT_BUFFER_SIZE);

		if (ctx->allocated_buffer == NULL)
			return PANCL_ERROR_ALLOC;

		ctx->buffer_size = DEFAULT_BUFFER_SIZE;
	}

	if (ops->init != NULL)
		err = ops->init(ops_data);
	else
		err = 0;

	if (err != 0) {
		pancl_free(ctx->allocated_buffer);
		ctx->allocated_buffer = NULL;
		return PANCL_ERROR_CTX_INIT;
	}

	return PANCL_SUCCESS;
}


/**
 * Sets up a pancl_context to parse from a FILE pointer.
 *
 * @param[out] ctx   The context to initialize
 * @param[in] file   Opened FILE pointer to be read from
 *
 * @retval PANCL_SUCCESS             Success
 * @retval PANCL_ERROR_ARG_INVALID   Invalid parameter
 * @retval PANCL_ERROR_*             Other failures
 */
int
pancl_parse_file(struct pancl_context *ctx, FILE *file)
{
	if (file == NULL)
		return PANCL_ERROR_ARG_INVALID;

	return pancl_context_setup(ctx, &file_parse_ops, file, true);
}


/**
 * Sets up a pancl_context to parse from a buffer.
 *
 * @param[out] ctx     The context to initialize
 * @param[in] buffer   The buffer to be read from
 * @param[in] size     Length of @p buffer in bytes
 *
 * @retval PANCL_SUCCESS             Success
 * @retval PANCL_ERROR_ARG_INVALID   Invalid parameter
 * @retval PANCL_ERROR_*             Other failures
 */
int
pancl_parse_buffer(struct pancl_context *ctx, const void *buffer, size_t size)
{
	int err;

	if (buffer == NULL)
		return PANCL_ERROR_ARG_INVALID;

	err = pancl_context_setup(ctx, &buffer_parse_ops, NULL, false);

	if (err == PANCL_SUCCESS) {
		/* Set these after setup since setup NULLs them. */
		ctx->cursor = buffer;
		ctx->end = ctx->cursor + size;
	}

	return err;
}

/**
 * Sets up a pancl_context to parse from a NUL-terminated string.
 *
 * @param[out] ctx     The context to initialize
 * @param[in] string   The string to be read from
 *
 * @retval PANCL_SUCCESS             Success
 * @retval PANCL_ERROR_ARG_INVALID   Invalid parameter
 * @retval PANCL_ERROR_*             Other failures
 */
int
pancl_parse_string(struct pancl_context *ctx, const char *string)
{
	if (string == NULL)
		return PANCL_ERROR_ARG_INVALID;

	return pancl_parse_buffer(ctx, string, strlen(string) + 1);
}


/**
 * Initialize a pancl_context so that it may be safely passed to
 * any function accepting a pancl_context.
 *
 * @param[out] ctx   The context to initialize
 */
void
pancl_context_init(struct pancl_context *ctx)
{
	if (ctx != NULL)
		memset(ctx, 0, sizeof(*ctx));
}

/**
 * Cleans up a pancl_context.
 *
 * @param[out] ctx   The context to clean up
 *
 * @note
 *   This function is safe to call once pancl_context_init() has been used to
 *   initialize the context.
 */
void
pancl_context_fini(struct pancl_context *ctx)
{
	if (ctx == NULL)
		return;

	/* Trigger fini, if one is set. */
	if (ctx->ops != NULL && ctx->ops->fini != NULL)
		ctx->ops->fini(ctx->ops_data);

	pancl_free(ctx->error_token);
	pancl_free(ctx->allocated_buffer);

	if (ctx->token1 != NULL) {
		token_fini(ctx->token1);
		pancl_free(ctx->token1);
	}

	/* Reset all the fields. */
	pancl_context_init(ctx);
}

// vim:ts=4:sw=4:autoindent
