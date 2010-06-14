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

#include "runtime/object.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED_ARGUMENT(x) (void)x
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

  ret = (struct ls_object *) malloc(sizeof *ret);
  memset(ret, 0, sizeof *ret);
  ret->type = type;

  return ret;
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
 */
void lsrt_check_arg_type(struct ls_object *args, int i, char c)
{
  switch (c) {
  case 'a': break;
  case 'o':
    if (args[i].type== ls_t_void)
      goto err;
    break;
  case 'n':
    if (args[i].type != ls_t_number &&
        args[i].type != ls_t_bignum)
      goto err;
    break;
  default:
    break;
  }

  return;

 err:
  lsrt_error("fucntion argument type mismatch");
}


int lsrt_main_retval(struct ls_object *lso)
{
  if (lso->type == ls_t_number)
    return lso->u1.val;

  if (lso->type == ls_t_boolean)
    return !!lso->u1.val - 1;

  return 0;
}


static struct ls_object *
lsrt_builtin_arith(const char op, int argc, struct ls_object *args)
{
  struct ls_object *ret = lsrt_new_object(ls_t_number);
  int i;

  ret->u1.val = args[0].u1.val;

  for (i = 1; i < argc; i++) {
    lsrt_check_arg_type(args, i, 'n');
    switch (op) {
    case '+': ret->u1.val += args[i].u1.val; break;
    case '-': ret->u1.val -= args[i].u1.val; break;
    case '*': ret->u1.val *= args[i].u1.val; break;
      /* this is however wrong, we need rationals */
    case '/': ret->u1.val /= args[i].u1.val; break;
    default:
      break;
    }
  }

  return ret;
}

struct ls_object *lsrt_builtin_plus(int argc, struct ls_object *args)
{
  lsrt_check_args_count(0, 0, argc);
  return lsrt_builtin_arith('+', argc, args);
}

struct ls_object *lsrt_builtin_minus(int argc, struct ls_object *args)
{
   lsrt_check_args_count(1, 0, argc);
   return lsrt_builtin_arith('-', argc, args);
}

struct ls_object *lsrt_builtin_multiply(int argc, struct ls_object *args)
{
  lsrt_check_args_count(0, 0, argc);
  return lsrt_builtin_arith('*', argc, args);
}

struct ls_object *lsrt_builtin_divide(int argc, struct ls_object *args)
{
  lsrt_check_args_count(1, 0, argc);
  return lsrt_builtin_arith('/', argc, args);
}

