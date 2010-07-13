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

#ifndef RUNTIME_NUMBER_H_
#define RUNTIME_NUMBER_H_

#include <stdbool.h>

#include <gmp.h>

/* lsrt's representation of real numbers */
struct ls_real
{
  int type;
  union {
    int v;
    mpz_t *z;
    mpq_t *q;
    mpf_t *f;
    void *content;
  };
};

static void
_re_duplicate(struct ls_real *dst, struct ls_real *src)
{
  dst->type = src->type;

  switch(dst->type) {
  case 0:
    dst->v = src->v;
    break;
  case 1:
    dst->z = (mpz_t *) ls_malloc(sizeof *dst->z);
    mpz_init(*dst->z);
    mpz_set(*dst->z, *src->z);
    break;
  case 2:
    dst->q = (mpq_t *) ls_malloc(sizeof *dst->q);
    mpq_init(*dst->q);
    mpq_set(*dst->q, *src->q);
    break;
  case 3:
    dst->f = (mpf_t *) ls_malloc(sizeof *dst->f);
    mpf_init(*dst->f);
    mpf_set(*dst->f, *src->f);
    break;
  }
}

static void
_re_clear(struct ls_real *dst)
{
  switch(dst->type) {
  case 1:
    mpz_clear(*dst->z);
    ls_free(dst->z);
    break;
  case 2:
    mpq_clear(*dst->q);
    ls_free(dst->q);
    break;
  case 3:
    mpf_clear(*dst->f);
    ls_free(dst->f);
    break;
  }
}

#define mpq_set_si_single(_a, _b) mpq_set_si(_a, _b, 1)
#define MP_PROMOTE_CASE(_n, _i, _o)                                     \
  case _n: {                                                            \
    mp ## _o ## _t *tmp = (mp ## _o ## _t *) ls_malloc(sizeof *tmp);    \
    mp ## _o ## _init (*tmp);                                           \
    mp ## _o ## _set_ ## _i(*tmp, *v);                                  \
    dst->type = _n;                                                     \
    dst->_o = tmp;                                                      \
    break;                                                              \
  }

static int
_re_promote(struct ls_real *dst, int dtype, int delold)
{
  int r = 0;
  int t = dst->type;
  void *old = NULL;

  if (dst->type >= dtype)
    return r;

  if (t == 0) {
    int *v = &dst->v;
    switch (dtype) {
      MP_PROMOTE_CASE(1, si, z);
      MP_PROMOTE_CASE(2, si_single, q);
      MP_PROMOTE_CASE(3, si, f);
    }
  } else if (t == 1) {
    mpz_t *v = dst->z;
    switch (dtype) {
      MP_PROMOTE_CASE(2, z, q);
      MP_PROMOTE_CASE(3, z, f);
    }
    old = v;
  } else if (t == 2) {
    mpq_t *v = dst->q;
    switch (dtype) {
      MP_PROMOTE_CASE(3, q, f);
    }
    old = v;
  }

  if (old && delold)
    ls_free(old);
  else if (old)
    r = 1;

  return r;
}

#undef mpq_set_si_single
#undef MP_PROMOTE_CASE

static void
_re_canonicalize(struct ls_real *dst)
{
  if (dst->type == 2) {
    mpq_canonicalize(*dst->q);
    if (mpz_cmp_si(mpq_denref(*dst->q), 1) == 0) {
      mpq_t *q = dst->q;
      dst->type = 1;
      dst->z = (mpz_t *) ls_malloc(sizeof *dst->z);
      mpz_init(*dst->z);
      mpz_set_q(*dst->z, *q);
      mpq_clear(*q);
      ls_free(q);
    }
  }

  if (dst->type == 1) {
    if (mpz_fits_sint_p(*dst->z)) {
      dst->type = 0;
      mpz_t *z = dst->z;
      dst->v = mpz_get_si(*z);
      mpz_clear(*z);
      ls_free(z);
    }
  }
}


