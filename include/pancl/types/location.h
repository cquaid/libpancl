/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_LOCATION
#define H_PANCL_TYPES_LOCATION

/**
 * Storage for the line/column position of the start of any given data.
 */
struct pancl_location {
	unsigned long column; /**< Column number (0-based) of the current line */
	unsigned long line; /**< Line number (0-based) */
};

#endif /* H_PANCL_TYPES_LOCATION */
// vim:ts=4:sw=4:autoindent
