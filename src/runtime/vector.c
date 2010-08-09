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

#include "utils.h"

/**********************************************************
 * Section 6.3.6 Vectors
 **********************************************************/

BUILTIN_LIB("vector?", vectorp);
struct ls_object *lsrt_builtin_vectorp(int argc, struct ls_object *args[],
                                       struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  obj = lsrt_new_object(ls_t_boolean);
  lso_boolean_set(obj, lso_is_vector(args[0]));
  return obj;
}

BUILTIN_LIB("vector-length", vlength);
struct ls_object *lsrt_builtin_vlength(int argc, struct ls_object *args[],
                                       struct ls_object *freelist[])
{
  struct ls_object* obj = NULL;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_vector_p(args[0]);

  obj = lsrt_new_object(ls_t_number);
  lso_number_set(obj, lso_vector_length(args[0]));
  return obj;
}

static struct ls_object* vector_new(int32_t len)
{
  struct ls_object* obj = lsrt_new_object(ls_t_vector);

  if (len > 0) {
    struct ls_object** arr = ls_malloc(len * (sizeof(struct ls_object*)));
    /* member is not initialized */
    lso_set_vec_addr0(obj, arr);
  }

  lso_vector_length(obj) = len;
  return obj;
}

BUILTIN_LIB("make-vector", mkvector);
struct ls_object *lsrt_builtin_mkvector(int argc, struct ls_object *args[],
                                        struct ls_object *freelist[])
{
  struct ls_object* obj, **addr0;
  int idx, len;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 2, argc);
  lsrt_number_p(args[0]);

  len = lso_simplenumber_get(args[0]);
  if (len < 0)
    lsrt_error("(Argument 1) out of range: %d", len);

  obj = vector_new(len);
  addr0 = lso_vector_addr0(obj);

  if (argc == 1) {
    for (idx = 0; idx < len; ++idx) {
      addr0[idx] = &global_unspec_obj;
    }
  } else {
    for (idx = 0; idx < len; ++idx) {
      addr0[idx] = args[1];   // TODO: deep-copy of args[1]
    }
  }

  return obj;
}

BUILTIN_LIB("vector", vector);
struct ls_object *lsrt_builtin_vector(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  struct ls_object* obj, **addr0;
  int idx;
  UNUSED_ARGUMENT(freelist);

  obj = vector_new(argc);
  addr0 = lso_vector_addr0(obj);

  for (idx = 0; idx < argc; ++idx) {
    addr0[idx] = args[idx];   // TODO: deep-copy of args[idx]
  }

  return obj;
}

BUILTIN_LIB("vector-ref", vecref);
struct ls_object *lsrt_builtin_vecref(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  int idx, len;
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(2, 2, argc);
  lsrt_vector_p(args[0]);
  lsrt_number_p(args[1]);

  len = lso_vector_length(args[0]);
  idx = lso_simplenumber_get(args[1]);

  if (idx < 0 || idx >= len)
    lsrt_error("(Argument 2) out of range: %d", idx);

  return lso_vector_addr0(args[0])[idx];
}

struct ls_object *lsrt_builtin_list(int, struct ls_object**,
                                    struct ls_object**);
BUILTIN_LIB("vector->list", vec2lst);
struct ls_object *lsrt_builtin_vec2lst(int argc, struct ls_object *args[],
                                       struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_vector_p(args[0]);

  return lsrt_builtin_list(lso_vector_length(args[0]),
                           lso_vector_addr0(args[0]), NULL);
}

/* vim: set et ts=2 sw=2 cin: */
