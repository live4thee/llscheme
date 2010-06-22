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

#define ls_t_void      0
#define ls_t_number    1
#define ls_t_boolean   2
#define ls_t_bignum    3
#define ls_t_symbol    4
#define ls_t_string    5
#define ls_t_pair      6
#define ls_t_func      7

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
TARGET struct ls_object
{
  uint32_t type;
  union {
    void *ptr;
    uint32_t val;
  } u1;
  union {
    void *ptr;
    uint32_t val;
  } u2;
};

TARGET typedef
struct ls_object * (*ls_func_type) (int argc, struct ls_object *args[],
                                    struct ls_object *freelist[]);

/*
 * usage:
 *
 * void:   u1, u2 undefined
 * number: u1.val <= number, u2 undefined
 * boolean: u1.val <= boolean, u2 undefined
 * bignum: u1.ptr <= bignum cell, u2.val  <= memory size
 * symbol: u1.ptr <= the object pointing to, u2.ptr <= symbol
 * string: u1.ptr <= string, u2.val <= memory size
 * pair: u1.ptr <= car object,  u2.ptr <= cdr object
 * func: u1.ptr <= function pointer, u2 <== free arguments
 */

/* predicates */
#define lso_is_void(x)    ((x)->type == ls_t_void)
#define lso_is_number(x)  ((x)->type == ls_t_number)
#define lso_is_boolean(x) ((x)->type == ls_t_boolean)
#define lso_is_bignum(x)  ((x)->type == ls_t_bignum)
#define lso_is_symbol(x)  ((x)->type == ls_t_symbol)
#define lso_is_string(x)  ((x)->type == ls_t_string)
#define lso_is_pair(x)    ((x)->type == ls_t_pair)
#define lso_is_func(x)    ((x)->type == ls_t_func)

/* accessors */
#define lso_number_get(x) ((x)->u1.val)
#define lso_boolean_get(x) ((x)->u1.val)
#define lso_symbol_deref(x) ((struct ls_object *) (x)->u1.ptr)
#define lso_symbol_name(x) ((char *) (x)->u2.val)
#define lso_string_get(x) ((char *) (x)->u1.val)
#define lso_string_maxsize(x) ((x)->u2.val)
#define lso_pair_car(x) ((struct ls_object *) (x)->u1.ptr)
#define lso_pair_cdr(x) ((struct ls_object *) (x)->u2.ptr)
#define lso_func_get(x) ((ls_func_type) (x)->u1.ptr)

#define lso_set_type(x, t) do { (x)->type = t; } while(0)
#define lso_number(x)   (x)->u1.val
#define lso_boolean(x)  (x)->u1.val

/* static initializers */
/*
 * use static inline for better type checking
 * copy should be eliminated when inlined, well... at least in C
 * `.val =' initializer doesn't work in C++, so we can only cast
 *
 * 8.5.1: ``...the braces shall only contain an initializer for the
 * first member of the union.''
 * put ptr before val to avoid losing precision, since
 * sizeof(void *) \ge sizeof(uint32_t)
 */
TARGET static inline struct ls_object
lso_mk_void(void)
{
  return ((struct ls_object) { ls_t_void, { 0 }, { 0 } });
}

TARGET static inline struct ls_object
lso_mk_number(uint32_t i)
{
  return ((struct ls_object) {
      ls_t_number,
      { (void *)(unsigned long) i }, { 0 } });
}

TARGET static inline struct ls_object
lso_mk_boolean(uint32_t b)
{
  return ((struct ls_object) {
      ls_t_boolean,
      { (void *)(unsigned long) b }, { 0 } });
}

/* TODO: dealing with multiple copies of the same symbol */
TARGET static inline struct ls_object
lso_mk_symbol(char *sym, struct ls_object *ref)
{
  return ((struct ls_object) {
      ls_t_symbol, { ref }, { sym } });
}

TARGET static inline struct ls_object
lso_mk_string(char *str)
{
  return ((struct ls_object) {
      ls_t_string, { str }, { 0 } });
}

TARGET static inline struct ls_object
lso_mk_pair(struct ls_object *car, struct ls_object *cdr)
{
  return ((struct ls_object) {
      ls_t_pair, { car }, { cdr } });
}

TARGET static inline struct ls_object
lso_mk_func(ls_func_type f)
{
  // get rid of c++ warnings
  return ((struct ls_object) {
      ls_t_func,
      { (void *)(const void *) f }, { 0 } });
}

#endif /* _LLSCHEME_RUNTIME */
#endif /* OBJECT_H_ */

/* vim: set et ts=2 sw=2 cin: */
