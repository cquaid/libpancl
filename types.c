/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"
#include "internal.h"

void
pancl_array_init(struct pancl_array *array)
{
	if (array != NULL)
		memset(array, 0, sizeof(*array));
}

void
pancl_array_fini(struct pancl_array *array)
{
	if (array == NULL)
		return;

	if (array->items != NULL) {
		size_t i;

		for (i = 0; i < array->count; ++i)
			pancl_value_fini(array->items[i]);

		free(array->items);
	}

	pancl_array_init(array);
}

int
pancl_array_append(struct pancl_array *array, struct pancl_value *item)
{
	void *d;

	if (array == NULL || item == NULL)
		return PANCL_ERROR_ARG_INVALID;

	/* Arrays have to validate that they only hold one type of data. */
	if (array->count != 0) {
		if (array->items[0]->type != item->type)
			return PANCL_ERROR_ARRAY_MEMBER_TYPE;
	}

	d = realloc(array->items, (sizeof(*(array->items)) * (array->count + 1)));

	if (d == NULL)
		return PANCL_ERROR_ALLOC;

	array->items = d;
	array->items[array->count] = item;
	array->count += 1;

	return PANCL_SUCCESS;
}


void
pancl_tuple_init(struct pancl_tuple *tuple)
{
	if (tuple != NULL)
		memset(tuple, 0, sizeof(*tuple));
}

void
pancl_tuple_fini(struct pancl_tuple *tuple)
{
	if (tuple == NULL)
		return;

	if (tuple->items != NULL) {
		size_t i;

		for (i = 0; i < tuple->count; ++i)
			pancl_value_fini(tuple->items[i]);

		free(tuple->items);
	}

	pancl_tuple_init(tuple);
}

int
pancl_tuple_append(struct pancl_tuple *tuple, struct pancl_value *item)
{
	void *d;

	if (tuple == NULL || item == NULL)
		return PANCL_ERROR_ARG_INVALID;

	d = realloc(tuple->items, (sizeof(*(tuple->items)) * (tuple->count + 1)));

	if (d == NULL)
		return PANCL_ERROR_ALLOC;

	tuple->items = d;
	tuple->items[tuple->count] = item;
	tuple->count += 1;

	return PANCL_SUCCESS;
}


void
pancl_custom_init(struct pancl_custom *custom)
{
	if (custom == NULL)
		return;

	memset(custom, 0, sizeof(*custom));
	pancl_tuple_init(&(custom->args));
}

void
pancl_custom_fini(struct pancl_custom *custom)
{
	if (custom == NULL)
		return;

	free(custom->name);
	pancl_tuple_fini(&(custom->args));

	pancl_custom_init(custom);
}


void
pancl_table_data_init(struct pancl_table_data *td)
{
	if (td != NULL)
		memset(td, 0, sizeof(*td));
}

void
pancl_table_data_fini(struct pancl_table_data *td)
{
	if (td == NULL)
		return;

	if (td->entries != NULL) {
		size_t i;

		for (i = 0; i < td->count; ++i)
			pancl_entry_fini(td->entries[i]);

		free(td->entries);
	}

	pancl_table_data_init(td);
}

int
pancl_table_data_append(struct pancl_table_data *td, struct pancl_entry *entry)
{
	void *d;

	if (td == NULL || entry == NULL)
		return PANCL_ERROR_ARG_INVALID;

	d = realloc(td->entries, (sizeof(*(td->entries)) * (td->count + 1)));

	if (d == NULL)
		return PANCL_ERROR_ALLOC;

	td->entries = d;
	td->entries[td->count] = entry;
	td->count += 1;

	return PANCL_SUCCESS;
}


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
		free(value->data.string);
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
pancl_entry_init(struct pancl_entry *entry)
{
	if (entry == NULL)
		return;

	memset(entry, 0, sizeof(*entry));
	pancl_value_init(&(entry->data), PANCL_TYPE_INTEGER);
}

void
pancl_entry_fini(struct pancl_entry *entry)
{
	if (entry == NULL)
		return;

	free(entry->name);
	pancl_value_fini(&(entry->data));

	pancl_entry_init(entry);
}


void
pancl_table_init(struct pancl_table *table)
{
	if (table == NULL)
		return;

	memset(table, 0, sizeof(*table));
	pancl_table_data_init(&(table->data));
}

void
pancl_table_fini(struct pancl_table *table)
{
	if (table == NULL)
		return;

	free(table->name);
	pancl_table_data_fini(&(table->data));

	pancl_table_init(table);
}

// vim:ts=4:sw=4:autoindent
