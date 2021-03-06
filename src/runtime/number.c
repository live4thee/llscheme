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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "utils.h"
#include "number.h"
/**********************************************************
 * Section 6.2. Numbers
 **********************************************************/

struct ls_object *
lsrt_duplicate_number(struct ls_object *num)
{
  struct ls_real re, im;

  _re_get_lso_re(&re, num);
  _re_get_lso_im(&im, num);

  return _re_new_lso_re_im(&re, &im);
}

/**********************************************************
 * Section 6.2.5 Numerical operations
 **********************************************************/

/* predicates */

static int
lso_is_real(struct ls_object *number)
{
  int r = 1;

  if (!lso_is_number(number))
    r = 0;

  if (lso_is_complex(number)) {
    if (lso_number_type_im(number) == 3 &&  /* inexact */
        mpf_cmp_ui(*(mpf_t *)number->u2.ptr, 0) == 0)
      r = 1;
    else
      r = 0;
  }

  return r;
}

static int
lso_is_integer(struct ls_object *number)
{
  int type = lso_number_type_re(number);

  if (!lso_is_real(number) || type == 2)
    return 0;

  if (type == 3)
    return mpf_integer_p(*(mpf_t *)number->u2.ptr);

  return 1;
}

static int
lso_is_exact(struct ls_object *number)
{
  if (lso_number_type_re(number) == 3 ||
      (lso_is_complex(number) && lso_number_type_im(number) == 3))
    return 0;

  return 1;
}

BUILTIN("number?", numberp);
struct ls_object *lsrt_builtin_numberp(int argc, struct ls_object *args[],
                                       struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  return lso_is_number(args[0]) ? &global_true_obj : &global_false_obj;
}

BUILTIN("complex?", complexp);
struct ls_object *lsrt_builtin_complexp(int argc, struct ls_object *args[],
                                        struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  return lso_is_complex(args[0]) ? &global_true_obj : &global_false_obj;
}

BUILTIN("real?", realp);
struct ls_object *lsrt_builtin_realp(int argc, struct ls_object *args[],
                                     struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  return lso_is_real(args[0]) ? &global_true_obj : &global_false_obj;
}

BUILTIN("rational?", rationalp);
struct ls_object *lsrt_builtin_rationalp(int argc, struct ls_object *args[],
                                         struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);

  /* we need a lso_is_rational() */
  return lso_is_real(args[0]) ? &global_true_obj : &global_false_obj;
}

BUILTIN("integer?", integerp);
struct ls_object *lsrt_builtin_integerp(int argc, struct ls_object *args[],
                                        struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  return lso_is_integer(args[0]) ? &global_true_obj : &global_false_obj;
}

BUILTIN("exact?", exactp);
struct ls_object *lsrt_builtin_exactp(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_number_p(args[0]);
  return lso_is_exact(args[0]) ? &global_true_obj : &global_false_obj;
}

BUILTIN("inexact?", inexactp);
struct ls_object *lsrt_builtin_inexactp(int argc, struct ls_object *args[],
                                        struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  lsrt_number_p(args[0]);
  return lso_is_exact(args[0]) ? &global_false_obj : &global_true_obj;
}

/* ordering */

static struct ls_object *
_order(char op, int argc, struct ls_object *args[])
{
  struct ls_real op1, op2;
  int res;
  int i;

  if (argc == 0)
    return &global_true_obj;

  if (op == '=')
    lsrt_number_p(args[0]);
  else if (!lso_is_real(args[0]))
    lsrt_error("expected real number for ordering");

  for (i = 1; i < argc; i++) {
    if (op == '=')
      lsrt_number_p(args[i]);
    else if (!lso_is_real(args[i]))
      lsrt_error("expected real number for ordering");

    _re_get_lso_re_ref(&op1, args[i - 1]);
    _re_get_lso_re_ref(&op2, args[i]);

    res = _re_order2(&op1, &op2);

    if (op == '=' && res == 0) {
      _re_get_lso_im_ref(&op1, args[i - 1]);
      _re_get_lso_im_ref(&op2, args[i]);

      res = _re_order2(&op1, &op2);
    }

    switch (op) {
    case '=':
      if (res != 0)
        return &global_false_obj;
      break;
    case '<':
      if (res >= 0)
        return &global_false_obj;
      break;
    case '>':
      if (res <= 0)
        return &global_false_obj;
      break;
    case 'l':
      if (res > 0)
        return &global_false_obj;
      break;
    case 'g':
      if (res < 0)
        return &global_false_obj;
      break;
    default:
      lsrt_error("unsupported op: %c", op);
      break;
    }
  }

  return &global_true_obj;
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

/* arithmetic */

static void
_arith2(const char op, struct ls_object *dst, struct ls_object *src)
{
  struct ls_real dr, sr, di, si;
  int is_complex;

  is_complex = lso_is_complex(dst) || lso_is_complex(src);

  _re_get_lso_re(&dr, dst);
  _re_get_lso_re(&sr, src);
  _re_get_lso_im(&di, dst);
  _re_get_lso_im(&si, src);

  switch(op) {
  case '+':
  case '-':
    _re_arith2(op, &dr, &sr);
    if (is_complex)
      _re_arith2(op, &di, &si);
    break;
  case '/':
    if (!is_complex) {
      _re_arith2(op, &dr, &sr);
      break;
    } else {
      struct ls_real r, i;
      _re_duplicate(&r, &sr);
      _re_duplicate(&i, &si);

      _re_arith2('*', &r, &sr);    /* c^2 */
      _re_arith2('*', &i, &si);    /* d^2 */
      _re_arith2('+', &r, &i);     /* c^2 + d^2 */

      _re_neg(&si);

      _re_arith2('/', &sr, &r);    /* src.re = c / (c^2 + d^2) */
      _re_arith2('/', &si, &r);    /* src.im = -di / (c^2 + d^2) */

      _re_clear(&r);
      _re_clear(&i);
    }
    /* FALLTHROUGH */
  case '*':
    if (!is_complex) {
      _re_arith2('*', &dr, &sr);
    } else {
      struct ls_real r, i;
      _re_duplicate(&r, &dr);
      _re_duplicate(&i, &di);

      _re_arith2('*', &dr, &sr);    /* ac  */
      _re_arith2('*', &di, &sr);    /* bci */

      _re_arith2('*', &r, &si);     /* adi */
      _re_arith2('*', &i, &si);     /* bd  */

      _re_arith2('-', &dr, &i);     /* dst.re = ac - bd */
      _re_arith2('+', &di, &r);     /* dst.im = (ad + bc)i */

      _re_clear(&r);
      _re_clear(&i);
    }
    break;
  }

  _re_update_lso_re(dst, &dr);
  _re_update_lso_im(dst, &di);
  _re_clear(&sr);
  _re_clear(&si);
}

static struct ls_object *
_arith(const char op, int argc, struct ls_object *args[])
{
  struct ls_object *ret = NULL;
  int i = 0;

  /* '/' and '-' requires at least one operand */
  if ((op == '/' || op == '-') && (argc == 0))
    lsrt_error("invalid number of operand (op=`%c', argc=%d)", op, argc);

  switch (op) {
    case '+': ret = lsrt_new_number(0); break;
    case '*': ret = lsrt_new_number(1); break;

    case '-': if (argc == 1) {
                ret = lsrt_new_number(0);
              } else {
                lsrt_number_p(args[0]);
                ret = lsrt_duplicate_number(args[0]);
                i = 1;
              }
              break;

    case '/': if (argc == 1) {
                ret = lsrt_new_number(1);
              } else {
                lsrt_number_p(args[0]);
                ret = lsrt_duplicate_number(args[0]);
                i = 1;
              }
              break;
    default:
      lsrt_error("invalid operator: %c", op);
  }

  for (; i < argc; i++) {
    lsrt_number_p(args[i]);
    _arith2(op, ret, args[i]);
  }

  return ret;
}

BUILTIN("+", plus);
struct ls_object *lsrt_builtin_plus(int argc, struct ls_object *args[],
                                    struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
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
  return _arith('*', argc, args);
}

BUILTIN("/", divide);
struct ls_object *lsrt_builtin_divide(int argc, struct ls_object *args[],
                                      struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 0, argc);
  return _arith('/', argc, args);
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
  static char *cstr = ".0123456789aAbBcCdDeEfF";
  char *res;
  int req;

  if (ch == 0)
    return 0;

  if (radix <= 10)
    req = radix;
  else
    req = 2 * radix - 10;

  res = strchr(cstr, ch);
  if (res)
    return (res - cstr) <= req;

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
  flag |= stonf_prec_mask &
    (precision == 's'? 16 :
     precision == 'f'? 32 :
     precision == 'd'? 64 :
     precision == 'l'? 128 : 0);

  *out = ptr;
  return flag;

 err:
  flag = 0;
  return flag;

}

/* not canonicalized */
void _lsrt_mpq_set_decimal(mpq_t q, const char *ptr, int radix)
{
  mpz_t num, dec, exp;
  mpq_t addend;
  int ex, expdigits;
  int s, e;
  char ch = '+';

  if (radix != 10)
    lsrt_error("unsupported radix for exact decimal number: %d", radix);

  mpz_init_set_ui(num, 0);
  mpz_init_set_ui(dec, 0);
  mpz_init(exp);
  mpq_init(addend);

  e = 0;
  sscanf(ptr, "%[+-]%n", &ch, &e);
  ptr += e;

  e = 0;
  gmp_sscanf(ptr, "%Zd%n", num, &e);
  ptr += e;

  s = 0;
  e = 0;
  expdigits = 0;
  gmp_sscanf(ptr, ".%n%Zd%n", &s, dec, &e);
  ptr += e;
  if (e >= s)
    expdigits = e - s;

  s = 0;
  e = 0;
  ex = 0;
  sscanf(ptr, "@%n%d%n", &s, &ex, &e);
  if (e - s >= 5 || ex >= 256)
    lsrt_error("decimal number out of range");

  mpz_set(mpq_numref(q), dec);
  mpz_ui_pow_ui(mpq_denref(q), radix, expdigits);
  mpq_set_z(addend, num);
  mpq_add(q, q, addend);

  if (ex > 0) {
    mpz_ui_pow_ui(exp, 10, ex);   /* exponent is always in radix 10 */
    mpz_mul(mpq_numref(q), mpq_numref(q), exp);
  } else if (ex < 0) {
    mpz_ui_pow_ui(exp, 10, -ex);
    mpz_mul(mpq_denref(q), mpq_denref(q), exp);
  }

  if (ch == '-')
    mpz_neg(mpq_numref(q), mpq_numref(q));

  mpq_clear(addend);
  mpz_clear(exp);
  mpz_clear(num);
  mpz_clear(dec);
}

void _ston(struct ls_object *obj, const char *num, int defradix)
{
  char *dup = strdup(num), *ptr = dup;
  char *end;
  char saved;
  int exactness = -1, radix = -1;
  int flag, flag2, type, allowre = 1, allowim = 1;
  struct ls_real re = { 0, { 0 } };

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

  /* parsing */
  flag = _ston_parse_real(ptr, &end, radix);
  if (!(flag & stonf_valid))
    goto err;

  if (*end == '/') {
    type = 2;
    flag2 = _ston_parse_real(end + 1, &end, radix);
    /* sanity for rational */
    if (!(flag2 & stonf_valid))
      goto err;
    if ((flag & stonf_decimal) ||
        (flag2 & stonf_signed) ||
        (flag2 & stonf_decimal))
      goto err;
  }

  if (flag & stonf_decimal)
    type = 3;

  if ((*end == 'i' && allowim == 0) ||
      (*end != 'i' && allowre == 0))
    goto err;

  /* generating initial result */
  if ((*end == 'i' && (flag & stonf_iunit))) {
    type = 0;
    if (!(flag & stonf_signed))
      goto err;
    if (*ptr == '+')
      re.v = 1;
    else
      re.v = -1;
  }


  saved = *end;
  *end = '\0';

  /* no _re_clear can be called from now until the content is fully set up */
  re.type = type;
  if (*ptr == '+')  /* tackle around strange behavior of mpx_set_str */
      ptr++;
  switch (re.type) {
  case 1:
    re.z = (mpz_t *) ls_malloc(sizeof *re.z);
    mpz_init(*re.z);
    mpz_set_str(*re.z, ptr, radix);
    break;
  case 2:
    re.q = (mpq_t *) ls_malloc(sizeof *re.q);
    mpq_init(*re.q);
    mpq_set_str(*re.q, ptr, radix);
    break;
  case 3:
    if (exactness == 1) {
      /*
       * unfortunately, #e<decimal> has to be handled here manually,
       * the result inexact->exact is normally not acceptable
       */
      re.type = 2;
      re.q = (mpq_t *) ls_malloc(sizeof *re.q);
      mpq_init(*re.q);
      _lsrt_mpq_set_decimal(*re.q, ptr, radix);
    } else {
      re.f = (mpf_t *) ls_malloc(sizeof *re.f);
      /*
       * TODO: gmp only guarantees that result prec is no lower
       * than specified, check R5RS requirement
       */
      if (flag & stonf_prec_mask)
        mpf_init2(*re.f, flag & stonf_prec_mask);
      else
        mpf_init(*re.f);
      mpf_set_str(*re.f, ptr, radix);
    }
    break;
  }

  *end = saved;

  /* canonicalize and transform */
  if (exactness == 0 ||
      ((flag & stonf_pound) && exactness != 1))
    _re_promote(&re, 3, 0);

  _re_canonicalize(&re);

  /* store */
  if (*end == 'i') {
    if (!(flag & stonf_signed))  /* imaginary must be signed */
      goto err;
    allowim = 0;
    _re_update_lso_im(obj, &re);
    end++;
  } else {
    allowre = 0;
    _re_update_lso_re(obj, &re);
  }

  if (*end) {
    ptr = end;
    goto restart;
  }

  free(dup);
  return;

 err:
  free(dup);
  _re_clear(&re);
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
  const char* str;

  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 2, argc);
  lsrt_string_p(args[0]);

  str = lso_string_get(args[0]);
  if (unlikely(strlen(str) == 0)) {
    return &global_false_obj;
  }

  ret = lsrt_new_object(ls_t_number);
  if (argc == 2 && lso_is_simplenumber(args[1]))
    radix = lso_simplenumber_get(args[1]);

  _ston(ret, str, radix);

  return ret;
}

char *_ntos(const struct ls_object *number, int radix)
{
  int is_complex, type_re, type_im;
  char fmt[20];
  char *str;
  int len, n;
  void *arg1 = NULL, *arg2 = NULL;

  fmt[0] = '\0';

  /* TODO: radix other than 10 */
  UNUSED_ARGUMENT(radix);

  is_complex = lso_is_complex(number);
  type_re = lso_number_type_re(number);
  type_im = lso_number_type_im(number);

  switch(type_re) {
  case 0:
    if (is_complex && number->u1.val == 0) {
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

  if (is_complex) {
    switch(type_im) {
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
