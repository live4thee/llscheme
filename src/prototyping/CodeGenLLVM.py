#!/usr/bin/env python

import llvm.core

llVoidType     = llvm.core.Type.void()
llIntType      = llvm.core.Type.int()

def toLLVMTy(ty):

    if ty is None:
        return llVoidType

    d = {
        int   : llIntType
        }

    if d.has_key(ty):
        return d[ty]

    raise Exception("Unknown type:", ty)

class SymbolGenerator:
    def __init__(self):
        self.counter = 0
        self.prefix  = 'tmp.'

    def genSymbol(self):
        s = self.prefix + str(self.counter)
        self.counter += 1
        return s

class CodeGenLLVM:
    """
    LLVM CodeGen class
    """

    def __init__(self):
        self.module           = llvm.core.Module.new("module")
        tyfunc                = llvm.core.Type.function(llIntType, [])
        func                  = self.module.add_function(tyfunc, "main")
        entry                 = func.append_basic_block("entry")
        self.builder          = llvm.core.Builder.new(entry)
        self.symmaker         = SymbolGenerator()


    def visitAdd(self, node):

        args = [self.visit(a) for a in node.args]

        if len(args) == 0:
            return self.visit(Const(0))
        elif len(args) == 1:
            return self.visit(args[0])

        idx, acc = 1, args[0]
        while idx < len(args):
            symbol = self.symmaker.genSymbol()
            addInst = self.builder.add(acc, args[idx], symbol)
            idx, acc = idx + 1, addInst

        print "; [AddOp] inst = ", addInst

        return addInst


    def visitSub(self, node):

        args = [self.visit(a) for a in node.args]
        if len(args) == 0:
            raise SyntaxError, "missing operands"
        elif len(args) == 1:
            return self.builder.sub(self.visit(Const(0)), args[0])

        idx, acc = 1, args[0]
        while idx < len(args):
          symbol = self.symmaker.genSymbol()
          subInst = self.builder.sub(acc, args[idx], symbol)
          idx , acc = idx + 1, subInst

        print "; [SubOp] inst = ", subInst
        self.builder.ret(subInst)
        print self.module

        return subInst


    def visitMul(self, node):

        args = [self.visit(a) for a in node.args]

        if len(args) == 0:
            return self.visit(Const(1))
        elif len(args) == 1:
            return self.visit(args[0])

        idx, acc = 1, args[0]
        while idx < len(args):
            symbol = self.symmaker.genSymbol()
            mulInst = self.builder.mul(acc, args[idx], symbol)
            idx, acc = idx + 1, mulInst

        print "; [MulOp] inst = ", mulInst

        return mulInst


    def visitDiv(self, node):

        args = [self.visit(a) for a in node.args]

        if len(args) == 0:
            raise SyntaxError, "missing operands"
        elif len(args) == 1:
            return self.builder.sub(self.visit(Const(1), args[0]))

        idx, acc = 1, args[0]
        while idx < len(args):
            symbol = self.symmaker.genSymbol()
            divInst = self.builder.sdiv(acc, args[idx], symbol)
            idx, acc = idx + 1, divInst

        print "; [DivOp] inst = ", divInst

        return divInst


    def mkLLConstInst(self, ty, value):

        llTy   = toLLVMTy(ty)

        bname = 'bsym';
        tname = 'tsym';
        allocInst = self.builder.alloca(llTy, bname)

        llConst = llvm.core.Constant.int(llIntType, value)

        storeInst = self.builder.store(llConst, allocInst)
        loadInst  = self.builder.load(allocInst, tname)

        print ";", loadInst

        return loadInst


    def visitConst(self, node):

        ty = type(node.value)

        return self.mkLLConstInst(ty, node.value)

# vim: set et ts=4 sw=4 ai:
