/**
 * @file jems.c
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
 *
 */

// *****************************************************************************
// Includes

#include "jems.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// *****************************************************************************
// Private types and definitions

// *****************************************************************************
// Private (static) storage

// *****************************************************************************
// Private (static, forward) declarations

static jems_t *push_level(jems_t *jems, bool is_object);
static jems_t *pop_level(jems_t *jems);
static jems_t *emit_char(jems_t *jems, char ch);
static jems_t *emit_string(jems_t *jems, const char *s);
static jems_t *emit_quoted_string(jems_t *jems, const char *s);
static jems_t *commify(jems_t *jems);
static jems_level_t *level_ref(jems_t *jems);

// *****************************************************************************
// Public code

jems_t *jems_init(jems_t *jems,
                  jems_level_t *levels,
                  size_t max_level,
                  jems_writer_fn writer,
                  uintptr_t arg) {
    jems->levels = levels;
    jems->max_level = max_level;
    jems->writer = writer;
    jems->arg = arg;
    return jems_reset(jems);
}

jems_t *jems_reset(jems_t *jems) {
    jems->curr_level = 0;
    level_ref(jems)->item_count = 0;
    level_ref(jems)->is_object = false;
    return jems;
}

jems_t *jems_object_open(jems_t *jems) {
    commify(jems);
    emit_char(jems, '{');
    return push_level(jems, true);
}

jems_t *jems_object_close(jems_t *jems) {
    emit_char(jems, '}');
    return pop_level(jems);
}

jems_t *jems_array_open(jems_t *jems) {
    commify(jems);
    emit_char(jems, '[');
    return push_level(jems, false);
}

jems_t *jems_array_close(jems_t *jems) {
    emit_char(jems, ']');
    return pop_level(jems);
}

jems_t *jems_number(jems_t *jems, double value) {
    char buf[22];
    int64_t i = value;
    if ((double)i == value) {
        // if number can be represented exactly as an int, print as int
        snprintf(buf, sizeof(buf), "%" PRId64, i);
    } else {
        snprintf(buf, sizeof(buf), "%lf", value);
    }
    commify(jems);
    return emit_string(jems, buf);
}

jems_t *jems_integer(jems_t *jems, int64_t value) {
    char buf[22]; // 20 digits, 1 sign, 1 null
    snprintf(buf, sizeof(buf), "%" PRId64, value);
    commify(jems);
    return emit_string(jems, buf);
}

jems_t *jems_string(jems_t *jems, const char *string) {
    commify(jems);
    emit_char(jems, '"');
    emit_quoted_string(jems, string);
    return emit_char(jems, '"');
}

jems_t *jems_bool(jems_t *jems, bool boolean) {
    commify(jems);
    return emit_string(jems, boolean ? "true" : "false");
}

jems_t *jems_true(jems_t *jems) {
    commify(jems);
    return emit_string(jems, "true");
}

jems_t *jems_false(jems_t *jems) {
    commify(jems);
    return emit_string(jems, "false");
}

jems_t *jems_null(jems_t *jems) {
    commify(jems);
    return emit_string(jems, "null");
}

size_t jems_curr_level(jems_t *jems) {
    return jems->curr_level;
}

size_t jems_item_count(jems_t *jems) {
    return level_ref(jems)->item_count;
}

// *****************************************************************************
// Private (static) code

static jems_t *push_level(jems_t *jems, bool is_object) {
    if (jems->curr_level < jems->max_level - 1) {
        jems->curr_level += 1;
        level_ref(jems)->item_count = 0;
        level_ref(jems)->is_object = is_object;
    }
    return jems;
}

static jems_t *pop_level(jems_t *jems) {
    if (jems->curr_level > 0) {
        jems->curr_level -= 1;
    }
    return jems;
}

static jems_t *emit_char(jems_t *jems, char ch) {
    jems->writer(ch, jems->arg);
    return jems;
}

static jems_t *emit_string(jems_t *jems, const char *s) {
    while(*s) {
      emit_char(jems, *s++);
    }
    return jems;
}

static jems_t *emit_quoted_string(jems_t *jems, const char *s) {
  while(*s) {
    if (*s < 0x20) {
      char buf[7];
      sprintf(buf, "\\u%04x", *s++);
      emit_string(jems, buf);
    } else {
      if ((*s == '\\') || (*s == '"')) {
        emit_char(jems, '\\');
      }
      emit_char(jems, *s++);
    }
  }
  return jems;
}

static jems_t *commify(jems_t *jems) {
    jems_level_t *level = level_ref(jems);
    size_t count = level->item_count;
    if (level->is_object) {
      // within { ... }:
      // the 0th item has no prefix
      // odd items are prefixed with a ':'
      // even items are prefixed with a ','
      if (count > 0) {
        emit_char(jems, (count & 1) ? ':' : ',');
      }
    } else {
      // for other lists:
      // the 0th item has no prefix
      // all other items are prefixed with ','
      if (count > 0) {
        emit_char(jems, ',');
      }
    }
    level->item_count += 1;
    return jems;
}

static jems_level_t *level_ref(jems_t *jems) {
  return &jems->levels[jems->curr_level];
}

// *****************************************************************************
// End of file
