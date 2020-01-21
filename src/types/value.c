/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl/pancl.h"
#include "internal.h"

void
pancl_value_init(struct pancl_value *value, enum pancl_type type)
{
	if (value == NULL)
		return;

	memset(value, 0, sizeof(*value));
	value->type = type;

	switch (type) {
	case PANCL_TYPE_ARRAY:
		pancl_array_init(&(value->data.array));
		break;

	case PANCL_TYPE_CUSTOM:
		pancl_custom_init(&(value->data.custom));
		break;

	case PANCL_TYPE_BOOLEAN:
		value->data.boolean = 0;
		break;

	case PANCL_TYPE_FLOATING:
		value->data.floating = 0.0;
		break;

	case PANCL_TYPE_INTEGER:
		value->data.integer = 0;
		break;

	case PANCL_TYPE_STRING:
		value->data.string = NULL;
		break;

	case PANCL_TYPE_TABLE:
		pancl_table_data_init(&(value->data.table));
		break;

	case PANCL_TYPE_TUPLE:
		pancl_tuple_init(&(value->data.tuple));
		break;

	case PANCL_TYPE_OPT_INT8:
		value->data.opt.int8 = 0;
		break;

	case PANCL_TYPE_OPT_UINT8:
		value->data.opt.uint8 = 0;
		break;

	case PANCL_TYPE_OPT_INT16:
		value->data.opt.int16 = 0;
		break;

	case PANCL_TYPE_OPT_UINT16:
		value->data.opt.uint16 = 0;
		break;

	case PANCL_TYPE_OPT_INT32:
		value->data.opt.int32 = 0;
		break;

	case PANCL_TYPE_OPT_UINT32:
		value->data.opt.uint32 = 0;
		break;

	case PANCL_TYPE_OPT_INT64:
		value->data.opt.int64 = 0;
		break;

	case PANCL_TYPE_OPT_UINT64:
		value->data.opt.uint64 = 0;
		break;
	}
}

int
pancl_value_new(struct pancl_value **value, enum pancl_type type)
{
	if (value == NULL)
		return PANCL_ERROR_ARG_INVALID;

	*value = pancl_alloc(sizeof(**value));

	if (*value == NULL)
		return PANCL_ERROR_ALLOC;

	pancl_value_init(*value, type);
	return PANCL_SUCCESS;
}

void
pancl_value_fini(struct pancl_value *value)
{
	if (value == NULL)
		return;

	switch (value->type) {
	case PANCL_TYPE_ARRAY:
		pancl_array_fini(&(value->data.array));
		break;

	case PANCL_TYPE_CUSTOM:
		pancl_custom_fini(&(value->data.custom));
		break;

	case PANCL_TYPE_BOOLEAN:
	case PANCL_TYPE_FLOATING:
	case PANCL_TYPE_INTEGER:
	case PANCL_TYPE_OPT_INT8:
	case PANCL_TYPE_OPT_UINT8:
	case PANCL_TYPE_OPT_INT16:
	case PANCL_TYPE_OPT_UINT16:
	case PANCL_TYPE_OPT_INT32:
	case PANCL_TYPE_OPT_UINT32:
	case PANCL_TYPE_OPT_INT64:
	case PANCL_TYPE_OPT_UINT64:
		break;

	case PANCL_TYPE_STRING:
		pancl_free(value->data.string);
		break;

	case PANCL_TYPE_TABLE:
		pancl_table_data_fini(&(value->data.table));
		break;

	case PANCL_TYPE_TUPLE:
		pancl_tuple_fini(&(value->data.tuple));
		break;
	}

	pancl_value_init(value, value->type);
}

void
pancl_value_destroy(struct pancl_value **value)
{
	if (value == NULL || *value == NULL)
		return;

	pancl_value_fini(*value);
	pancl_free(*value);
	*value = NULL;
}

// vim:ts=4:sw=4:autoindent
