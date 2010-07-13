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

#include <string.h>

#include <gmp.h>

#include "utils.h"

/*
 * We need:
 *  - a memory mangement system (or malloc is llvm builtin?)
 *  - error handling
 *  - utility functions for parameter checking
 *  - primitive functions like arithmetic
 */

/* TODO: more sophisticated GC memory allocator */
static void* ls_realloc(void* ptr, size_t old_size, size_t new_size)
{
  if(old_size == new_size)
    return ptr;

  return GC_realloc(ptr, new_size);
}

static void ls_free_mp(void *ptr, size_t size)
{
  UNUSED_ARGUMENT(ptr);
  UNUSED_ARGUMENT(size);
}

void lsrt_memory_init(void)
{
  GC_init();
  mp_set_memory_functions(ls_malloc, ls_realloc, ls_free_mp);
}

struct ls_object *lsrt_new_object(int type)
{
  struct ls_object *ret;

  ret = (struct ls_object *) ls_malloc(sizeof *ret);
  memset(ret, 0, sizeof *ret);
  lso_set_type(ret, type);

  return ret;
}

struct ls_object *lsrt_new_number(uint32_t num)
{
  struct ls_object *ret;

  ret = (struct ls_object *) ls_malloc(sizeof *ret);
  memset(ret, 0, sizeof *ret);
  lso_set_type(ret, ls_t_number);
  lso_number(ret) = num;

  return ret;
}

/*
 * XXX: can we just save pointers safely? Does the
 * content need to be saved as well?
 */
struct ls_object **lsrt_new_freelist(int count)
{
  struct ls_object **ret;

  ret = (struct ls_object **) ls_malloc(count * sizeof(*ret));
  memset(ret, 0, count * sizeof(*ret));
  return ret;
}

void lsrt_fill_freelist(struct ls_object* freelist[],
                        int i, struct ls_object* obj)
{
  freelist[i] = obj;
}

void lsrt_func_p(const struct ls_object *obj)
{
  if (!lso_is_func(obj) || lso_func_get(obj) == NULL)
      lsrt_error("expected function");
}

void lsrt_check_symbol_unbound(const struct ls_object *arg)
{
  if (!lso_is_symbol(arg))
    lsrt_error("internal error");

  if (lso_symbol_deref(arg) == NULL)
    lsrt_error("unbounded symbol: %s", lso_symbol_name(arg));
}

void lsrt_check_args_count(int min, int max, int argc)
{
  if ((min != 0 && min > argc) ||
      (max != 0 && max < argc)) {
    lsrt_error("fucntion count mismatch: min=%d, max=%d, argc=%d",
        min, max, argc);
  }
}

int lsrt_main_retval(const struct ls_object *lso)
{
  if (lso_is_simplenumber(lso))
    return lso_simplenumber_get(lso);

  if (lso_is_boolean(lso))
    return !!lso_boolean_get(lso) - 1;

  return 0;
}

int lsrt_test_expr(const struct ls_object *lso)
{
  if (lso_is_boolean(lso) && lso_boolean_get(lso) == 0)
    return 0;
  else
    return 1;
}

/* vim: set et ts=2 sw=2 cin: */
