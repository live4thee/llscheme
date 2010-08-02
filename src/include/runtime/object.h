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

#ifndef OBJECT_H_
#define OBJECT_H_

#include <stdint.h>

#ifdef __cplusplus
# define TARGET extern "C"
#else
# define TARGET
#endif

#define ls_tm_major    0xff
#define ls_tm_sub      0xffffff00u

#define ls_t_unspec    0  /* unspecified */
#define ls_t_number    1

#define ls_num_complex       0x10000
#define ls_num_re_shift      8
#define ls_num_re_mask       0xf00
#define ls_num_re_small      0x000
#define ls_num_re_big        0x100
#define ls_num_re_rational   0x200
#define ls_num_re_inexact    0x300
#define ls_num_im_shift      12
#define ls_num_im_mask       0xf000
#define ls_num_im_small      0x0000
#define ls_num_im_big        0x1000
#define ls_num_im_rational   0x2000
#define ls_num_im_inexact    0x3000

#define ls_t_boolean   2
#define ls_t_vector    3
#define ls_t_symbol    4
#define ls_t_string    5
#define ls_t_pair      6
#define ls_t_nil       7
#define ls_t_func      8

#define ls_t_unknown   0xff

/*
 * TODO: how to ensure consistency??
 *  - consistency between runtime and compiler after changes
 *  - consistency between C/C++/llvm IR
 */

#ifdef _LLSCHEME_RUNTIME
/*
 * The LLVM IR assembly representation
 *   %ls_object = type { i32, %anon.um, %anon.um }
 *   &anon.um = type { i8* }      ; union { i32, i8* }
 */
/*
 * 8.5.1: ``...the braces shall only contain an initializer for the
 * first member of the union.''
 * put ptr before val to avoid losing precision, since
 * sizeof(void *) \ge sizeof(uint32_t)
 */
TARGET struct ls_object
{
  uint32_t type;
  union {
    void *ptr;
    int32_t val;
  } u1;
  union {
    void *ptr;
    int32_t val;
  } u2;
};

TARGET typedef
struct ls_object * (*ls_func_type) (int argc, struct ls_object *args[],
                                    struct ls_object *freelist[]);

/*
 * usage:
 *
 * void:   u1, u2 undefined
 * number: subtypes:
 *   ls_num_complex:
 *     u1: real part    u2: imaginary part
 *   ls_num_{re,im}_{small,big,rational,inexact}
 *     small is embedded u{1,2}.val
 *     big is big integer u{1,2}.ptr -> mpz_t
 *     rational is exact rational u{1,2}.ptr -> mpq_t
 *     inexact is float u{1,2}.ptr -> mpf_t
 *
 * boolean: u1.val <= boolean, u2 undefined
 * vector: u1.val <= length, u2.ptr <= address of first element
 * symbol: u1.ptr <= the object pointing to, u2.ptr <= symbol
 * string: u1.ptr <= string, u2 undefined
 * pair: u1.ptr <= car object,  u2.ptr <= cdr object
 * func: u1.ptr <= function pointer, u2 <== free arguments
 */

#define lso_type(x)           ((x)->type & ls_tm_major)
#define lso_sub_type(x)       ((x)->type & ls_tm_sub)

/* predicates */
#define lso_is_unspec(x)    (lso_type(x) == ls_t_unspec)
#define lso_is_number(x)  (lso_type(x) == ls_t_number)

#define lso_is_simplenumber(x)  ((x)->type == ls_t_number)  /* used for simple parameters */
#define lso_is_complex(x)       (lso_is_number(x) && (lso_sub_type(x) & ls_num_complex))
#define lso_number_type_re(x)   (((x)->type >> 8) & 0xf)
#define lso_number_type_im(x)   (((x)->type >> 12) & 0xf)

#define lso_is_boolean(x) (lso_type(x) == ls_t_boolean)
#define lso_is_vector(x)  (lso_type(x) == ls_t_vector)
#define lso_is_symbol(x)  (lso_type(x) == ls_t_symbol)
#define lso_is_string(x)  (lso_type(x) == ls_t_string)
#define lso_is_pair(x)    (lso_type(x) == ls_t_pair)
#define lso_is_nil(x)     (lso_type(x) == ls_t_nil)
#define lso_is_func(x)    (lso_type(x) == ls_t_func)

/* accessors */
#define lso_simplenumber_get(x) ((x)->u1.val)
#define lso_boolean_get(x)      ((x)->u1.val)
#define lso_vector_length(x)    ((x)->u1.val)
#define lso_vector_addr0(x)     ((struct ls_object**) (&(x)->u2.ptr))
#define lso_symbol_deref(x)     ((struct ls_object *) (x)->u1.ptr)
#define lso_symbol_name(x)      ((char *) (x)->u2.ptr)
#define lso_string_get(x)       ((char *) (x)->u1.ptr)
#define lso_pair_car(x)         ((struct ls_object *) (x)->u1.ptr)
#define lso_pair_cdr(x)         ((struct ls_object *) (x)->u2.ptr)
#define lso_set_car(x, t)       do { (x)->u1.ptr = t; } while(0)
#define lso_set_cdr(x, t)       do { (x)->u2.ptr = t; } while(0)
#define lso_func_get(x)         ((ls_func_type) (x)->u1.ptr)

#define lso_set_type(x, t)      do { (x)->type = t; } while(0)
#define lso_set_vec_addr0(x, a) do { (x)->u2.ptr = a; } while(0)
#define lso_number_set(x, b)   (x)->u1.val = b
#define lso_boolean_set(x, n)  (x)->u1.val = n
#define lso_string_set(x, p)   (x)->u1.ptr = p

#endif /* _LLSCHEME_RUNTIME */
#endif /* OBJECT_H_ */

/* vim: set et ts=2 sw=2 cin: */
