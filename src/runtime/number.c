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

#include <gmp.h>

/**********************************************************
 * Section 6.2. Numbers
 **********************************************************/

/**********************************************************
 * Section 6.2.5 Numerical operations
 **********************************************************/

static void
lso_negative(struct ls_object* dstobj, const struct ls_object* srcobj)
{
  if (lso_is_number(srcobj)) {
    lso_number(dstobj) = - (lso_number_get(srcobj));
  } else if (lso_is_bignum(srcobj)) {
    mpz_init(lso_bignum_get(dstobj));
    mpz_neg(lso_bignum_get(dstobj), lso_bignum_get(srcobj));
  } else {
    lsrt_error("unsupported type detected");
  }
}        

static void
lso_reciprocal(struct ls_object* dstobj, const struct ls_object* srcobj)
{
  if (lso_is_number(srcobj)) {
    lso_number(dstobj) = 1 / (lso_number_get(srcobj));
  } else if (lso_is_bignum(srcobj)) {
    mpz_t one;
    mpz_init_set_str(one, "1", 10);
    mpz_init(lso_bignum_get(dstobj));
    mpz_tdiv_q(lso_bignum_get(dstobj), one, lso_bignum_get(srcobj));
    mpz_clear(one);
  } else {
    lsrt_error("unsupported type detected");
  }
}

static struct ls_object *
_arith(const char op, int argc, struct ls_object *args[])
{
  struct ls_object *ret = NULL;
  int i = 0;

  /* '/' and '-' requires at least one operand */
  if ((op == '/' || op == '-') && (argc == 0))
    lsrt_error("invalid number of operand");

  switch (op) {
    case '+': ret = lsrt_new_number(0); break;
    case '*': ret = lsrt_new_number(1); break;

    case '-': if (argc == 1) {
                return lsrt_new_number(- lso_number_get(args[0]));
              }
              ret = lsrt_new_number(lso_number_get(args[0]));
              i = 1;
              break;

    case '/': if (argc == 1) {
                return lsrt_new_number(1 / lso_number_get(args[0]));
              }
              ret = lsrt_new_number(lso_number_get(args[0]));
              i = 1;
              break;
    default:
      lsrt_error("invalid operator");
  }

  for (; i < argc; i++) {
    lsrt_check_arg_type(args, i, 'n');
    switch (op) {
    case '+': lso_number(ret) += lso_number_get(args[i]); break;
    case '-': lso_number(ret) -= lso_number_get(args[i]); break;
    case '*': lso_number(ret) *= lso_number_get(args[i]); break;
      /* this is however wrong, we need rationals */
    case '/': lso_number(ret) /= lso_number_get(args[i]); break;
    }
  }

  return ret;
}

BUILTIN("+", plus);
struct ls_object *lsrt_builtin_plus(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(0, 0, argc);
  return _arith('+', argc, args);
}

BUILTIN("-", minus);
struct ls_object *lsrt_builtin_minus(int argc, struct ls_object *args[],
                                     struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 0, argc);
  return _arith('-', argc, args);
}

BUILTIN("*", multiply);
struct ls_object *lsrt_builtin_multiply(int argc, struct ls_object *args[],
                                        struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(0, 0, argc);
  return _arith('*', argc, args);
}

BUILTIN("/", devide);
struct ls_object *lsrt_builtin_divide(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 0, argc);
  return _arith('/', argc, args);
}

static struct ls_object *
_order(char op, int argc, struct ls_object *args[])
{
  struct ls_object *ret = lsrt_new_object(ls_t_boolean);
  int i;
  uint32_t n;

  lso_boolean(ret) = 1;

  if (argc == 0)
    return ret;
  else {
    lsrt_check_arg_type(args, 0, 'n');
    n = lso_number_get(args[0]);
    for (i = 1; i < argc; i++) {
      switch (op) {
      case '=':
        if (lso_number_get(args[i]) != n)
          goto fail;
        break;
      case '<':
        if (lso_number_get(args[i]) >= n)
          goto fail;
        break;
      case '>':
        if (lso_number_get(args[i]) <= n)
          goto fail;
        break;
      case 'l':
        if (lso_number_get(args[i]) > n)
          goto fail;
        break;
      case 'g':
        if (lso_number_get(args[i]) < n)
          goto fail;
        break;
      default:
        break;
      }
    }
  }
  return ret;
 fail:
  lso_boolean(ret) = 0;
  return ret;
}

BUILTIN("=", eq);
struct ls_object *lsrt_builtin_eq(int argc, struct ls_object *args[],
                                  struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);

  return _order('=', argc, args);
}

BUILTIN("<", lt);
struct ls_object *lsrt_builtin_lt(int argc, struct ls_object *args[],
                                  struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);

  return _order('<', argc, args);
}

BUILTIN(">", gt);
struct ls_object *lsrt_builtin_gt(int argc, struct ls_object *args[],
                                  struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);

  return _order('>', argc, args);
}

BUILTIN("<=", le);
struct ls_object *lsrt_builtin_le(int argc, struct ls_object *args[],
                                  struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);

  return _order('l', argc, args);
}

BUILTIN(">=", ge);
struct ls_object *lsrt_builtin_ge(int argc, struct ls_object *args[],
                                  struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);

  return _order('g', argc, args);
}

/**********************************************************
 * Section 6.2.6 Numerical Input and Output
 **********************************************************/

/* create a bignum ls_object with given a number string */
BUILTIN("string->number", string2number);
struct ls_object *lsrt_builtin_string2number(int argc, struct ls_object *args[],
                                             struct ls_object *freelist[])
{
  struct ls_object *ret;
  mpz_t *n;

  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 2, argc);
  lsrt_check_arg_type(args, 0, 's');

  ret = lsrt_new_object(ls_t_bignum);

  // TODO: inexact, rational and complex numbers
  // init a base10 big number
  n = (mpz_t *) ls_malloc(sizeof *n);
  ret->u1.ptr = n;
  mpz_init_set_str(*n, lso_string_get(args[0]), 10);

  return ret;
}

/* vim: set et ts=2 sw=2 cin: */
