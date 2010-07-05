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

#include "utils.h"

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

#define MP_ARITH_CASE(_n, _i)                                           \
  case _n: {                                                            \
    switch(op) {                                                        \
    case '+':                                                           \
      mp ## _i ## _add(*dst->_i, *dst->_i, *op2->_i);                   \
      break;                                                            \
    case '-':                                                           \
      mp ## _i ## _sub(*dst->_i, *dst->_i, *op2->_i);                   \
      break;                                                            \
    case '*':                                                           \
      mp ## _i ## _mul(*dst->_i, *dst->_i, *op2->_i);                   \
      break;                                                            \
    case '/':                                                           \
      mp ## _i ## _div(*dst->_i, *dst->_i, *op2->_i);                   \
      break;                                                            \
    }                                                                   \
    break;                                                              \
  }

static void
_re_arith2(const char op, struct ls_real *dst, struct ls_real *src)
{
  int nf;
  struct ls_real tmp = *src;
  struct ls_real *op2 = &tmp;

  if (dst->type == 0 && op2->type == 0)
    _re_promote(dst, 1, 1);

  if (dst->type < op2->type)
    _re_promote(dst, op2->type, 1);

  if (op == '/' && dst->type == 1)
    _re_promote(dst, 2, 1);

  if (op2->type < dst->type)
    nf = _re_promote(op2, dst->type, 0);

  switch (dst->type) {
    MP_ARITH_CASE(1, z);
    MP_ARITH_CASE(2, q);
    MP_ARITH_CASE(3, f);
  }

  _re_canonicalize(dst);
  if (nf)
    _re_clear(op2);
}

#undef MP_ARITH_CASE

/*
 * after ref is taken, the content of lso is invalid,
 * the user must explicitly update back
 */
static void
_re_get_lso_re_ref(struct ls_real *dst, struct ls_object *obj)
{
  dst->type = lso_number_type_re(obj);
  dst->content = obj->u1.ptr;
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
_re_update_lso_re(struct ls_object *obj, struct ls_real *src)
{
  obj->type &= ~ls_num_re_mask;
  obj->type |= src->type << ls_num_re_shift;
  obj->u1.ptr = src->content;
}

static void
_re_update_lso_im(struct ls_object *obj, struct ls_real *src)
{
  obj->type &= ~(ls_num_complex | ls_num_re_mask);
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

#endif

/* vim: set et ts=2 sw=2 cin: */
