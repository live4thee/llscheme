/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
 *
 * Copyright (C)
 *         2010 David Lee <live4thee@gmail.com>
 *         2010 Qing He <qing.x.he@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/* This file is part of the runtime */

#include <string.h>
#include <strings.h>

#include "utils.h"

/**********************************************************
 * Section 6.3.3 Symbols
 **********************************************************/
BUILTIN("symbol?", symp);
struct ls_object *lsrt_builtin_symp(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, lso_is_symbol(args[0]));
  return obj;
}

BUILTIN("symbol->string", sym2str);
struct ls_object *lsrt_builtin_sym2str(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_symbol_p(args[0]);

  obj = lsrt_new_object(ls_t_string);
  // maybe we need to duplicate a copy?
  lso_string_set(obj, lso_symbol_name(args[0]));
  return obj;
}


/**********************************************************
 * Section 6.3.5 Strings
 **********************************************************/

BUILTIN("string?", strp);
struct ls_object *lsrt_builtin_strp(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, lso_is_string(args[0]));
  return obj;
}

BUILTIN("string-length", strlen);
struct ls_object *lsrt_builtin_strlen(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_string_p(args[0]);

  obj = lsrt_new_object(ls_t_number);
  lso_number_set(obj, strlen(lso_string_get(args[0])));
  return obj;
}

BUILTIN("string=?", streq);
struct ls_object *lsrt_builtin_streq(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(2, 2, argc);
  lsrt_string_p(args[0]);
  lsrt_string_p(args[1]);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, strcmp(lso_string_get(args[0]),
      lso_string_get(args[1])) == 0);
  return obj;
}

BUILTIN("string-ci=?", strcaseeq);
struct ls_object *lsrt_builtin_strcaseeq(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(2, 2, argc);
  lsrt_string_p(args[0]);
  lsrt_string_p(args[1]);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, strcasecmp(lso_string_get(args[0]),
      lso_string_get(args[1])) == 0);
  return obj;
}

BUILTIN("substring", substring);
struct ls_object *lsrt_builtin_substring(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  int start, end, len, buflen;
  char* str, *buf;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(3, 3, argc);
  lsrt_string_p(args[0]);
  lsrt_number_p(args[1]);
  lsrt_number_p(args[2]);

  str = lso_string_get(args[0]);
  len = strlen(str);
  start = lso_simplenumber_get(args[1]);
  end = lso_simplenumber_get(args[2]);

  if (start < 0 || start > end || end > len)
    lsrt_error("Out of range, start=%d, end=%d, len=%d", start, end, len);

  buflen = end - start + 1;
  buf = ls_malloc(buflen);
  strncpy(buf, str+start, buflen-1);
  buf[buflen] = '\0';

  obj = lsrt_new_object(ls_t_string);
  lso_string_set(obj, buf);
  return obj;
}

BUILTIN("string-append", strappend);
struct ls_object *lsrt_builtin_strappend(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  int idx, total_len = 0;
  char *buf;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 0, argc);

  for (idx = 0; idx < argc; ++idx) {
    lsrt_string_p(args[idx]);
    total_len += strlen(lso_string_get(args[idx]));
  }

  buf = ls_malloc(total_len + 1);
  bzero(buf, total_len+1);

  for (idx = 0; idx < argc; ++idx) {
    strcat(buf, lso_string_get(args[idx]));
  }

  obj = lsrt_new_object(ls_t_string);
  lso_string_set(obj, buf);
  return obj;
}

/* vim: set et ts=2 sw=2 cin: */
