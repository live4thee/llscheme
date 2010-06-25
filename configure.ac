AC_PREREQ(2.60)
AC_INIT([llscheme],[0.1.0],[http://github.com/live4thee/llscheme],[llscheme])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([dist-bzip2 foreign])
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_LIBTOOL

# check for clang and llvm tools
AC_CHECK_PROG(LLVMAS, llvm-as, llvm-as)
AC_CHECK_PROG(LLVMLD, llvm-ld, llvm-ld)

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
AC_SEARCH_LIBS([GC_malloc_atomic], [gc], [have_gc=yes])

if test "x${have_gc}" = xyes; then
  AC_CHECK_HEADERS([gc/gc.h], [], [have_gc=no])
fi

if test "x${have_gc}" = xno; then
  echo "-------------------------------------------------------------------"
  echo " The bdw-gc library and header file is required to build llscheme. "
  echo " Stopping... Check 'config.log' for more information.              "
  echo "-------------------------------------------------------------------"
  (exit 1); exit 1;
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

echo
echo "llscheme is now configured"
echo
