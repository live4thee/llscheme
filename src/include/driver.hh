// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
//
// Copyright (C)
//         2010 David Lee <live4thee@gmail.com>
//         2010 Qing He <qing.x.he@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef DRIVER_HH_
#define DRIVER_HH_

#include <llvm/Type.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>

#include "astnodes.hh"
#include "env.hh"
#include "runtime/object.h"

using llvm::Type;
using llvm::Value;
using llvm::Constant;
using llvm::ConstantInt;
using llvm::ConstantExpr;
using llvm::ConstantStruct;
using llvm::Function;
using llvm::FunctionType;
using llvm::GlobalVariable;
using llvm::LLVMContext;
using llvm::Module;
using llvm::IRBuilder;

extern Module *module;
extern IRBuilder<> builder;
extern ExecutionEnv eenv;

extern const Type *LSObjType;
extern FunctionType *LSFuncType;
extern const Type *LSFuncPtrType;

extern int codegen(ASTNode *);

// Create a ls_object of TYPE defined in runtime/object.h Return the
// pointer to the allocated memory.  Since the ls_object is an
// aggregate data structure, we need GEP (getelementptr) instruction
// to access its content.

static inline Value *
LSObjNew(LLVMContext &context,
         int type) {
  Value *t;
  Function *f;

  t = ConstantInt::get(Type::getInt32Ty(context), type);
  f = module->getFunction("lsrt_new_object");
  return builder.CreateCall(f, t);
}

static inline Value *
GEP1(LLVMContext &context,
     Value *val, int i0) {
  Constant *idx0 =
    ConstantInt::get(Type::getInt32Ty(context), i0);
  Value *offset[] = {idx0};

  return builder.CreateInBoundsGEP(val, offset, offset + 1);
}

static inline Value *
GEP2(LLVMContext &context,
     Value *val, int i0, int i1) {
  Constant *idx0 =
    ConstantInt::get(Type::getInt32Ty(context), i0);
  Constant *idx1 =
    ConstantInt::get(Type::getInt32Ty(context), i1);
  Value *offset[] = {idx0, idx1};

  return builder.CreateInBoundsGEP(val, offset, offset + 2);
}

static inline Value *
LSObjGetTypeAddr(LLVMContext &context,
                 Value *lso, int arrayIndex) {
  return GEP2(context, lso, arrayIndex, 0);
}

static inline Value *
GEP3(LLVMContext &context,
     Value *val, int i0, int i1, int i2) {
  Constant *idx0 =
    ConstantInt::get(Type::getInt32Ty(context), i0);
  Constant *idx1 =
    ConstantInt::get(Type::getInt32Ty(context), i1);
  Constant *idx2 =
    ConstantInt::get(Type::getInt32Ty(context), i2);
  Value *offset[] = {idx0, idx1, idx2};

  return builder.CreateInBoundsGEP(val, offset, offset + 3);
}

static inline Value *
LSObjGetPointerAddr(LLVMContext &context,
                    Value *lso, int arrayIndex, int index) {
  return GEP3(context, lso, arrayIndex, index, 0);
}

static inline Value *
LSObjGetValueAddr(LLVMContext &context,
                  Value *lso, int arraryIndex, int index) {
  Value *addr = LSObjGetPointerAddr(context, lso, arraryIndex, index);

  return builder.CreateBitCast(addr,
                               Type::getInt32Ty(context)->getPointerTo());
}

// Create ls_t_func object and its initializer
static inline Value *
LSObjFunctionInit(LLVMContext &context,
                  Function *func, const std::string &name) {
  GlobalVariable *g;
  Constant *init;
  std::vector<Constant *> v, m, idx;

  v.push_back(ConstantInt::get(Type::getInt32Ty(context), ls_t_func));
  m.push_back(ConstantExpr::getBitCast(func,
                  Type::getInt8Ty(context)->getPointerTo()));

  v.push_back(ConstantStruct::get(context, m, false));
  m.clear();

  idx.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
  idx.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));

  m.push_back(Constant::getNullValue(Type::getInt8Ty(context)->getPointerTo()));
  v.push_back(ConstantStruct::get(context, m, false));
  init = ConstantStruct::get(context, v, false);

  g = new GlobalVariable(*module, LSObjType, false,
                llvm::GlobalValue::PrivateLinkage,
                init, "_funcobj_" + name);

  return g;
}
#endif

/* vim: set et ts=2 sw=2 cin: */
