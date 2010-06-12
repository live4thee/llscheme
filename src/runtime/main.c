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
  ret->type = type;

  return ret;
}

void lsrt_error(const char *str)
{
  fprintf(stderr, "error: %s\n", str? str: "");
}

/*
 * may be slow, but we get simplicity at this point
 * this needs to be moved to code generator
 *
 * `a' for any
 * `o' for any but void
 * `n' for number or bignum
 * `p' for pair
 * `*' for vararg, the type is previous one
 *
 * e.g. the arg spec of operation `+' is "n*"
 */
void lsrt_check_args(const char *spec, int argc, struct ls_object *args)
{
  UNUSED_ARGUMENT(spec);
  UNUSED_ARGUMENT(argc);
  UNUSED_ARGUMENT(args);

  /* XXX: we can't get more info with this simple approach :-( */
  if (0) {
    lsrt_error("type error");
  }
  return;
}

static struct ls_object *
lsrt_builtin_arith(const char op, int argc, struct ls_object *args)
{
  struct ls_object *ret = lsrt_new_object(ls_t_number);
  int i;

  lsrt_check_args("n*", argc, args);

  ret->u1.val = args[0].u1.val;

  for (i = 1; i < argc; i++) {
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
  return lsrt_builtin_arith('+', argc, args);
}

struct ls_object *lsrt_builtin_minus(int argc, struct ls_object *args)
{
  return lsrt_builtin_arith('-', argc, args);
}

struct ls_object *lsrt_builtin_multiply(int argc, struct ls_object *args)
{
  return lsrt_builtin_arith('*', argc, args);
}

struct ls_object *lsrt_builtin_divide(int argc, struct ls_object *args)
{
  return lsrt_builtin_arith('/', argc, args);
}

