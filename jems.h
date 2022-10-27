/**
 * @file jems.h
 *
 * MIT License
 *
 * Copyright (c) 2022 R. Dunbar Poor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

 /**
  * @brief a pure-C JSON serializer for embedded systems
  */

#ifndef _JEMS_H_
#define _JEMS_H_

// *****************************************************************************
// Includes

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

typedef struct {
  size_t item_count;       // # of items emitted at this level
  bool is_object;     // if true, use ':' separator
} jems_level_t;

// Signature for the jems_emit function
typedef void (*jems_writer_fn)(char ch, uintptr_t arg);

typedef struct _jems {
  jems_level_t *levels;
  size_t max_level;
  size_t curr_level;
  jems_writer_fn writer;
  uintptr_t arg;
} jems_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Initialize the jems system.
 *
 * Example:
 *
 *     #include "jems.h"
 *     #include <stdio.h>
 *
 *     #define JEMS_MAX_LEVEL 10
 *     static jems_level_t jems_levels[JEMS_MAX_LEVEL];
 *     static jems_t jems_obj;
 *     jems_init(&jems_obj, jems_levels, JEMS_MAX_LEVEL, puts)
 *
 * @param jems A jems struct to hold state.
 * @param level An array of jems_level objects.
 * @param max_level The number of elements in @ref level.
 * @param writer A function that takes one char arg and renders it.
 * @param arg User-supplied argument passed to the writer function.
 */
jems_t *jems_init(jems_t *jems,
                  jems_level_t *levels,
                  size_t max_level,
                  jems_writer_fn writer,
                  uintptr_t arg);

/**
 * @brief Reset to top level.
 */
jems_t *jems_reset(jems_t *jems);

/**
 * @brief Start a JSON object, i.e. emit '{'
 */
jems_t *jems_object_open(jems_t *jems);

/**
 * @brief End a JSON object, i.e. emit '}'
 */
jems_t *jems_object_close(jems_t *jems);

/**
 * @brief Start a JSON array, i.e. emit '['
 */
jems_t *jems_array_open(jems_t *jems);

/**
 * @brief End a JSON array, i.e. emit ']'
 */
jems_t *jems_array_close(jems_t *jems);

/**
 * @brief Emit a number in JSON format.
 *
 * Note: if value can be exactly represented as an integer, this is equivalent
 * to jems_integer(jems, value);
 */
jems_t *jems_number(jems_t *jems, double value);

/**
 * @brief Emit an integer in JSON format.
 */
jems_t *jems_integer(jems_t *jems, int64_t value);

/**
 * @brief Emit a string in JSON format.
 */
jems_t *jems_string(jems_t *jems, const char *string);

/**
 * @brief Emit a string of known length in JSON format.
 *        Useful for strings which are not null-terminated.
 */
jems_t *jems_string_span(jems_t *jems, const char *string, size_t len);

/**
 * @brief Emit a boolean (true or false) in JSON format.
 */
jems_t *jems_bool(jems_t *jems, bool boolean);

/**
 * @brief Emit a true value in JSON format.
 */
jems_t *jems_true(jems_t *jems);

/**
 * @brief Emit a false value in JSON format.
 */
jems_t *jems_false(jems_t *jems);

/**
 * @brief Emit a null in JSON format.
 */
jems_t *jems_null(jems_t *jems);

/**
 * @brief Return the current expression depth.
 */
size_t jems_curr_level(jems_t *jems);

/**
 * @brief Return the number of items emitted at this level.
 */
size_t jems_item_count(jems_t *jems);

// *****************************************************************************
// End of file

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _JEMS_H_ */
