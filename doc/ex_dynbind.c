/*
 * Copyright (C) 2010 Qing He <qing.x.he@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby
 * granted, provided that the above copyright notice and
 * this permission notice appear in all copies.
 *
 */

/*
 * handcrafted compilation of scheme code with dynamic bindings
 * usage: ./ex_dynbind; echo $?
 *
 * the scheme code used is as below:
 *   (define (a x) (plus x 1 2 3 4 5))
 *   (define plus +)
 *   (a 10)
 */

#include <malloc.h>

#define ls_t_number 0
#define ls_t_symbol 1
#define ls_t_value  2
#define ls_t_func   3

struct ls_object
{
    int type;
    union {
        int val;
        void *ptr;
    } u;
};

#define TO_OBJ(x)  ((struct ls_object *) (x))
#define TO_FUNC(x) ((struct ls_object *(*)(int, struct ls_object *)) (x))

struct ls_object _builtin_sym_plus;
struct ls_object _builtin_func_plus;

struct ls_object _sym_a;
struct ls_object _sym_plus;
struct ls_object _obj_func_1;


struct ls_object *
_primitive_plus(int argc, struct ls_object *args)
{
    struct ls_object *ret = malloc(sizeof *ret);

    // assert (argc != 0)

    ret->type = ls_t_number;
    ret->u.val = 0;

    while (argc--) {
        // assert (args[argc].type == number)
        ret->u.val += args[argc].u.val;
    }

    return ret;
}

// equivlent: (lambda (x) (plus x 1 2 3 4 5))
struct ls_object *
_func_1(int argc, struct ls_object *args)
{
    struct ls_object a[6];

    // assert (argc == 1)

    a[0] = args[0];
    a[1] = (struct ls_object) { ls_t_number, { 1 } };
    a[2] = (struct ls_object) { ls_t_number, { 2 } };
    a[3] = (struct ls_object) { ls_t_number, { 3 } };
    a[4] = (struct ls_object) { ls_t_number, { 4 } };
    a[5] = (struct ls_object) { ls_t_number, { 5 } };

    // assert (TO_OBJ(x).type == ls_t_symbol)
    // assert (TO_FUNC(x).type == ls_t_func)
    return TO_FUNC(TO_OBJ(_sym_plus.u.ptr)->u.ptr) (6, a);
}

void init()
{
    // set up `+'
    _builtin_func_plus.type = ls_t_func;
    _builtin_func_plus.u.ptr = (void *) &_primitive_plus;
    _builtin_sym_plus.type = ls_t_symbol;
    _builtin_sym_plus.u.ptr = (void *) &_builtin_func_plus;
}

int main()
{
    struct ls_object *ret;

    // prelogue
    init();

    // ; (define (a x) (plus x 1 2 3 4 5))
    _obj_func_1.type = ls_t_func;
    _obj_func_1.u.ptr = (void *) _func_1;
    _sym_a.type = ls_t_symbol;
    _sym_a.u.ptr = (void *) &_obj_func_1;

    // ; (define plus +)
    _sym_plus = _builtin_sym_plus;

    // ; (a 10)
    // assert (TO_OBJ(x).type == ls_t_symbol)
    // assert (TO_FUNC(x).type == ls_t_func)
    ret = TO_FUNC(TO_OBJ(_sym_a.u.ptr)->u.ptr)
        (1, &((struct ls_object) { ls_t_number, { 10 } }));

    // epilogue
    return ret->u.val;
}