/* Check signed overflow of `a - b', `a + b' and `a * b'.
 *
 * a = -1, b = 2147483647 (INT_MAX)
 * a + b = 2147483646, overflow = 0
 * a - b = -2147483648, overflow = 0
 *
 * a = -2, b = 2147483647
 * a + b = 2147483645, overflow = 0
 * a - b = 2147483647, overflow = 1
 *
 * a = 2, b = -2147483647
 * a + b = -2147483645, overflow = 0
 * a - b = -2147483647, overflow = 1
 *
 * a = 2147483647, b = 1
 * a + b = -2147483648, overflow = 1
 * a - b = 2147483646, overflow = 0
 *
 * a = -2147483647, b = -2
 * a + b = 2147483647, overflow = 1
 * a - b = -2147483645, overflow = 0
 *
 * a = -2147483647, b = -1
 * a + b = -2147483648, overflow = 0
 * a - b = -2147483646, overflow = 0
 */
static bool
__sub_overflow_p(int a, int b, int* res)
{
  int t = a - b;
  if ((a < 0) == (b > 0) && ((t < 0) != (a < 0)))
    return true;

  *res = t;
  return false;
}

static bool
__add_overflow_p(int a, int b, int* res)
{
  int t = a + b;
  if ((a < 0) == (b < 0) && ((t < 0) != (a < 0)))
    return true;

  *res = t;
  return false;
}

static bool
__mul_overflow_p(int a, int b, int* res)
{
  int64_t x = (int64_t) a * b;
  int y = (int) (x >> 31);

  if (y != 0 && y != -1)
    return true;

  *res = (int) x;
  return false;
}

/*
 * ls_real arithmetic: _re_arith2
 *   for efficiency consideration, _re_arith2 is a little complex,
 * since there are many different situations to handle.
 *   below macros seems a little cryptic, but it reduces the redundency
 * and makes things easier than not
 */
static inline unsigned int
__si_to_ui(int si, int *sgn)
{
  if (si > 0) {
    *sgn = 1;
    return (unsigned) si;
  }

  if (si < 0) {
    *sgn = -1;
    return (unsigned)(-(si + 1)) + 1;   /* INT_MIN */
  }

  *sgn = 0;
  return 0;
}

#define ls_mpz_div         mpz_divexact
#define ls_mpz_div_ui      mpz_divexact_ui
#define ls_mpq_div         mpq_div
#define ls_mpf_div         mpf_div
#define ls_mpf_div_ui      mpf_div_ui

#define MP_ARITH_CASE_SIGN(_n, _sfx, _i, _op2, _sgn)                    \
  case _n: {                                                            \
    switch(op) {                                                        \
    case '+':                                                           \
      if (_sgn != -1)                                                   \
        mp ## _i ## _add ## _sfx(*dst->_i, *dst->_i, _op2);             \
      else                                                              \
        mp ## _i ## _sub ## _sfx(*dst->_i, *dst->_i, _op2);             \
      break;                                                            \
    case '-':                                                           \
      if (_sgn != -1)                                                   \
        mp ## _i ## _sub ## _sfx(*dst->_i, *dst->_i, _op2);             \
      else                                                              \
        mp ## _i ## _add ## _sfx(*dst->_i, *dst->_i, _op2);             \
      break;                                                            \
    case '*':                                                           \
      mp ## _i ## _mul ## _sfx(*dst->_i, *dst->_i, _op2);               \
      if (_sgn == -1)                                                   \
        mp ## _i ## _neg(*dst->_i, *dst->_i);                           \
      break;                                                            \
    case '/':                                                           \
      ls_mp ## _i ## _div ## _sfx(*dst->_i, *dst->_i, _op2);            \
      if (_sgn == -1)                                                   \
        mp ## _i ## _neg(*dst->_i, *dst->_i);                           \
      break;                                                            \
    }                                                                   \
    break;                                                              \
  }

