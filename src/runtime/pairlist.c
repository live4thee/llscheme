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
#include "runtime/object.h"

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
  ret->u1.ptr = args[0];
  ret->u2.ptr = args[1];

  return ret;
}

BUILTIN("car", car);
struct ls_object *lsrt_builtin_car(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_check_arg_type(args, 0, 'p');

  return lso_pair_car(args[0]);
}

BUILTIN("cdr", cdr);
struct ls_object *lsrt_builtin_cdr(int argc, struct ls_object *args[],
                                   struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_check_arg_type(args, 0, 'p');

  return lso_pair_cdr(args[0]);
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

BUILTIN_LIB("length", length);
struct ls_object *lsrt_builtin_length(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  struct ls_object *obj, *it;
  int n;

  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  obj = lsrt_new_object(ls_t_number);

  for (n = 0, it = args[0];;) {
    if (lso_is_nil(it)) {
      lso_number(obj) = n;
      break;
    }
    else if (lso_is_pair(it)) {
      it = lso_pair_cdr(it);
      n++;
    }
    else
      lsrt_error("length requires a list");
  }

  return obj;
}

/* vim: set et ts=2 sw=2 cin: */
