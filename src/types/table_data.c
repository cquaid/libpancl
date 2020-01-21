/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl/pancl.h"
#include "internal.h"

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
			pancl_entry_destroy(&(td->entries[i]));

		pancl_free(td->entries);
	}

	pancl_table_data_init(td);
}

int
pancl_table_data_append(struct pancl_table_data *td, struct pancl_entry *entry)
{
	int err;

	if (td == NULL || entry == NULL)
		return PANCL_ERROR_ARG_INVALID;

	if (!can_inc(td->count))
		return PANCL_ERROR_OVERFLOW;

	err = pancl_resize((void **)&(td->entries), sizeof(*(td->entries)),
			td->count + 1);

	if (err == PANCL_SUCCESS) {
		td->entries[td->count] = entry;
		td->count += 1;
	}

	return err;
}

// vim:ts=4:sw=4:autoindent
