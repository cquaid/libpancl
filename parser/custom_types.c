/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"

#include "internal.h"
#include "parser/custom_types.h"
#include "parser/str_to_int.h"

static int
handle_int(struct pancl_value *value, enum pancl_type type)
{
	int err = PANCL_ERROR_ARG_INVALID;
	union pancl_type_union new_data;
	const char *str = NULL;
	int base = 0;

	struct pancl_tuple *tuple = &(value->data.custom.args);

	/* Validate the tuple. */
	if (tuple->count < 1 || tuple->count > 2)
		return PANCL_ERROR_OPT_INT_ARG_COUNT;

	/* [Arg 0] Validate and grab the string portion */
	if (tuple->items[0]->type != PANCL_TYPE_STRING)
		return PANCL_ERROR_OPT_INT_ARG_0_NOT_STRING;

	str = tuple->items[0]->data.string;

	/* [Arg 1] Validate and grab the optional base portion */
	if (tuple->count == 2) {
		if (tuple->items[1]->type != PANCL_TYPE_INTEGER)
			return PANCL_ERROR_OPT_INT_ARG_1_NOT_INT;

		base = (int)tuple->items[1]->data.integer;
	}

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

static int
handle_array(struct pancl_value *value)
{
	struct pancl_tuple *tuple = &(value->data.custom.args);
	struct pancl_array array;

	if (tuple->count == 0) {
		pancl_value_fini(value);
		pancl_value_init(value, PANCL_TYPE_ARRAY);
		return PANCL_SUCCESS;
	}

	/* Validate that all members are of the same type. */
	{
		size_t i;
		enum pancl_type type = tuple->items[0]->type;

		for (i = 1; i < tuple->count; ++i) {
			if (type != tuple->items[i]->type)
				return PANCL_ERROR_ARRAY_MEMBER_TYPE;
		}
	}

	pancl_array_init(&array);
	array.count = tuple->count;
	array.items = tuple->items;

	/* Clear these so they don't get cleaned up by pancl_value_fini(). */
	tuple->count = 0;
	tuple->items = NULL;

	pancl_value_fini(value);
	pancl_value_init(value, PANCL_TYPE_ARRAY);
	value->data.array = array;

	return PANCL_SUCCESS;
}

static int
handle_tuple(struct pancl_value *value)
{
	struct pancl_tuple tuple;

	tuple = value->data.custom.args;

	pancl_value_fini(value);
	pancl_value_init(value, PANCL_TYPE_TUPLE);
	value->data.tuple = tuple;

	return PANCL_SUCCESS;
}

static int
handle_table(struct pancl_value *value)
{
	int ret = PANCL_SUCCESS;

	size_t i;
	struct pancl_table_data table;
	struct pancl_tuple *tuple = &(value->data.custom.args);

	/* Count must be a multiple of 2 (key+value pairs) */
	if ((tuple->count % 2) != 0)
		return PANCL_ERROR_INLINE_TABLE_COUNT;

	pancl_table_data_init(&table);

	for (i = 0; i < tuple->count; i += 2) {
		int err;
		struct pancl_entry *entry;

		/* Key type must always be "string". */
		if (tuple->items[i]->type != PANCL_TYPE_STRING) {
			ret = PANCL_ERROR_INLINE_TABLE_KEY_NOT_STRING;
			goto out;
		}

		ret = pancl_entry_new(&entry);

		if (ret != PANCL_SUCCESS)
			goto out;

		/* First item is the string key. Take it's value as the new entry
		 * name.
		 */
		entry->name = tuple->items[i]->data.string;
		tuple->items[i]->data.string = NULL;

		/* Second is the associated data. */
		{
			struct pancl_value *v = tuple->items[i + 1];
			tuple->items[i + 1] = NULL;

			entry->data = *v;
			/* Don't fini/destroy the value since we copied it's data, just
			 * gotta free the memory.
			 */
			free(v);
		}

		err = pancl_table_data_append(&table, entry);

		if (err != PANCL_SUCCESS) {
			pancl_entry_destroy(&entry);
			goto out;
		}
	}

	/* Success */
	ret = PANCL_SUCCESS;

	pancl_value_fini(value);
	pancl_value_init(value, PANCL_TYPE_TABLE);
	value->data.table = table;

out:
	if (ret != PANCL_SUCCESS)
		pancl_table_data_fini(&table);

	return ret;
}

int
handle_known_custom_types(struct pancl_value *value)
{
	const char *name = value->data.custom.name;

	if (strcmp(name, "::Integer") == 0)
		return handle_int(value, PANCL_TYPE_INTEGER);

	if (strcmp(name, "::Int8") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT8);

	if (strcmp(name, "::Uint8") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT8);

	if (strcmp(name, "::Int16") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT16);

	if (strcmp(name, "::Uint16") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT16);

	if (strcmp(name, "::Int32") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT32);

	if (strcmp(name, "::Uint32") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT32);

	if (strcmp(name, "::Int64") == 0)
		return handle_int(value, PANCL_TYPE_OPT_INT64);

	if (strcmp(name, "::Uint64") == 0)
		return handle_int(value, PANCL_TYPE_OPT_UINT64);

	if (strcmp(name, "::Array") == 0)
		return handle_array(value);

	if (strcmp(name, "::Tuple") == 0)
		return handle_tuple(value);

	if (strcmp(name, "::Table") == 0)
		return handle_table(value);

#if 0 /* XXX: Implement at some point. */
	if (strcmp(name, "::Float") == 0)
		return handle_float(value);
#endif

	/* Unhandled, just let the end-user handle it. */
	return PANCL_SUCCESS;
}

// vim:ts=4:sw=4:autoindent
