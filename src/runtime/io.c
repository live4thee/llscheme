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

#include "utils.h"

/**********************************************************
 * Section 6.6 Input and Output
 **********************************************************/

/**********************************************************
 * Section 6.6.3 Output
 **********************************************************/

char *_ntos(const struct ls_object *number);

static void _display(const struct ls_object *lso, int fp)
{
  int type = lso_type(lso);
  switch (type) {
  case ls_t_number:
    if (lso_is_simplenumber(lso))
      printf("%d", lso_simplenumber_get(lso));
    else
      printf("%s", _ntos(lso));
    break;
  case ls_t_boolean:
    if (lso_boolean_get(lso))
      printf("#t");
    else
      printf("#f");
    break;
  case ls_t_symbol:
    if (lso_symbol_name(lso))
      printf("%s", lso_symbol_name(lso));
    else
      printf("#<invalid symbol>");
    break;
  case ls_t_string:
    printf("\"%s\"", lso_string_get(lso));
    break;
  case ls_t_pair:
    if (!fp)
      printf("(");

    _display(lso_pair_car(lso), 0);
    if (!lso_is_nil(lso_pair_cdr(lso))) {
      if (lso_is_pair(lso_pair_cdr(lso)))
        printf(" ");
      else
        printf(" . ");
      _display(lso_pair_cdr(lso), 1);
    }

    if (!fp)
      printf(")");
    break;
  case ls_t_nil:
    printf("()");
    break;
  case ls_t_func:
    printf("#<procedure %p>", lso);
    break;
  case ls_t_vector:
    do {
      struct ls_object* const* addr0 = lso_vector_addr0(lso);
      int32_t idx, len = lso_vector_length(lso);
      printf("#(");
      for (idx = 0; idx < len - 1; ++idx) {
        _display(addr0[idx], 0);
        putchar(' ');
      }
      if (len > 0) _display(addr0[idx], 0);
      putchar(')');
    } while (0);
    break;
  case ls_t_unspec:
    printf("#<unspecified>");
    break;
  default:
    lsrt_error("unknown type: %d", type);
  }
}

BUILTIN_LIB("display", display);
struct ls_object *lsrt_builtin_display(int argc, struct ls_object *args[],
                                       struct ls_object *freelist[])
{
  UNUSED_ARGUMENT(freelist);
  lsrt_check_args_count(1, 1, argc);
  _display(args[0], 0);
  printf("\n");

  return &global_unspec_obj;
}

/* vim: set et ts=2 sw=2 cin: */
