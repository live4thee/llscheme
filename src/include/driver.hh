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

#include "ast2.hh"
#include "env.hh"
#include "runtime/object.h"

extern llvm::Module *module;
extern llvm::IRBuilder<> builder;
extern ExecutionEnv eenv;

extern const llvm::Type *LSObjType;
extern llvm::FunctionType *LSFuncType;
extern const llvm::Type *LSFuncPtrType;

extern int codegen(ASTNode *);

static inline llvm::Value *
LSObjNew(llvm::LLVMContext &context,
         int type) {
  llvm::Value *t;
  llvm::Function *f;

  t = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), type);
  f = module->getFunction("lsrt_new_object");
  return builder.CreateCall(f, t);
}

static inline llvm::Value *
GEP1(llvm::LLVMContext &context,
     llvm::Value *val, int i0) {
  llvm::Constant *idx0 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i0);
  llvm::Value *offset[] = {idx0};

  return builder.CreateInBoundsGEP(val, offset, offset + 1);
}

static inline llvm::Value *
GEP2(llvm::LLVMContext &context,
     llvm::Value *val, int i0, int i1) {
  llvm::Constant *idx0 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i0);
  llvm::Constant *idx1 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i1);
  llvm::Value *offset[] = {idx0, idx1};

  return builder.CreateInBoundsGEP(val, offset, offset + 2);
}

static inline llvm::Value *
LSObjGetTypeAddr(llvm::LLVMContext &context,
                 llvm::Value *lso, int arrayIndex) {
  return GEP2(context, lso, arrayIndex, 0);
}

static inline llvm::Value *
GEP3(llvm::LLVMContext &context,
     llvm::Value *val, int i0, int i1, int i2) {
  llvm::Constant *idx0 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i0);
  llvm::Constant *idx1 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i1);
  llvm::Constant *idx2 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i2);
  llvm::Value *offset[] = {idx0, idx1, idx2};

  return builder.CreateInBoundsGEP(val, offset, offset + 3);
}

static inline llvm::Value *
LSObjGetPointerAddr(llvm::LLVMContext &context,
                    llvm::Value *lso, int arrayIndex, int index) {
  return GEP3(context, lso, arrayIndex, index, 0);
}

static inline llvm::Value *
LSObjGetValueAddr(llvm::LLVMContext &context,
                  llvm::Value *lso, int arraryIndex, int index) {
  llvm::Value *addr = LSObjGetPointerAddr(context, lso, arraryIndex, index);

  return builder.CreateBitCast(addr,
                               llvm::Type::getInt32Ty(context)->getPointerTo());
}

// Create ls_t_func object and its initializer
static inline llvm::Value *
LSObjFunctionInit(llvm::LLVMContext &context,
                  llvm::Function *func, const std::string &name) {
  llvm::GlobalVariable *g;
  llvm::Constant *init;
  std::vector<llvm::Constant *> v, m, idx;

  v.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), ls_t_func));

  m.clear();
  m.push_back(llvm::ConstantExpr::getBitCast(func,
                  llvm::Type::getInt8Ty(context)->getPointerTo()));
  v.push_back(llvm::ConstantStruct::get(context, m, false));

  m.clear();
  idx.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
  idx.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));

  m.push_back(llvm::Constant::getNullValue(llvm::Type::getInt8Ty(context)->getPointerTo()));
  v.push_back(llvm::ConstantStruct::get(context, m, false));
  init = llvm::ConstantStruct::get(context, v, false);

  g = new llvm::GlobalVariable(*module, LSObjType, false,
                llvm::GlobalValue::PrivateLinkage,
                init, "_funcobj_" + name);

  return g;
}
#endif
