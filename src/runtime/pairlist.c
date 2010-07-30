/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
 *
 * Copyright (C)
 *         2010 Qing He <qing.x.he@gmail.com>
 *         2010 David Lee <live4thee@gmail.com>
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

#include "utils.h"

/**********************************************************
 * Section 6.3.2 Pairs and Lists
 **********************************************************/
BUILTIN("cons", cons);
struct ls_object *lsrt_builtin_cons(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object *ret;

  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(2, 2, argc);

  ret = lsrt_new_object(ls_t_pair);
  lso_set_car(ret, args[0]);
  lso_set_cdr(ret, args[1]);

  return ret;
}

BUILTIN("car", car);
struct ls_object *lsrt_builtin_car(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_pair_p(args[0]);

  return lso_pair_car(args[0]);
}

BUILTIN("set-car!", setcar);
struct ls_object *lsrt_builtin_setcar(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(2, 2, argc);
  lsrt_pair_p(args[0]);

  lso_set_car(args[0], args[1]);
  return lsrt_new_object(ls_t_unspec);
}

BUILTIN("cdr", cdr);
struct ls_object *lsrt_builtin_cdr(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_pair_p(args[0]);

  return lso_pair_cdr(args[0]);
}

BUILTIN("set-cdr!", setcdr);
struct ls_object *lsrt_builtin_setcdr(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(2, 2, argc);
  lsrt_pair_p(args[0]);

  lso_set_cdr(args[0], args[1]);
  return lsrt_new_object(ls_t_unspec);
}

BUILTIN("null?", nullp);
struct ls_object *lsrt_builtin_nullp(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, lso_is_nil(args[0]));
  return obj;
}

BUILTIN("pair?", pairp);
struct ls_object *lsrt_builtin_pairp(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, lso_is_pair(args[0]));
  return obj;
}

BUILTIN_LIB("list", list);
struct ls_object *lsrt_builtin_list(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  struct ls_object *last, *pair = NULL;
  int i;
  UNUSED_ARGUMENT(freelist);

  last = lsrt_new_object(ls_t_nil);

  for (i = argc - 1; i >= 0; i--) {
    pair = lsrt_new_object(ls_t_pair);
    pair->u1.ptr = args[i];
    pair->u2.ptr = last;
    last = pair;
  }

  return pair;
}

/* Return the length of a list and -1 if not a list.
 * potential bug if list length can't be held in an `int'.
 */
static int _length(const struct ls_object* obj)
{
  int len = 0;
  const struct ls_object* it = obj;

  while (!lso_is_nil(it)) {
    if (lso_is_pair(it)) {
      it = lso_pair_cdr(it);
      len++;
    } else return -1;
  }

  return len;
}

static int _listp(const struct ls_object* obj)
{
  const struct ls_object* it = obj;

  while (!lso_is_nil(it)) {
    if (lso_is_pair(it)) {
      it = lso_pair_cdr(it);
    } else return 0;
  }

  return 1;
}

BUILTIN("list?", listp);
struct ls_object *lsrt_builtin_listp(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, _listp(args[0]));
  return obj;
}

BUILTIN_LIB("length", length);
struct ls_object *lsrt_builtin_length(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  struct ls_object *obj;
  int n;

  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  n = _length(args[0]);
  if (n == -1)
    lsrt_error("%s requires a list", __func__);

  obj = lsrt_new_object(ls_t_number);
  lso_number_set(obj, n);
  return obj;
}

BUILTIN_LIB("list-ref", listref);
struct ls_object *lsrt_builtin_listref(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  struct ls_object *it;
  int len, idx;

  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(2, 2, argc);
  lsrt_number_p(args[1]);

  len = _length(args[0]);
  if (len == -1)
    lsrt_error("%s requires a list", __func__);

  idx = lso_simplenumber_get(args[1]);
  if (idx < 0 || idx >= len)
    lsrt_error("(Argument 2) out of range: %d", idx);

  for (it = args[0]; idx > 0; --idx) {
    it = lso_pair_cdr(it);
  }

  return lso_pair_car(it);
}

/* vim: set et ts=2 sw=2 cin: */
