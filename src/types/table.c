/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"
#include "internal.h"

void
pancl_table_init(struct pancl_table *table)
{
	if (table != NULL) {
		memset(table, 0, sizeof(*table));
		pancl_table_data_init(&(table->data));
	}
}

void
pancl_table_fini(struct pancl_table *table)
{
	if (table == NULL)
		return;

	pancl_free(table->name);
	pancl_table_data_fini(&(table->data));

	pancl_table_init(table);
}

// vim:ts=4:sw=4:autoindent
