AC_PREREQ(2.60)
AC_INIT([llscheme],[0.2.0],[http://github.com/live4thee/llscheme],[llscheme])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([dist-bzip2 foreign])
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_LIBTOOL([dlopen])

# check for clang and llvm tools
AC_CHECK_PROG(LLVM_AS, llvm-as, llvm-as)
AC_CHECK_PROG(LLVM_LD, llvm-ld, llvm-ld)

# get for llvm library and include location
AC_PATH_PROG(LLVM_CONFIG, llvm-config, no)
if test "x${LLVM_CONFIG}" = "xno"; then
  AC_MSG_ERROR([llvm-config not found.])
fi

LLVM_CPPFLAGS=`${LLVM_CONFIG} --cppflags`
LLVM_LDFLAGS=`${LLVM_CONFIG} --ldflags`
LLVM_LIBS=`${LLVM_CONFIG} --libs core jit native`
LLVM_LIBS="${LLVM_LDFLAGS} ${LLVM_LIBS}"

AC_SUBST(LLVM_CPPFLAGS)
AC_SUBST(LLVM_LIBS)

# check for bdw-gc
# http://www.hpl.hp.com/personal/Hans_Boehm/gc/
have_gc=no
AC_SUBST([TARGET_LIBS])
LSC_SEARCH_TARGET_LIBS([GC_malloc_atomic], [gc], [have_gc=yes])

if test "x${have_gc}" = xyes; then
  AC_CHECK_HEADERS([gc/gc.h], [], [have_gc=no])
fi

if test "x${have_gc}" = xno; then
  AC_MSG_FAILURE([
-------------------------------------------------------------------
 The bdw-gc library and header file is required to build llscheme.
 Stopping... Check 'config.log' for more information.
-------------------------------------------------------------------
])
fi

# check for gmp-dev
AC_CHECK_HEADERS([gmp.h], [have_gmp=yes], [have_gmp=no])

if test "x${have_gmp}" = xno; then
  AC_MSG_FAILURE([
-------------------------------------------------------------------
 The gmp-dev library and header file is required to build llscheme.
 Stopping... Check 'config.log' for more information.
-------------------------------------------------------------------
])
fi

# write output
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/runtime/Makefile
])
AC_CONFIG_FILES([
  src/llscheme
], [chmod +x src/llscheme])
AC_OUTPUT

AC_MSG_NOTICE([

llscheme is now configured

])

