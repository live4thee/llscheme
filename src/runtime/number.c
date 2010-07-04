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
#include <string.h>
#include <ctype.h>
#include <malloc.h>

/**********************************************************
 * Section 6.2. Numbers
 **********************************************************/

/**********************************************************
 * Section 6.2.5 Numerical operations
 **********************************************************/
/*
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
*/
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
                return lsrt_new_number(- lso_simplenumber_get(args[0]));
              }
              ret = lsrt_new_number(lso_simplenumber_get(args[0]));
              i = 1;
              break;

    case '/': if (argc == 1) {
                return lsrt_new_number(1 / lso_simplenumber_get(args[0]));
              }
              ret = lsrt_new_number(lso_simplenumber_get(args[0]));
              i = 1;
              break;
    default:
      lsrt_error("invalid operator");
  }

  for (; i < argc; i++) {
    lsrt_check_arg_type(args, i, 'n');
    switch (op) {
    case '+': lso_number(ret) += lso_simplenumber_get(args[i]); break;
    case '-': lso_number(ret) -= lso_simplenumber_get(args[i]); break;
    case '*': lso_number(ret) *= lso_simplenumber_get(args[i]); break;
      /* this is however wrong, we need rationals */
    case '/': lso_number(ret) /= lso_simplenumber_get(args[i]); break;
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
  int32_t n;

  lso_boolean(ret) = 1;

  if (argc == 0)
    return ret;
  else {
    lsrt_check_arg_type(args, 0, 'n');
    n = lso_simplenumber_get(args[0]);
    for (i = 1; i < argc; i++) {
      switch (op) {
      case '=':
        if (lso_simplenumber_get(args[i]) != n)
          goto fail;
        break;
      case '<':
        if (lso_simplenumber_get(args[i]) >= n)
          goto fail;
        break;
      case '>':
        if (lso_simplenumber_get(args[i]) <= n)
          goto fail;
        break;
      case 'l':
        if (lso_simplenumber_get(args[i]) > n)
          goto fail;
        break;
      case 'g':
        if (lso_simplenumber_get(args[i]) < n)
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

#define stonf_valid     1 << 12
#define stonf_iunit     1 << 11
#define stonf_signed    1 << 10
#define stonf_decimal   1 << 9
#define stonf_pound     1 << 8
#define stonf_prec_mask 0xff

static int _ston_isdigit(char ch, int radix)
{

  if (ch == 0)
    return 0;

  switch (radix) {
  case 2:
    return !!strchr("01.", ch);
  case 8:
    return !!strchr("01234567.", ch);
  case 10:
    return !!strchr("0123456789.", ch);
  case 16:
    return !!strchr("0123456789abcdefABCDEF.", ch);
  }

  return 0;
}

static int _ston_parse_real(char *ptr, char **out, int radix)
{
  char *digits, *exp;
  int sign = 0, decimal = 0, pound = 0, iunit = 0;
  char precision = 'e';
  int flag = 0;

  switch (*ptr) {
  case '+':
  case '-':
    sign = 1;
    ptr++;
    break;
  }

  digits = ptr;

  while(_ston_isdigit(*ptr, radix)) {
    if (*ptr == '.') {
      if (decimal || radix != 10)
        goto err;
      decimal = 1;
    }
    ptr++;
  }
  if ((ptr - digits == 1) && *(ptr - 1) == '.')
    goto err;
  if (ptr == digits) {
    if (*ptr == 'i')
      iunit = 1;
    else
      goto err;
  }

  while (*ptr == '.' || *ptr == '#' ) {
    if (*ptr == '#') {
      pound = 1;
      *ptr = '0';
    }
    if (*ptr == '.') {
      if (decimal || radix != 10)
        goto err;
      decimal = 1;
    }
    ptr++;
  }

  switch (*ptr) {
  case 'e':
  case 's':
  case 'f':
  case 'd':
  case 'l':
    precision = *ptr;
    if (radix != 10)
      goto err;
    decimal = 1;
    *ptr++ = '@';
    exp = ptr;
    if (*ptr == '+' || *ptr == '-')
      ptr++;
    while (isdigit(*ptr))
      ptr++;
    if (ptr == exp)
      goto err;
  }

  flag = stonf_valid;
  flag |= sign? stonf_signed: 0;
  flag |= iunit? stonf_iunit: 0;
  flag |= decimal? stonf_decimal: 0;
  flag |= pound? stonf_pound: 0;
  flag |= precision & stonf_prec_mask;

  *out = ptr;
  return flag;

 err:
  flag = 0;
  return flag;

}

void _ston(struct ls_object *obj, const char *num, int defradix)
{
  char *dup = strdup(num), *ptr = dup;
  char *end;
  char saved;
  int exactness = -1, radix = -1;
  int flag1, flag2, allowre = 1, allowim = 1, type;
  mpz_t *z;
  mpq_t *q;
  mpf_t *f;
  int v;
  void *last = NULL;

  /* prefix */
  while (*ptr == '#') {
    ptr++;
    switch (*ptr) {
    case 'i':
    case 'e':
      if (exactness == -1)
        exactness = ((*ptr == 'i')? 0: 1);
      else
        goto err;

      ptr++;
      break;
    case 'b':
    case 'o':
    case 'd':
    case 'x':
      if (radix == -1)
        radix = ((*ptr == 'b')? 2:
                 (*ptr == 'o')? 8:
                 (*ptr == 'd')? 10: 16);
      else
        goto err;

      ptr++;
      break;
    default:
      goto err;
    }
  }

  if (radix == -1)
    radix = defradix;

 restart:
  /* default to big integer */
  type = 1;
  z = NULL;
  q = NULL;
  f = NULL;
  v = 0;

  /* parsing */
  flag1 = _ston_parse_real(ptr, &end, radix);
  if (!(flag1 & stonf_valid))
    goto err;
  if (*end == '/') {
    type = 2;
    flag2 = _ston_parse_real(end + 1, &end, radix);
    /* sanity for rational */
    if (!(flag2 & stonf_valid))
      goto err;
    if ((flag1 & stonf_decimal) ||
        (flag2 & stonf_signed) ||
        (flag2 & stonf_decimal))
      goto err;
  }
  if (flag1 & stonf_decimal)
    type = 3;

  if ((*end == 'i' && allowim == 0) ||
      (*end != 'i' && allowre == 0))
    goto err;

  /* generating initial result */
  if ((*end == 'i' && (flag1 & stonf_iunit))) {
    type = 0;
    if (!(flag1 & stonf_signed))
      goto err;
    if (*ptr == '+')
      v = 1;
    else
      v = -1;
  }

  saved = *end;
  *end = '\0';

  if (*ptr == '+')  /* tackle around strange behavior of mpx_set_str */
      ptr++;
  switch (type) {
  case 1:
    last = z = (mpz_t *) ls_malloc(sizeof *z);
    mpz_init(*z);
    mpz_set_str(*z, ptr, radix);
    break;
  case 2:
    last = q = (mpq_t *) ls_malloc(sizeof *q);
    mpq_init(*q);
    mpq_set_str(*q, ptr, radix);
    break;
  case 3:
    last = f = (mpf_t *) ls_malloc(sizeof *f);
    mpf_init(*f);
    mpf_set_str(*f, ptr, radix);
    break;
  }

  *end = saved;

  /* canonicalize and transform */
  if (type == 1 && ((flag1 & stonf_pound) || exactness == 0)) {
    last = f = (mpf_t *) ls_malloc(sizeof *f);
    mpf_init(*f);
    mpf_set_z(*f, *z);
    mpz_clear(*z);
    ls_free(z);
    type = 3;
  }
  if (type == 2 && ((flag1 & stonf_pound) || exactness == 0)) {
    last = f = (mpf_t *) ls_malloc(sizeof *f);
    mpf_init(*f);
    mpf_set_q(*f, *q);
    mpq_clear(*q);
    ls_free(q);
    type = 3;
  }
  if (type == 3 && exactness == 1) {
    last = q = (mpq_t *) ls_malloc(sizeof *q);
    mpq_init(*q);
    // TODO: default mpq_set_f is insane!! see #e0.1
    mpq_set_f(*q, *f);
    mpf_clear(*f);
    ls_free(f);
    type = 2;
  }
  if (type == 2 && exactness != 0) {
    mpq_canonicalize(*q);
    if (mpz_cmp_si(mpq_denref(*q), 1) == 0) {
      last = z = (mpz_t *) ls_malloc(sizeof *z);
      mpz_init(*z);
      mpz_set_q(*z, *q);
      mpq_clear(*q);
      ls_free(q);
      type = 1;
    }
  }
  if (type == 1 && exactness != 0) {
    if (mpz_fits_sint_p(*z)) {
      v = mpz_get_si(*z);
      mpz_clear(*z);
      ls_free(z);
      type = 0;
    }
  }

  /* store */
  if (*end == 'i') {
    if (!(flag1 & stonf_signed))  /* imaginary must be signed */
      goto err;
    allowim = 0;
    obj->type |= (type << 12) | ls_num_complex;
    switch (type) {
    case 0:
      if (v == 0) {
        obj->type &= ~(ls_num_complex | ls_num_im_mask);
        obj->u2.val = 0;
      }
      else
        obj->u2.val = v;
      break;
    case 1:
      obj->u2.ptr = z;
      break;
    case 2:
      obj->u2.ptr = q;
      break;
    case 3:
      if (mpf_cmp_si(*f, 0) == 0) {
        obj->type &= ~(ls_num_complex | ls_num_im_mask);
        obj->u2.val = 0;
        mpf_clear(*f);
        ls_free(f);
      }
      else
        obj->u2.ptr = f;
      break;
    }
    end++;
  } else {
    allowre = 0;
    switch (type) {
    case 0:
      obj->u1.val = v;
      break;
    case 1:
      obj->u1.ptr = z;
      break;
    case 2:
      obj->u1.ptr = q;
      break;
    case 3:
      obj->u1.ptr = f;
      break;
    }
    obj->type |= type << 8;
  }

  if (*end) {
    ptr = end;
    goto restart;
  }

  free(dup);
  return;

 err:
  free(dup);
  if (last)
    ls_free(last);

  lso_set_type(obj, ls_t_boolean);
  obj->u1.val = 0;
}

/* create a bignum ls_object with given a number string */
BUILTIN("string->number", string2number);
struct ls_object *lsrt_builtin_string2number(int argc, struct ls_object *args[],
                                             struct ls_object *freelist[])
{
  struct ls_object *ret;
  int radix = 10;

  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 2, argc);
  lsrt_check_arg_type(args, 0, 's');

  ret = lsrt_new_object(ls_t_number);
  if (argc == 2 && lso_is_simplenumber(args[1]))
    radix = lso_simplenumber_get(args[1]);

  _ston(ret, lso_string_get(args[0]), radix);

  return ret;
}

char *_ntos(struct ls_object *number, int radix)
{
  int complex, type1, type2;
  char fmt[20];
  char *str;
  int len, n;
  void *arg1 = NULL, *arg2 = NULL;

  fmt[0] = '\0';

  /* TODO: radix other than 10 */
  UNUSED_ARGUMENT(radix);

  complex = lso_is_complex(number);
  type1 = lso_number_type_re(number);
  type2 = lso_number_type_im(number);

  switch(type1) {
  case 0:
    if (complex && number->u1.val == 0) {
      arg1 = &n;     /* simply placeholder */
      strcpy(fmt, "%n");
    } else {
      arg1 = (void *)((long) number->u1.val);
      strcpy(fmt, "%d");
    }
    break;
  case 1:
    arg1 = *((mpz_t *)number->u1.ptr);
    strcpy(fmt, "%Zd");
    break;
  case 2:
    arg1 = *((mpq_t *)number->u1.ptr);
   strcpy(fmt, "%Qd");
    break;
  case 3:
    arg1 = *((mpf_t *)number->u1.ptr);
    /* we have to use different styles for integers and non-integers */
    if (mpf_integer_p(*((mpf_t *)number->u1.ptr)))
      strcpy(fmt, "%#.1Ff");
    else
      strcpy(fmt, "%.16Fg");
    break;
  }

  if (complex) {
    switch(type2) {
    case 0:
      arg2 = (void *)((long) number->u2.val);
      if (number->u2.val == 1) {
        arg2 = &n;
        strcat(fmt, "+%ni");
      } else if (number->u2.val == -1) {
        arg2 = &n;
        strcat(fmt, "-%ni");
      } else
        strcat(fmt, "%+di");
      break;
    case 1:
      arg2 = *((mpz_t *)number->u2.ptr);
      strcat(fmt, "%+Zdi");
      break;
    case 2:
      arg2 = *((mpq_t *)number->u2.ptr);
      strcat(fmt, "%+Qdi");
      break;
    case 3:
      arg2 = *((mpf_t *)number->u2.ptr);
    if (mpf_integer_p(*((mpf_t *)number->u2.ptr)))
      strcat(fmt, "%#+.1Ffi");
    else
      strcat(fmt, "%+.16Fgi");
      break;
    }
  }

  len = gmp_snprintf(NULL, 0, fmt, arg1, arg2);

  str = (char *) ls_malloc(len + 1);
  gmp_snprintf(str, len + 1, fmt, arg1, arg2);

  return str;
}

/* vim: set et ts=2 sw=2 cin: */
