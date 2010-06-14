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

extern llvm::Module *module;
extern llvm::IRBuilder<> builder;

extern llvm::Type *LSObjType;
extern llvm::FunctionType *LSFuncType;
extern llvm::Type *LSFuncPtrType;

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
LSObjGetTypeAddr(llvm::LLVMContext &context,
                 llvm::Value *lso, int arrayIndex) {
  llvm::Constant *idx0 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), arrayIndex);
  llvm::Constant *idx1 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *offset[] = {idx0, idx1};

  return builder.CreateInBoundsGEP(lso, offset, offset + 2);
}

static inline llvm::Value *
LSObjGetPointerAddr(llvm::LLVMContext &context,
                    llvm::Value *lso, int arrayIndex, int index) {
  llvm::Constant *idx0 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), arrayIndex);
  llvm::Constant *idx1 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), index);
  llvm::Constant *idx2 =
    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *offset[] = {idx0, idx1, idx2};

  return builder.CreateInBoundsGEP(lso, offset, offset + 3);
}

static inline llvm::Value *
LSObjGetValueAddr(llvm::LLVMContext &context,
                  llvm::Value *lso, int arraryIndex, int index) {
  llvm::Value *addr = LSObjGetPointerAddr(context, lso, arraryIndex, index);

  return builder.CreateBitCast(addr,
                               llvm::Type::getInt32Ty(context)->getPointerTo());
}

#endif
