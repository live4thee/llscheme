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

#if !defined _LLSCHEME_RUNTIME
#define _LLSCHEME_RUNTIME
#endif

#ifndef RUNTIME_UTILS_H_
#define RUNTIME_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* The Boehm-Demers-Weiser conservative garbage collector */
#include <gc/gc.h>

#include <runtime/object.h>

#define UNUSED_ARGUMENT(x) (void)(x)
#define ls_malloc GC_malloc
#define ls_free(x) do {} while(0)

#ifdef __GNUC__
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#ifndef likely
# define likely(x)      x
#endif
#ifndef unlikely
# define unlikely(x)    x
#endif

void lsrt_memory_init(void);

/* These objects are made globally unique */
extern struct ls_object global_true_obj;
extern struct ls_object global_false_obj;
extern struct ls_object global_unspec_obj;

struct ls_object *lsrt_new_object(int type);
struct ls_object *lsrt_new_number(int32_t num);
struct ls_object *lsrt_duplicate_number(struct ls_object *num);
struct ls_object **lsrt_new_freelist(int count);
void lsrt_fill_freelist(struct ls_object* freelist[],
                        int i, struct ls_object* obj);

int lsrt_main_retval(const struct ls_object *lso);
int lsrt_test_expr(const struct ls_object *lso);

/* Its value will be zero if no definition found by loader. */
void lsrt_exit_hook(void) __attribute__ ((weak));

/* ------------------------------------------------------------------ */

#define lsrt_error(fmt...) do {                             \
    fprintf(stderr, "%s:%d: error: ", __FILE__, __LINE__);  \
    fprintf(stderr, fmt);                                   \
    fputc('\n', stderr);                                    \
    if (lsrt_exit_hook) lsrt_exit_hook();                   \
    exit(1);                                                \
  } while (0)

/* TODO:
 * These three functions are used by codegen.cc, hide them in future.
 */
void lsrt_check_args_count(int min, int max, int argc);
void lsrt_check_symbol_unbound(const struct ls_object *arg);
void lsrt_func_p(const struct ls_object *obj);

#define fail_on(cond, msg...) do {              \
  if ((cond)) lsrt_error(msg);                  \
} while(0)

#define lsrt_pair_p(obj) fail_on(!lso_is_pair(obj), "expected pair")
#define lsrt_number_p(obj) fail_on(!lso_is_number(obj), "expected number")
#define lsrt_vector_p(obj) fail_on(!lso_is_vector(obj), "expected vector")
#define lsrt_string_p(obj) fail_on(!lso_is_string(obj), "expected string")
#define lsrt_symbol_p(obj) fail_on(!lso_is_symbol(obj), "expected symbol")

/*
 * it doesn't do much at the moment, but in future, it can behave
 * like EXPORT_SYMBOL in linux
 */
#define BUILTIN(_r5rs_name, _builtin_suffix)          \
  struct ls_object *lsrt_builtin_ ## _builtin_suffix( \
      int, struct ls_object *[], struct ls_object*[])
#define BUILTIN_LIB(_n,_s)  BUILTIN((_n),_s)

#endif
/* vim: set et ts=2 sw=2 cin: */
