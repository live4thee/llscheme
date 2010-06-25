
AC_DEFUN([LSC_PROG_CC_FOR_TARGET], [])

AC_DEFUN([LSC_SEARCH_TARGET_LIBS],
         [saved_LIBS=$LIBS
          LIBS=$TARGET_LIBS
dnl need to use cc_for_target
          AC_SEARCH_LIBS($@)
          TARGET_LIBS=$LIBS
          LIBS=$saved_LIBS])
