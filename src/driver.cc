// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
//
// Copyright (C)
//         2010 Qing He <qing.x.he@gmail.com>
//         2010 David Lee <live4thee@gmail.com>
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

#include "ast2.hh"
#include "driver.hh"

#include <llvm/Analysis/Verifier.h>
#include <vector>
#include <iostream>

using llvm::Type;
using llvm::Value;
using llvm::Function;
using llvm::FunctionType;
using llvm::StructType;
using llvm::BasicBlock;

using llvm::Module;
using llvm::IRBuilder;
using llvm::getGlobalContext;
using llvm::verifyFunction;

// qhe: I'm getting sick of it, it's too long and everywhere.
// Let's use context for now.
// Eventually, this need to be a member function or an argument
// and we will be generating to any context instead of
// hardwiring to the global context.
#define context getGlobalContext()

Module *module;
IRBuilder<> builder(context);

Type *LSObjType;
FunctionType *LSFuncType;
Type *LSFuncPtrType;

static void InitializeLSTypes(void) {
  std::vector<const Type*> v;
  Type *type;
  FunctionType *ftype;

  /* generate ls_object type */
  v.push_back(Type::getInt8Ty(context)->getPointerTo());
  type = StructType::get(context, v);

  v.clear();
  v.push_back(Type::getInt32Ty(context));
  v.push_back(type);
  v.push_back(type);
  type = StructType::get(context, v);
  module->addTypeName("ls_object", type);
  LSObjType = type;

  v.clear();
  v.push_back(Type::getInt32Ty(context));
  v.push_back(LSObjType->getPointerTo());

  ftype = FunctionType::get(LSObjType->getPointerTo(), v, false);
  type = ftype->getPointerTo();
  module->addTypeName("ls_func_ptr", type);
  LSFuncType = ftype;
  LSFuncPtrType = type;
}

static void InitializeLSRTFunctions(void) {
  std::vector<const Type*> v;
  FunctionType *ftype;

  v.push_back(Type::getInt8Ty(context)->getPointerTo());
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_error", module);

  v.clear();
  v.push_back(Type::getInt32Ty(context));
  ftype = FunctionType::get(LSObjType->getPointerTo(), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_new_object", module);

  v.clear();
  v.push_back(Type::getInt32Ty(context));
  v.push_back(Type::getInt32Ty(context));
  v.push_back(Type::getInt32Ty(context));
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_check_args_count", module);
  v.clear();
  v.push_back(LSObjType->getPointerTo());
  v.push_back(Type::getInt32Ty(context));
  v.push_back(Type::getInt8Ty(context));
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_check_arg_type", module);

  v.clear();
  v.push_back(LSObjType->getPointerTo());
  ftype = FunctionType::get(Type::getInt32Ty(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_main_retval", module);
}

// Several things we need to do before generating real code
static void codegenInit(void) {
  std::vector<const Type*> v;
  std::vector<Value*> args;
  FunctionType *ftype;
  Function *entry, *prolog;
  BasicBlock *bb;

  InitializeLSTypes();
  InitializeLSRTFunctions();

  v.clear();
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  prolog = Function::Create(ftype, Function::InternalLinkage,
                            "_prolog", module);

  v.clear();
  v.push_back(Type::getInt32Ty(context));
  v.push_back(Type::getInt8Ty(context)->getPointerTo()->getPointerTo());
  ftype = FunctionType::get(Type::getInt32Ty(context), v, false);

  entry = Function::Create(ftype, Function::ExternalLinkage,
                           "main", module);
  bb = BasicBlock::Create(context, "entry", entry);
  builder.SetInsertPoint(bb);

  builder.CreateCall(prolog, args.begin(), args.end(), "");
}

static void codegenFinish(Value *value) {
  BasicBlock *bb;
  Function *f;
  Value *ret;

  f = module->getFunction("lsrt_main_retval");
  ret = builder.CreateCall(f, value);
  builder.CreateRet(ret);

  f = module->getFunction("_prolog");
  bb = BasicBlock::Create(context, "entry", f);
  builder.SetInsertPoint(bb);

  builder.CreateRetVoid();
}

/* mater driver function called by main() */
int codegen(ASTNode *ast) {
  module = new Module("lls", getGlobalContext());

  codegenInit();
  codegenFinish(ast->codeGen());

  module->print(std::cout, NULL);

  return 0;
}
