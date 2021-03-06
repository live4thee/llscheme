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

#include <llvm/Analysis/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetSelect.h>
#include <llvm/Target/TargetData.h>

#include <vector>
#include <memory>
#include <iostream>

#include "astnodes.hh"
#include "driver.hh"
#include "env.hh"
#include "error.hh"

using llvm::StructType;
using llvm::BasicBlock;

using llvm::getGlobalContext;
using llvm::ExecutionEngine;
using llvm::verifyFunction;

// qhe: I'm getting sick of it, it's too long and everywhere.
// Let's use context for now.
// Eventually, this need to be a member function or an argument
// and we will be generating to any context instead of
// hardwiring to the global context.
#define context getGlobalContext()

Module *module;
IRBuilder<> builder(context);
ExecutionEnv eenv;

const Type *LSObjType;
FunctionType *LSFuncType;
const Type *LSFuncPtrType;

static std::auto_ptr<ExecutionEngine> JIT;

static void InitializeLSTypes(void) {
  std::vector<const Type*> v;
  const Type *type;
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
  v.push_back(LSObjType->getPointerTo()->getPointerTo());
  v.push_back(LSObjType->getPointerTo()->getPointerTo());

  ftype = FunctionType::get(LSObjType->getPointerTo(), v, false);
  type = ftype->getPointerTo();
  module->addTypeName("ls_func_ptr", type);
  LSFuncType = ftype;
  LSFuncPtrType = type;
}

static void InitializeLSRTFunctions(void) {
  std::vector<const Type*> v;
  FunctionType *ftype;

  v.push_back(Type::getInt32Ty(context));
  ftype = FunctionType::get(LSObjType->getPointerTo(), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_new_object", module);

  v.clear();
  v.push_back(Type::getInt32Ty(context));
  ftype = FunctionType::get(LSObjType->getPointerTo()->getPointerTo(),
                            v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_new_freelist", module);

  v.clear();
  v.push_back(LSObjType->getPointerTo()->getPointerTo());
  v.push_back(Type::getInt32Ty(context));
  v.push_back(LSObjType->getPointerTo());
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_fill_freelist", module);

  v.clear();
  v.push_back(Type::getInt32Ty(context));
  v.push_back(Type::getInt32Ty(context));
  v.push_back(Type::getInt32Ty(context));
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_check_args_count", module);
  v.clear();
  v.push_back(LSObjType->getPointerTo());
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_func_p", module);
  v.clear();
  v.push_back(LSObjType->getPointerTo());
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_check_symbol_unbound", module);

  v.clear();
  v.push_back(LSObjType->getPointerTo());
  ftype = FunctionType::get(Type::getInt32Ty(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_main_retval", module);

  v.clear();
  v.push_back(LSObjType->getPointerTo());
  ftype = FunctionType::get(Type::getInt32Ty(context), v, false);
  Function::Create(ftype, Function::ExternalLinkage,
                   "lsrt_test_expr", module);
}

static const struct _builtin_proc {
  const std::string proc;
  const std::string builtin;
} builtin_proc[] = {
  /* Numbers */
  { "number?", "numberp" },
  { "complex?", "complexp" },
  { "real?", "realp" },
  { "rational?", "rationalp" },
  { "integer?", "integerp" },
  { "exact?", "exactp" },
  { "inexact?", "inexactp" },

  /* Arithmetic */
  { "+", "plus" },
  { "-", "minus" },
  { "*", "multiply" },
  { "/", "divide" },
  { "=", "eq"},
  { "<", "lt"},
  { ">", "gt"},
  { "<=", "le"},
  { ">=", "ge"},

  /* I/O */
  { "display", "" },

  /* Pair and List */
  { "cons", "" },
  { "car", "" },
  { "cdr", "" },
  { "set-car!", "setcar" },
  { "set-cdr!", "setcdr" },
  { "list", "" },
  { "list-ref", "listref" },
  { "length", "" },
  { "null?", "nullp" },
  { "pair?", "pairp" },
  { "list?", "listp" },

  /* vector */
  { "vector?", "vectorp" },
  { "vector-length", "vlength" },
  { "make-vector", "mkvector" },
  { "vector", "" },
  { "vector-ref", "vecref" },
  { "vector->list", "vec2lst" },

  /* String and Symbol */
  { "string?", "strp" },
  { "symbol?", "symp" },
  { "string=?", "streq" },
  { "string-ci=?", "strcaseeq" },
  { "string-length", "strlen" },
  { "substring", "" },
  { "string-append", "strappend" },
  { "symbol->string", "sym2str" },
  { "string->number", "string2number" },
};

static const int num_builtin_proc =
  sizeof(builtin_proc) / sizeof(_builtin_proc);

static void CheckBuiltinProcs(void) {
  int i;
  Value *val, *obj;
  Function *func;
  std::string fname;

  for (i = 0; i < num_builtin_proc; i++) {
    val = eenv.searchBinding(builtin_proc[i].proc);
    if (val) {
      fname = builtin_proc[i].builtin;
      if (fname == "")
        fname = builtin_proc[i].proc;

      func = module->getFunction("lsrt_builtin_" + fname);
      if (func)
        continue;

      func = Function::Create(LSFuncType, Function::ExternalLinkage,
                              "lsrt_builtin_" + fname, module);
      // obj(sym).u1.ptr -> obj(func).u1.ptr -> func_ptr
      // obj(sym).u2.ptr -> symbol
      obj = LSObjFunctionInit(context, func, fname);

      builder.CreateStore(builder.CreateBitCast(obj,
                              Type::getInt8Ty(context)->getPointerTo()),
                          LSObjGetPointerAddr(context, val, 0, 1));
    }
  }
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

  ftype = FunctionType::get(Type::getVoidTy(context), v, false);
  prolog = Function::Create(ftype, Function::InternalLinkage,
                            "_prolog", module);

  v.push_back(Type::getInt32Ty(context));
  v.push_back(Type::getInt8Ty(context)->getPointerTo()->getPointerTo());
  ftype = FunctionType::get(Type::getInt32Ty(context), v, false);

  entry = Function::Create(ftype, Function::ExternalLinkage,
                           "main", module);
  bb = BasicBlock::Create(context, "entry", entry);
  builder.SetInsertPoint(bb);

  builder.CreateCall(prolog, args.begin(), args.end(), "");

  /* Init llscheme memory allocator */
  ftype = FunctionType::get(Type::getVoidTy(context), false);
  Function* gc_init_func = Function::Create(ftype, Function::ExternalLinkage,
      "lsrt_memory_init", module);
  builder.CreateCall(gc_init_func, "");
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

  CheckBuiltinProcs();
  builder.CreateRetVoid();
}

static void codegenInitJIT(Module* mod) {
  std::string errmsg;
  ExecutionEngine *ee;

  // If not set, exception handling will not be turned on
  llvm::DwarfExceptionHandling = true;

  llvm::InitializeNativeTarget();

  ee = llvm::EngineBuilder(mod).setErrorStr(&errmsg).create();
  if (!ee) {
    std::cerr << "warning: " << errmsg << std::endl;
    return;
  }

  mod->setDataLayout(ee->getTargetData()->getStringRepresentation());
  JIT.reset(ee);
}

/* mater driver function called by main() */
int codegen(ASTNode *ast, int fd) {
  module = new Module("lls", getGlobalContext());

  codegenInit();
  codegenInitJIT(module);
  codegenFinish(ast->codeGenEval());

  llvm::raw_fd_ostream fdos(fd, false);
  module->print(fdos, NULL);

  return 0;
}

// TODO: restructuring compiler and interpreter
static inline void JITRunFunc(Function* func)
{
  void *f = JIT.get()->getPointerToFunction(func);
  void (*fptr)() = (void (*)()) (intptr_t) f;
  fptr();
}

static void InterpreterProlog(void) {
  std::vector<const Type*> v;
  FunctionType *ftype;
  Function *func;
  BasicBlock *bb;

  InitializeLSTypes();
  InitializeLSRTFunctions();

  v.push_back(Type::getVoidTy(context));
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);

  func = Function::Create(ftype, Function::PrivateLinkage,
                           "", module);
  bb = BasicBlock::Create(context, "", func);
  builder.SetInsertPoint(bb);

  /* Init llscheme memory allocator */
  ftype = FunctionType::get(Type::getVoidTy(context), false);
  Function* gc_init_func = Function::Create(ftype, Function::ExternalLinkage,
      "lsrt_memory_init", module);
  builder.CreateCall(gc_init_func, "");
  builder.CreateRetVoid();

  JITRunFunc(func);
}

int InterpreterInit() {
  module = new Module("lls", getGlobalContext());

  codegenInitJIT(module);
  InterpreterProlog();
}

int InterpreterRun(ASTNode *ast) {
  std::vector<const Type*> v;
  FunctionType *ftype;
  Function *func;
  BasicBlock *funcbb, *bb;

  v.push_back(Type::getVoidTy(context));
  ftype = FunctionType::get(Type::getVoidTy(context), v, false);

  func = Function::Create(ftype, Function::PrivateLinkage,
                           "", module);
  funcbb = BasicBlock::Create(context, "", func);
  builder.SetInsertPoint(funcbb);
  ast->codeGenEval();
  builder.CreateRetVoid();

  bb = BasicBlock::Create(context, "");
  func->getBasicBlockList().push_front(bb);
  builder.SetInsertPoint(bb);

  CheckBuiltinProcs();
  builder.CreateBr(funcbb);

  JITRunFunc(func);
}

/* Throw an exception and return the control to the caller,
 * so that lsrt_exit() will not terminate the interpreter.
 */
extern "C" void lsrt_exit_hook()
{
  throw Error("");
}

/* vim: set et ts=2 sw=2 cin: */
