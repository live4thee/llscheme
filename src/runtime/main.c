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

#define _LLSCHEME_RUNTIME

#include "runtime/object.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED_ARGUMENT(x) (void)(x)

#ifdef BDWGC
/* The Boehm-Demers-Weiser conservative garbage collector */
#  include <gc.h>
#  define ls_malloc GC_malloc_atomic
#else
#  define ls_malloc malloc
#endif
/*
 * We need:
 *  - a memory mangement system (or malloc is llvm builtin?)
 *  - error handling
 *  - utility functions for parameter checking
 *  - primitive functions like arithmetic
 */

/* TODO: more sophisticated GC memory allocator */
struct ls_object *lsrt_new_object(int type)
{
  struct ls_object *ret;

  ret = (struct ls_object *) ls_malloc(sizeof *ret);
  memset(ret, 0, sizeof *ret);
  lso_set_type(ret, type);

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

/* TODO: printf like parameter */
void lsrt_error(const char *str)
{
  fprintf(stderr, "error: %s\n", str? str: "");
  exit(1);
}

/* optional argument spec later */
void lsrt_check_args_count(int min, int max, int argc)
{
  if ((min != 0 && min > argc) ||
      (max != 0 && max < argc)) {
    lsrt_error("fucntion count mismatch");
  }

  return;
}

/*
 * may be slow, but we get simplicity at this point
 * this needs to be moved to code generator
 *
 * `a' for any
 * `o' for any but void
 * `n' for number or bignum
 * `p' for pair
 * `f' for function
 */
void lsrt_check_arg_type(struct ls_object *args[], int i, char c)
{
  switch (c) {
  case 'a': break;
  case 'o':
    if (!lso_is_void(args[i]))
      lsrt_error("fucntion argument type mismatch");
    break;
  case 'n':
    if (!lso_is_number(args[i]) &&
        !lso_is_bignum(args[i]))
      lsrt_error("expected number");
    break;
  case 'f':
    if (!lso_is_func(args[i]) || lso_func_get(args[i]) == NULL)
      lsrt_error("expected function");
  default:
    break;
  }

  return;
}

void lsrt_check_symbol_unbound(struct ls_object *arg)
{
  if (!lso_is_symbol(arg))
    lsrt_error("internal error");

  if (lso_symbol_deref(arg) == NULL)
    lsrt_error("unbounded variable");
}

int lsrt_main_retval(struct ls_object *lso)
{
  if (lso_is_number(lso))
    return lso_number_get(lso);

  if (lso_is_boolean(lso))
    return !!lso_boolean_get(lso) - 1;

  return 0;
}


static struct ls_object *
lsrt_builtin_arith(const char op, int argc, struct ls_object *args[])
{
  struct ls_object *ret = lsrt_new_object(ls_t_number);
  int i;

  lso_number(ret) = lso_number_get(args[0]);

  for (i = 1; i < argc; i++) {
    lsrt_check_arg_type(args, i, 'n');
    switch (op) {
    case '+': lso_number(ret) += lso_number_get(args[i]); break;
    case '-': lso_number(ret) -= lso_number_get(args[i]); break;
    case '*': lso_number(ret) *= lso_number_get(args[i]); break;
      /* this is however wrong, we need rationals */
    case '/': lso_number(ret) /= lso_number_get(args[i]); break;
    default:
      break;
    }
  }

  return ret;
}

struct ls_object *lsrt_builtin_plus(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(0, 0, argc);
  return lsrt_builtin_arith('+', argc, args);
}

struct ls_object *lsrt_builtin_minus(int argc, struct ls_object *args[],
                                     struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 0, argc);
  return lsrt_builtin_arith('-', argc, args);
}

struct ls_object *lsrt_builtin_multiply(int argc, struct ls_object *args[],
                                        struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(0, 0, argc);
  return lsrt_builtin_arith('*', argc, args);
}

struct ls_object *lsrt_builtin_divide(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 0, argc);
  return lsrt_builtin_arith('/', argc, args);
}

static void _display(struct ls_object *lso, int fp)
{
  switch (lso->type) {
  case ls_t_void:
    printf("<void>");
    break;
  case ls_t_number:
    printf("%d", lso_number_get(lso));
    break;
  case ls_t_boolean:
    if (lso_boolean_get(lso))
      printf("#t");
    else
      printf("#f");
    break;
  case ls_t_bignum:
    printf("<bignum>");
    break;
  case ls_t_symbol:
    if (lso_symbol_name(lso))
      printf("%s", lso_symbol_name(lso));
    else
      printf("<anon symbol...>");
    break;
  case ls_t_string:
    printf("\"%s\"", lso_string_get(lso));
    break;
  case ls_t_pair:
    if (!fp)
      printf("(");

    _display(lso_pair_car(lso), 0);
    if (!lso_is_void(lso_pair_cdr(lso))) {
      if (lso_is_pair(lso_pair_cdr(lso)))
        printf(" ");
      else
        printf(" . ");
      _display(lso_pair_cdr(lso), 1);
    }

    if (!fp)
      printf(")");
    break;
  case ls_t_func:
    printf("<procedure %p>", lso_func_get(lso));
    break;
  default:
    break;
  }
}

struct ls_object *lsrt_builtin_display(int argc, struct ls_object *args[],
                                       struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  _display(args[0], 0);
  printf("\n");

  return lsrt_new_object(ls_t_void);
}

/* vim: set et ts=2 sw=2 cin: */
