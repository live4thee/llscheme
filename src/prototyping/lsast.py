##
## Copyright (C) 2010 David Lee <live4thee@gmail.com>
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
Scheme abstract syntax node definitions.
"""

def flatten(seq):
    l = []
    for elt in seq:
        t = type(elt)
        if t is tuple or t is list:
            for elt2 in flatten(elt):
                l.append(elt2)
        else:
            l.append(elt)
    return l

def flatten_nodes(seq):
    return [n for n in flatten(seq) if isinstance(n, AST)]

class AST:
    """Abstract base class for ast nodes."""
    def getChildren(self):
        pass # implemented by subclasses
    def __iter__(self):
        for n in self.getChildren():
            yield n
    def asList(self): # for backwards compatibility
        return self.getChildren()
    def getChildNodes(self):
        pass # implemented by subclasses

class Const(AST):
    def __init__(self, value, lineno=None):
        self.value = value
        self.lineno = lineno

    def getChildren(self):
        return self.value

    def getChildNodes(self):
        return ()

    def __repr__(self):
        return "Const(%s)" % (repr(self.value),)

# (+)       => 0
# (+ 1)     => 1
# (+ 1 2)   => 3
# (+ 1 2 3) => 6
class Add(AST):
    def __init__(self, args, lineno=None):
        self.args = args
        self.lineno = lineno

    def getChildren(self):
        return tuple(self.args)

    def getChildNodes(self):
        return tuple(self.args)

    def __repr__(self):
        return "Add(%s)" % (repr(self.args))

class CallFunc(AST):
    def __init__(self, node, args, lineno=None):
        self.node = node
        self.args = args
        self.lineno = lineno

    def getChildren(self):
        children = []
        children.append(self.node)
        children.append(self.args)
        return tuple(children)

    def getChildNodes(self):
        nodelist = []
        nodelist.append(self.node)
        nodelist.extend(flatten_nodes(self.args))
        return tuple(nodelist)

    def __repr__(self):
        return "CallFunc(%s, %s)" % (repr(self.node), repr(self.args))


# vim: set et ts=4 sw=4 ai:
