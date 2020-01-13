/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>

#include "pancl.h"
#include "pancl_error.h"


static void print_value(struct pancl_value *, int);
static void print_entry(struct pancl_entry *entry, int level);


static void
print_array(struct pancl_array *array, int level)
{
	size_t i;

	printf("%*.sItems: %zu\n", level, "", array->count);

	for (i = 0; i < array->count; ++i)
		print_value(array->items[i], level + 2);
}

static void
print_tuple(struct pancl_tuple *tuple, int level)
{
	size_t i;

	printf("%*.sItems: %zu\n", level, "", tuple->count);

	for (i = 0; i < tuple->count; ++i)
		print_value(tuple->items[i], level + 2);
}

static void
print_table_data(struct pancl_table_data *table, int level)
{
	size_t i;

	printf("%*.sEntries: %zu\n", level, "", table->count);

	for (i = 0; i < table->count; ++i)
		print_entry(table->entries[i], 2);
}

static void
print_value(struct pancl_value *value, int level)
{
	switch (value->type) {
	case PANCL_TYPE_BOOLEAN:
		printf("%*.sBOOL (%d)\n", level, "", value->data.boolean);
		break;

	case PANCL_TYPE_INTEGER:
		printf("%*.sINT (%d)\n", level, "", value->data.integer);
		break;

	case PANCL_TYPE_FLOATING:
		printf("%*.sFLOAT (%f)\n", level, "", value->data.floating);
		break;

	case PANCL_TYPE_STRING:
		printf("%*.sSTRING (\"%s\")\n", level, "", value->data.string);
		break;

	case PANCL_TYPE_CUSTOM:
		printf("%*.sCUSTOM (%s)\n", level, "", value->data.custom.name);
		print_tuple(&(value->data.custom.args), level + 2);
		break;

	case PANCL_TYPE_ARRAY:
		printf("%*.sARRAY:\n", level, "");
		print_array(&(value->data.array), level + 2);
		break;

	case PANCL_TYPE_TUPLE:
		printf("%*.sTUPLE:\n", level, "");
		print_tuple(&(value->data.tuple), level + 2);
		break;

	case PANCL_TYPE_TABLE:
		printf("%*.sTABLE:\n", level, "");
		print_table_data(&(value->data.table), level + 2);
		break;

	case PANCL_TYPE_OPT_INT8:
		printf("%*.sINT8 (%lld)\n", level, "",
			(long long)value->data.opt.int8);
		break;

	case PANCL_TYPE_OPT_UINT8:
		printf("%*.sUINT8 (%llu)\n", level, "",
			(unsigned long long)value->data.opt.uint8);
		break;

	case PANCL_TYPE_OPT_INT16:
		printf("%*.sINT16 (%lld)\n", level, "",
			(long long)value->data.opt.int16);
		break;

	case PANCL_TYPE_OPT_UINT16:
		printf("%*.sUINT16 (%llu)\n", level, "",
			(unsigned long long)value->data.opt.uint16);
		break;

	case PANCL_TYPE_OPT_INT32:
		printf("%*.sINT32 (%lld)\n", level, "",
			(long long)value->data.opt.int32);
		break;

	case PANCL_TYPE_OPT_UINT32:
		printf("%*.sUINT32 (%llu)\n", level, "",
			(unsigned long long)value->data.opt.uint32);
		break;

	case PANCL_TYPE_OPT_INT64:
		printf("%*.sINT64 (%lld)\n", level, "",
			(long long)value->data.opt.int64);
		break;

	case PANCL_TYPE_OPT_UINT64:
		printf("%*.sUINT64 (%llu)\n", level, "",
			(unsigned long long)value->data.opt.uint64);
		break;
	}
}

static void
print_entry(struct pancl_entry *entry, int level)
{
	printf("%*.sEntry: \"%s\"\n", level, "", entry->name);
	print_value(&(entry->data), level + 2);
}

static void
print_table(struct pancl_table *table)
{
	size_t i;

	if (table->name == NULL)
		printf("<< Global Table >>\n");
	else
		printf("Table: \"%s\"\n", table->name);

	printf("Entries: %zu\n", table->data.count);

	for (i = 0; i < table->data.count; ++i)
		print_entry(table->data.entries[i], 2);

	printf("\n");
}

int
main(int argc, char *argv[])
{
	int err;
	FILE *f;
	struct pancl_context ctx;
	struct pancl_table table;

	pancl_context_init(&ctx);
	pancl_table_init(&table);

	if (argc != 2)
		return -1;

	f = fopen(argv[1], "r");

	if (f == NULL)
		return -1;
	if (ferror(f))
		return -1;

	err = pancl_parse_file(&ctx, f);

	if (err != 0) {
		fprintf(stderr, "Failed to attach to file: %d\n", err);
		goto out;
	}

	for (;;) {
		err = pancl_get_table(&ctx, &table);

		if (err == PANCL_END_OF_INPUT) {
			printf("== END OF INPUT ==\n");
			err = 0;
			break;
		}

		if (err != PANCL_SUCCESS) {
			fprintf(stderr, "Parser error {%ld,%ld}: %d\n",
				ctx.pos.line, ctx.pos.column, err);
			goto out;
		}

		print_table(&table);

		pancl_table_fini(&table);
	}

out:
	pancl_table_fini(&table);
	pancl_context_fini(&ctx);
	fclose(f);
	return err;
}

