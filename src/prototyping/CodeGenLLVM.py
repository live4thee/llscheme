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
            return self.builder.ret(self.visit(Const(0)))
        elif len(args) == 1:
            return self.builder.ret(args[0])

        idx = 0
        acc = args[idx]
        while idx < len(args) - 1:
            tmp = args[idx + 1]
            idx += 1
            symbol = self.symmaker.genSymbol()
            addInst = self.builder.add(acc, tmp, symbol)
            acc = addInst

        print "; [AddOp] inst = ", addInst

        self.builder.ret(addInst)
        print self.module

        return addInst


    def visitSub(self, node):

        lLLInst = self.visit(node.left)
        rLLInst = self.visit(node.right)

        subInst = self.builder.sub(lLLInst, rLLInst, tmpSym.name)
        print "; [SubOp] inst = ", subInst

        return subInst


    def visitMul(self, node):

        lLLInst = self.visit(node.left)
        rLLInst = self.visit(node.right)

        mulInst = self.builder.mul(lLLInst, rLLInst, tmpSym.name)
        print "; [MulOp] inst = ", mulInst

        return mulInst


    def visitDiv(self, node):

        lLLInst = self.visit(node.left)
        rLLInst = self.visit(node.right)

        divInst = self.builder.fdiv(lLLInst, rLLInst, tmpSym.name)
        print "; [DIvOp] inst = ", divInst

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