#define MP_ARITH_RATIONAL_CASE_SIGN(_sfx, _op2, _sgn)                   \
  case 2: {                                                             \
    switch(op) {                                                        \
    case '+':                                                           \
      if (_sgn != -1)                                                   \
        mpz_addmul ## _sfx(mpq_numref(*dst->q),                         \
                           mpq_denref(*dst->q), _op2);                  \
      else                                                              \
        mpz_submul ## _sfx(mpq_numref(*dst->q),                         \
                           mpq_denref(*dst->q), _op2);                  \
      break;                                                            \
    case '-':                                                           \
      if (_sgn != -1)                                                   \
        mpz_submul ## _sfx(mpq_numref(*dst->q),                         \
                           mpq_denref(*dst->q), _op2);                  \
      else                                                              \
        mpz_addmul ## _sfx(mpq_numref(*dst->q),                         \
                           mpq_denref(*dst->q), _op2);                  \
      break;                                                            \
    case '*':                                                           \
      mpz_mul ## _sfx(mpq_numref(*dst->q), mpq_numref(*dst->q), _op2);  \
      if (_sgn == -1)                                                   \
        mpz_neg(mpq_numref(*dst->q), mpq_numref(*dst->q));              \
      break;                                                            \
    case '/':                                                           \
      mpz_mul ## _sfx(mpq_denref(*dst->q), mpq_denref(*dst->q), _op2);  \
      if (_sgn == -1)                                                   \
        mpz_neg(mpq_denref(*dst->q), mpq_denref(*dst->q));              \
      break;                                                            \
    }                                                                   \
    break;                                                              \
  }

/* rely on compiler to cut off the unused conditional for constant _sgn */
#define MP_ARITH_CASE(_n, _sfx, _i, _op2)       \
  MP_ARITH_CASE_SIGN(_n, _sfx, _i, _op2, 1)
#define MP_ARITH_RATIONAL_CASE(_sfx, _op2)      \
  MP_ARITH_RATIONAL_CASE_SIGN(_sfx, _op2, 1)

/*
 *   avoid allocation as much as possible, i.e. avoid unnecessary
 * _re_promote
 */
static void
_re_arith2(const char op, struct ls_real *dst, struct ls_real *src)
{
  /* changing src is not allowed */
  int need_free = 0;
  struct ls_real tmp = *src;
  struct ls_real *op2 = &tmp;

  /* do best to stay in plain integer */
  if (dst->type == 0 && src->type == 0) {
    int need_promote = 0;
    int a = dst->v, b = src->v;

    switch (op) {
    case '+':
      if (__add_overflow_p(a, b, &dst->v))
        need_promote = 1;
      break;
    case '-':
      if (__sub_overflow_p(a, b, &dst->v))
        need_promote = 1;
      break;
    case '*':
      if (__mul_overflow_p(a, b, &dst->v))
          need_promote = 1;
      break;
    case '/':
      if (a % b != 0)
        need_promote = 1;
      else
        dst->v /= b;
      break;
    }

    if (!need_promote)
      return;

    if (op == '/')
      _re_promote(dst, 2, 1);
    else
      _re_promote(dst, 1, 1);
  }

  /* promote to mpq if src is {int, mpz} but not divisible */
  if (op == '/' && dst->type == 1) {
    if (src->type == 0) {
      int sgn;
      if (!mpz_divisible_ui_p(*dst->z, __si_to_ui(src->v, &sgn)))
        _re_promote(dst, 2, 1);
    } else if (src->type == 1) {
      if (!mpz_divisible_p(*dst->z, *src->z))
        _re_promote(dst, 2, 1);
    }
  }

  /* if src is integer, we don't bother create any gmp referent */
  if (src->type == 0) {
    int si = src->v;
    unsigned int ui;
    int sgn;
    ui = __si_to_ui(si, &sgn);
    switch (dst->type) {
      MP_ARITH_CASE_SIGN(1, _ui, z, ui, sgn);
      MP_ARITH_RATIONAL_CASE_SIGN(_ui, ui, sgn);
      MP_ARITH_CASE_SIGN(3, _ui, f, ui, sgn);
    }
    _re_canonicalize(dst);
    return;
  }

  /* rational op integer */
  if (dst->type == 2 && src->type == 1) {
    switch (dst->type) {
      MP_ARITH_RATIONAL_CASE(, *src->z);
    }
    _re_canonicalize(dst);
    return;
  }

  /* others */
  if (dst->type < src->type)
    _re_promote(dst, src->type, 1);


  if (src->type < dst->type) {
    /* we can't promote src but only op2 */
    need_free = _re_promote(op2, dst->type, 0);
  }

  switch (dst->type) {
    MP_ARITH_CASE(1, , z, *op2->z);
    MP_ARITH_CASE(2, , q, *op2->q);
    MP_ARITH_CASE(3, , f, *op2->f);
  }

  _re_canonicalize(dst);
  if (need_free)
    _re_clear(op2);
}

#undef MP_ARITH_CASE
#undef MP_ARITH_CASE_SIGN
#undef MP_ARITH_RATIONAL_CASE_SIGN
#undef ls_mpz_div
#undef ls_mpz_div_ui
#undef ls_mpq_div
#undef ls_mpf_div
#undef ls_mpf_div_ui

/* returns sgn(dst-src) */
static int
_re_order2(struct ls_real *dst, struct ls_real *src)
{
  int result = 0;

  if (src->type == 0) {
    switch(dst->type) {
    case 0:
#define _sgn(x) ((x) < 0)? -1: ((x) != 0)
      result = _sgn(dst->v - src->v);
#undef _sgn
      break;
    case 1:
      result = mpz_cmp_si(*dst->z, src->v);
      break;
    case 2:
      result = mpq_cmp_si(*dst->q, src->v, 1);
      break;
    case 3:
      result = mpf_cmp_si(*dst->f, src->v);
      break;
    }
  } else if (dst->type == 0) {
    switch(src->type) {
    case 1:
      result = - mpz_cmp_si(*src->z, dst->v);
      break;
    case 2:
      result = - mpq_cmp_si(*src->q, dst->v, 1);
      break;
    case 3:
      result = - mpf_cmp_si(*src->f, dst->v);
      break;
    }
  } else if (dst->type == 1 && src->type == 2) {
    mpz_t tmp;
    int s1, s2;

    s1 = mpz_sgn(*dst->z);
    s2 = mpq_sgn(*src->q);

    if (s1 != s2)
      result = (s1 > s2) ? 1: -1;
    else {
      mpz_init_set(tmp, *dst->z);
      mpz_mul(tmp, tmp, mpq_denref(*src->q));
      result = mpz_cmp(tmp, mpq_numref(*src->q));
      mpz_clear(tmp);
    }
  } else if (dst->type == 2 && src->type == 1) {
    mpz_t tmp;
    int s1, s2;

    s1 = mpq_sgn(*dst->q);
    s2 = mpz_sgn(*src->z);

    if (s1 != s2)
      result = (s1 > s2) ? 1: -1;
    else {
      mpz_init_set(tmp, *src->z);
      mpz_mul(tmp, tmp, mpq_denref(*dst->q));
      result = mpz_cmp(mpq_numref(*dst->q), tmp);
      mpz_clear(tmp);
    }
  } else if (dst->type == src->type) {
    switch(dst->type) {
    case 1:
      result = mpz_cmp(*dst->z, *src->z);
      break;
    case 2:
      result = mpq_cmp(*dst->q, *src->q);
      break;
    case 3:
      result = mpf_cmp(*dst->f, *src->f);
      break;
    }
  } else if (dst->type == 3) {
    mpf_t tmp;
    int s1, s2;

    s1 = mpf_sgn(*dst->f);
    if (src->type == 1)
      s2 = mpz_sgn(*src->z);
    else
      s2 = mpq_sgn(*src->q);

    if (s1 != s2)
      result = (s1 > s2) ? 1: -1;
    else {
      mpf_init(tmp);
      if (src->type == 1)
        mpf_set_z(tmp, *src->z);
      else
        mpf_set_q(tmp, *src->q);

      result = mpf_cmp(*dst->f, tmp);
      mpf_clear(tmp);
    }
  } else if (src->type == 3) {
    mpf_t tmp;
    int s1, s2;

    s2 = mpf_sgn(*src->f);
    if (dst->type == 1)
      s1 = mpz_sgn(*dst->z);
    else
      s1 = mpq_sgn(*dst->q);

    if (s1 != s2)
      result = (s1 > s2) ? 1: -1;
    else {
      mpf_init(tmp);
      if (dst->type == 1)
        mpf_set_z(tmp, *dst->z);
      else
        mpf_set_q(tmp, *dst->q);

      result = mpf_cmp(tmp, *src->f);
      mpf_clear(tmp);
    }
  }

  return result;
}

/*
 * duplicating instead of reference for better usability
 */
static void
_re_get_lso_re_ref(struct ls_real *dst, struct ls_object *obj)
{
  dst->type = lso_number_type_re(obj);
  dst->content = obj->u1.ptr;
}

static void
_re_get_lso_re(struct ls_real *dst, struct ls_object *obj)
{
  struct ls_real tmp;

  _re_get_lso_re_ref(&tmp, obj);
  _re_duplicate(dst, &tmp);
}

static void
_re_get_lso_im_ref(struct ls_real *dst, struct ls_object *obj)
{
  if (lso_is_complex(obj)) {
    dst->type = lso_number_type_im(obj);
    dst->content = obj->u2.ptr;
  } else {
    dst->type = 0;
    dst->v = 0;
  }
}

static void
_re_get_lso_im(struct ls_real *dst, struct ls_object *obj)
{
  struct ls_real tmp;

  _re_get_lso_im_ref(&tmp, obj);
  _re_duplicate(dst, &tmp);
}

static void
_re_update_lso_re(struct ls_object *obj, struct ls_real *src)
{
  int type = lso_number_type_re(obj);

  switch(type) {
  case 1:
    mpz_clear(*(mpz_t *)obj->u1.ptr);
    ls_free(obj->u1.ptr);
    break;
  case 2:
    mpq_clear(*(mpq_t *)obj->u1.ptr);
    ls_free(obj->u1.ptr);
    break;
  case 3:
    mpf_clear(*(mpf_t *)obj->u1.ptr);
    ls_free(obj->u1.ptr);
    break;
  }

  obj->type &= ~ls_num_re_mask;
  obj->type |= src->type << ls_num_re_shift;
  obj->u1.ptr = src->content;
}

static void
_re_update_lso_im(struct ls_object *obj, struct ls_real *src)
{
  int type = lso_number_type_im(obj);

  switch(type) {
  case 1:
    mpz_clear(*(mpz_t *)obj->u2.ptr);
    ls_free(obj->u2.ptr);
    break;
  case 2:
    mpq_clear(*(mpq_t *)obj->u2.ptr);
    ls_free(obj->u2.ptr);
    break;
  case 3:
    mpf_clear(*(mpf_t *)obj->u2.ptr);
    ls_free(obj->u2.ptr);
    break;
  }

  obj->type &= ~(ls_num_complex | ls_num_im_mask);
  obj->type |= src->type << ls_num_im_shift;
  obj->u2.ptr = src->content;

  if (!(src->type == 0 && src->v == 0))
    obj->type |= ls_num_complex;
}

static struct ls_object *
_re_new_lso_re_im(struct ls_real *re, struct ls_real *im)
{
  struct ls_object *lso;

  lso = lsrt_new_object(ls_t_number);
  lso->type |= re->type << ls_num_re_shift;
  lso->u1.ptr = re->content;

  if (im && !(im->type == 0 && im->v == 0)) {
    lso->type |= ls_num_complex;
    lso->type |= im->type << ls_num_im_shift;
    lso->u2.ptr = im->content;
  }

  return lso;
}

static void
_re_neg(struct ls_real *dst)
{
  switch(dst->type) {
  case 0:
    dst->v = 0 - dst->v;
    break;
  case 1:
    mpz_neg(*dst->z, *dst->z);
    break;
  case 2:
    mpq_neg(*dst->q, *dst->q);
    break;
  case 3:
    mpf_neg(*dst->f, *dst->f);
    break;
  }
}

static void
_re_sqrt(struct ls_real *dst)
{
  if (dst->type == 0)
    _re_promote(dst, 1, 1);

  if (dst->type == 1) {
    if (mpz_perfect_square_p(*dst->z)) {
      mpz_sqrt(*dst->z, *dst->z);
      _re_canonicalize(dst);
    } else
      _re_promote(dst, 3, 1);
  }

  if (dst->type == 2) {
    if (mpz_perfect_square_p(mpq_numref(*dst->q)) &&
        mpz_perfect_square_p(mpq_denref(*dst->q))) {
      mpz_sqrt(mpq_numref(*dst->q), mpq_numref(*dst->q));
      mpz_sqrt(mpq_denref(*dst->q), mpq_denref(*dst->q));
      /* no need to canonicalize, it's guaranteed :-) */
    } else
      _re_promote(dst, 3, 1);
  }

  if (dst->type == 3)
    mpf_sqrt(*dst->f, *dst->f);
}

#endif

/* vim: set et ts=2 sw=2 cin: */
