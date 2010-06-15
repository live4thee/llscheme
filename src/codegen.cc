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
#include "error.hh"
#include "env.hh"

#include <llvm/Analysis/Verifier.h>
#include <vector>

using llvm::Type;
using llvm::Constant;
using llvm::ConstantInt;
using llvm::Value;
using llvm::Function;
using llvm::FunctionType;
using llvm::StructType;
using llvm::BasicBlock;

using llvm::Module;
using llvm::IRBuilder;
using llvm::getGlobalContext;
using llvm::verifyFunction;

#define context getGlobalContext()

Value *NumberASTNode::codeGen() {
  Value *obj, *addr;

  obj = LSObjNew(context, ls_t_number);
  addr = LSObjGetValueAddr(context, obj, 0, 1);
  builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), val), addr);

  return obj;
}

Value *BooleanASTNode::codeGen() {
  return NULL;
}

Value *SymbolASTNode::codeGen() {
  Constant *str, *s, *init;
  Value *g;
  std::vector<Constant *> v, m, idx;

  // N.B. local variables must be added manually, unknown symbols
  // are treated as global.
  g = eenv.searchBinding(symbol);
  if (g)
    return g;

  // creating symbol objects and their initializers
  // NB. ConstantExpr::getInBoundsGetElementPtr is needed to
  //     change const string literals [ n x i8 ]* into i8*
  str = llvm::ConstantArray::get(context, symbol);
  s = new llvm::GlobalVariable(*module, str->getType(), true,
                llvm::GlobalValue::PrivateLinkage, str, "_str_s_" + symbol);

  v.push_back(ConstantInt::get(Type::getInt32Ty(context), ls_t_symbol));
  m.push_back(Constant::getNullValue(Type::getInt8Ty(context)->getPointerTo()));
  v.push_back(llvm::ConstantStruct::get(context, m, false));

  m.clear();
  idx.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
  idx.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));

  m.push_back(llvm::ConstantExpr::getInBoundsGetElementPtr(s, &idx[0], idx.size()));
  v.push_back(llvm::ConstantStruct::get(context, m, false));
  init = llvm::ConstantStruct::get(context, v, false);

  g = new llvm::GlobalVariable(*module, LSObjType, false,
                llvm::GlobalValue::PrivateLinkage,
                init, "_sym_" + symbol);

  eenv.addGlobalBinding(symbol, g);

  return g;
}

Value *SymbolASTNode::codeGenEval() {
  Value *s = codeGen();
  Value *addr, *val;

  // TODO: does local variables needs another redirection like globals?
  if (eenv.searchCurrentScopeBinding(symbol))
      return s;

  builder.CreateCall(module->getFunction("lsrt_check_symbol_unbound"), s);
  addr = LSObjGetPointerAddr(context, s, 0, 1);
  val = builder.CreateBitCast(builder.CreateLoad(addr),
                              LSObjType->getPointerTo());

  return val;
}

Value *StringASTNode::codeGen() {
  return NULL;
}

// N.B. syntax can be implemented using handlers, but not
// primitive procs, like `+'. Although a handler would work
// for most cases, there is one substantial situation that
// doesn't:
//    (define plus +)
// note the first class status of even primitive procs
//
// This also implies that function argument check can't be
// done at compile time but only is possible at runtime!
// Still considering `+', it's vararg:
//    (plus 1 2 3 4 5 6)
// At compile time, we simply don't know how many arguments
// it can take.
//
// The above conclusion is based on the fact that binding of
// names is of dynamic nature. Considering:
//    (define k +)
//    (if (= n 3)
//      (set! k *) (set! k -))
//    (k 3 1)
// This means name bindings and lookups need to happen at
// runtime, that's what first-class means.
typedef Value *syntaxHandler(SExprASTNode *);
static syntaxHandler handleBegin;
static syntaxHandler handleDefine;
static syntaxHandler handleLambda;
static syntaxHandler handleQuote;

static const struct _builtin_syntax {
  const std::string key;
  syntaxHandler *handler;
} builtin_syntax[] = {
  { "begin", &handleBegin },
  { "define", &handleDefine },
  { "lambda", &handleLambda },
  { "quote", &handleQuote },
};
static const int num_builtin_syntax =
  sizeof(builtin_syntax) / sizeof(_builtin_syntax);

Value *SExprASTNode::codeGen() {
  return NULL;
}

