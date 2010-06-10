##
## Copyright (C) 2010 Li Qun <liqun82@users.sf.net>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##
##
"""
A trivial Scheme to LLVM compiler.
"""

import lsast
import walker
import CodeGenLLVM

def main():
    a1 = lsast.Add([lsast.Const(1), lsast.Const(2), lsast.Const(3)])
    a2 = lsast.Sub([lsast.Const(7), a1])

    visitor = CodeGenLLVM.CodeGenLLVM()
    walker.walk(a2, visitor, None, 1)


if __name__ == '__main__':
    main()

# vim: set et ts=4 sw=4 ai:
