/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl/pancl.h"

#include "internal.h"
#include "parser/custom_types.h"
#include "parser/str_to_int.h"

static int
handle_int(struct pancl_value *value, enum pancl_type type)
{
	int err = PANCL_ERROR_ARG_INVALID;
	union pancl_type_union new_data;
	const char *str = NULL;
	struct pancl_utf8_string *utf8_str = NULL;
	int base = 0;

	struct pancl_tuple *tuple = &(value->data.custom.tuple);

	/* Validate the tuple. */
	if (tuple->count < 1 || tuple->count > 2)
		return PANCL_ERROR_OPT_INT_ARG_COUNT;

	/* [Arg 0] Validate and grab the string portion */
	if (tuple->values[0]->type != PANCL_TYPE_STRING)
		return PANCL_ERROR_OPT_INT_ARG_0_NOT_STRING;

	utf8_str = tuple->values[0]->data.string;

	/* [Arg 1] Validate and grab the optional base portion */
	if (tuple->count == 2) {
		if (tuple->values[1]->type != PANCL_TYPE_INTEGER)
			return PANCL_ERROR_OPT_INT_ARG_1_NOT_INT;

		base = (int)tuple->values[1]->data.integer;
	}

	/* Make sure the string is in a good format (ascii and no embedded NULs) */
	if (!pancl_utf8_string_is_ascii(utf8_str))
		return PANCL_ERROR_STR_TO_INT_CHAR;

	if (pancl_utf8_string_contains_nul(utf8_str))
		return PANCL_ERROR_STR_TO_INT_CHAR;

	str = utf8_str->data;

	switch (type) {
	case PANCL_TYPE_INTEGER:
		err = str_to_int32(&(new_data.integer), str, base);
		break;

	case PANCL_TYPE_OPT_INT8:
		err = str_to_int8(&(new_data.opt.int8), str, base);
		break;

	case PANCL_TYPE_OPT_UINT8:
		err = str_to_uint8(&(new_data.opt.uint8), str, base);
		break;

	case PANCL_TYPE_OPT_INT16:
		err = str_to_int16(&(new_data.opt.int16), str, base);
		break;

	case PANCL_TYPE_OPT_UINT16:
		err = str_to_uint16(&(new_data.opt.uint16), str, base);
		break;

	case PANCL_TYPE_OPT_INT32:
		err = str_to_int32(&(new_data.opt.int32), str, base);
		break;

	case PANCL_TYPE_OPT_UINT32:
		err = str_to_uint32(&(new_data.opt.uint32), str, base);
		break;

	case PANCL_TYPE_OPT_INT64:
		err = str_to_int64(&(new_data.opt.int64), str, base);
		break;

	case PANCL_TYPE_OPT_UINT64:
		err = str_to_uint64(&(new_data.opt.uint64), str, base);
		break;

	default:
		break;
	}

	if (err == PANCL_SUCCESS) {
		/* Clean up the old value */
		pancl_value_fini(value);

		/* Set the new value. */
		pancl_value_init(value, type);
		value->data = new_data;
	}

	return err;
}


int
handle_known_custom_types(struct pancl_value *value)
{
	struct pancl_utf8_string *name = value->data.custom.name;

	/* Custom type names come from Raw Identifiers so they can
	 * only ever be ASCII but we validate the assumption regardless.
	 */
	if (!pancl_utf8_string_is_ascii(name))
		return PANCL_SUCCESS;

	if (pancl_utf8_string_contains_nul(name))
		return PANCL_SUCCESS;

	if (strcmp(name->data, "::Integer") == 0)
		return handle_int(value, PANCL_TYPE_INTEGER);

	if (strcmp(name->data, "::Int8") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT8);

	if (strcmp(name->data, "::Uint8") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT8);

	if (strcmp(name->data, "::Int16") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT16);

	if (strcmp(name->data, "::Uint16") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT16);

	if (strcmp(name->data, "::Int32") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT32);

	if (strcmp(name->data, "::Uint32") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT32);

	if (strcmp(name->data, "::Int64") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT64);

	if (strcmp(name->data, "::Uint64") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT64);

#if 0 /* XXX: Implement at some point. */
	if (strcmp(name->data, "::Float") == 0)
		return handle_float(value);
#endif

	/* Unhandled, just let the end-user handle it. */
	return PANCL_SUCCESS;
}

// vim:ts=4:sw=4:autoindent