Value *SExprASTNode::codeGenEval() {
  int i;
  Value *func = NULL, *addr, *val;
  Constant *size;

  // if arg[0] is symbol:
  //   syntaxhandler(this) if its keyword
  //   codegen: eval arg[0] otherwise
  // elif arg[0] is s-expr:
  //   codegen: eval arg[0]
  // else
  //   error
  // codegen: runtime error situation if arg[0] isn't resolved to function
  // codegen: eval the remaining args
  // codegen: call function with arg count and vectors of args
  // TODO: ( args . rest)

  if (exp[0]->getType() == SymbolAST) {
    SymbolASTNode *node = static_cast<SymbolASTNode *>(exp[0]);
    // syntax
    for (i = 0; i < num_builtin_syntax; i++) {
      if (node->symbol == builtin_syntax[i].key)
        return builtin_syntax[i].handler(this);
    }

    // procs
    func = exp[0]->codeGenEval();
  }
  else if (exp[0]->getType() == SExprAST) {
    // possibly lambdas
    func = exp[0]->codeGenEval();
  }
  else {
    throw Error(std::string("not a function"));
  }

  // TODO: put things in one alloca
  // func -> func_ptr, type safety check
  addr = builder.CreateAlloca(LSObjType->getPointerTo(),
                              ConstantInt::get(Type::getInt32Ty(context), 1));
  builder.CreateStore(func, addr);
  builder.CreateCall3(module->getFunction("lsrt_check_arg_type"),
                      addr,
                      ConstantInt::get(Type::getInt32Ty(context), 0),
                      ConstantInt::get(Type::getInt8Ty(context), 'f'));
  addr = LSObjGetPointerAddr(context, func, 0, 1);
  func = builder.CreateLoad(addr);

  size = ConstantInt::get(Type::getInt32Ty(context), exp.size() - 1);
  addr = builder.CreateAlloca(LSObjType->getPointerTo(), size);

  for (i = 1; i < numArgument(); i++) {
    val = exp[i]->codeGenEval();
    builder.CreateStore(val, GEP1(context, addr, i - 1));
  }

  func = builder.CreateBitCast(func, LSFuncPtrType);
  return builder.CreateCall2(func, size, addr);
}


// Syntax handlers
static Value *handleBegin(SExprASTNode *sexpr) {
  Value *v = NULL;
  int i;

  if (sexpr->numArgument() == 1) // (begin)
    v = LSObjNew(context, ls_t_void);

  for (i = 1; i < sexpr->numArgument(); i++) {
    v = sexpr->getArgument(i)->codeGenEval();
  }

  if (sexpr->hasRest())
    throw Error(std::string("unexpected `.' in begin"));

  return v;
}

static Value *createFunction(SExprASTNode *def,
                             SExprASTNode *formals, int start,
                             const std::string &name) {
  Function *f;
  Function::arg_iterator ai;
  BasicBlock *bb, *prevb;
  BasicBlock::iterator previ;
  Value *obj, *argval;
  std::string fname;
  SymbolASTNode *arg;
  std::vector<Constant *> idx;
  int i, n;

  if (name == "")
    fname = ".anon";
  else
    fname = name;

  n = formals->numArgument() - start;
  f = Function::Create(LSFuncType, Function::PrivateLinkage, "_func_" + fname, module);

  obj = LSObjFunctionInit(context, f, name);

  // need to bind it if defined a name
  eenv.newScope();

  // save current insert block and restore later
  bb = BasicBlock::Create(context, "entry", f);
  prevb = builder.GetInsertBlock();
  previ = builder.GetInsertPoint();
  builder.SetInsertPoint(bb);

  ai = f->arg_begin();
  ai->setName("argc");
  builder.CreateCall3(module->getFunction("lsrt_check_args_count"),
                      llvm::ConstantInt::get(Type::getInt32Ty(context), n),
                      llvm::ConstantInt::get(Type::getInt32Ty(context), n),
                      ai);
  ai++;
  ai->setName("args");
  argval = builder.CreateLoad(ai);

  for (i = start; i < n ; i++) {
    if (formals->getArgument(i)->getType() != SymbolAST)
      throw Error(std::string("argument names need to be symbol"));

    arg = static_cast<SymbolASTNode *>(formals->getArgument(i));

    eenv.addBinding(arg->symbol, builder.CreateLoad(GEP1(context, ai, i - start)));
  }

  for (i = 2; i < def->numArgument(); i++)
    argval = def->getArgument(i)->codeGenEval();

  builder.CreateRet(argval);

  builder.SetInsertPoint(prevb, previ);
  eenv.lastScope();

  llvm::verifyFunction(*f);

  return obj;
}

static Value *handleDefine(SExprASTNode *sexpr) {
  (void) sexpr;
  return NULL;
}

static Value *handleLambda(SExprASTNode *sexpr) {

  SExprASTNode *formals;

  if (sexpr->numArgument() < 3)
    throw Error(std::string("too few arguments for lambda"));

  if (sexpr->getArgument(1)->getType() != SExprAST)
    throw Error(std::string("lambda expects argument list"));

  formals = static_cast<SExprASTNode *> (sexpr->getArgument(1));
  return createFunction(sexpr, formals, 0, "");
}

static Value *handleQuote(SExprASTNode *sexpr) {
  if (sexpr->numArgument() != 2)
    throw Error(std::string("quote takes only one argument"));

  return sexpr->getArgument(1)->codeGen();
}

/* vim: set et ts=2 sw=2 cin: */
