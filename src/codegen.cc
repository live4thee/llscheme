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

#include "astnodes.hh"
#include "driver.hh"
#include "error.hh"
#include "env.hh"

#include <llvm/Analysis/Verifier.h>
#include <vector>
#include <iostream>
#include <cctype>

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

static inline bool
numberFitsInt32(const std::string &val) {
  std::string::const_iterator i;

  if (val.size() > 9)
    return false;

  for(i = val.begin(); i != val.end(); i++)
    if (!std::isdigit(*i))
      return false;

  return true;
}

Value *NumberASTNode::codeGen() {
  Value *obj, *addr;
  int num;

  if (numberFitsInt32(val)) {
    num = std::atoi(val.c_str());
    obj = LSObjSimpleNumberInit(context, num);
  }
  else {
    SExprASTNode sexp;
    sexp.addArgument(new SymbolASTNode("string->number"));
    sexp.addArgument(new StringASTNode(val));
    obj = sexp.codeGenEval();
    // N.B. no need to explicitly delete arguments, they are automatically
    // destructed by refcnt system
  }

  return obj;
}

Value *BooleanASTNode::codeGen() {
  Value *obj;

  obj = LSObjNew(context, ls_t_boolean);
  builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), boolean),
                      LSObjGetValueAddr(context, obj, 0, 1));
  return obj;
}

Value *SymbolASTNode::getGlobal() {
  Constant *str, *s, *init;
  Value *g;
  std::vector<Constant *> v, m, idx;

  // N.B. local variables must be added manually, unknown symbols
  // are treated as global.
  g = eenv.searchGlobalBinding(symbol);
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

Value *SymbolASTNode::codeGen() {
  Value *s;

  s = eenv.searchLocalBinding(symbol);
  if (s)
      return s;

  return getGlobal();
}

Value *SymbolASTNode::codeGenNoBind() {
  Value *obj;
  Constant *str, *s;

  obj = LSObjNew(context, ls_t_symbol);
  str = llvm::ConstantArray::get(context, symbol);
  s = new llvm::GlobalVariable(*module, str->getType(), true,
                               llvm::GlobalValue::PrivateLinkage, str, "_str_snb_" + symbol);

  builder.CreateStore(Constant::getNullValue(Type::getInt8Ty(context)->getPointerTo()),
                      LSObjGetPointerAddr(context, obj, 0, 1));
  builder.CreateStore(builder.CreateBitCast(s,
                                            Type::getInt8Ty(context)->getPointerTo()),
                      LSObjGetPointerAddr(context, obj, 0, 2));

  return obj;
}

Value *SymbolASTNode::codeGenEval() {
  Value *s;
  Value *addr, *val;

  // TODO: does local variables needs another redirection like globals?
  s =eenv.searchLocalBinding(symbol);
  if (s)
      return s;

  s = getGlobal();

  builder.CreateCall(module->getFunction("lsrt_check_symbol_unbound"), s);
  addr = LSObjGetPointerAddr(context, s, 0, 1);
  val = builder.CreateBitCast(builder.CreateLoad(addr),
                              LSObjType->getPointerTo());

  return val;
}

Value *StringASTNode::codeGen() {
  return LSObjStringInit(context, str);
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
static syntaxHandler handleIf;

static const struct _builtin_syntax {
  const std::string key;
  syntaxHandler *handler;
} builtin_syntax[] = {
  { "begin", &handleBegin },
  { "define", &handleDefine },
  { "lambda", &handleLambda },
  { "quote", &handleQuote },
  { "if", &handleIf},
  //  { "or", &handleOr},
  //  { "and", &handleAnd},
};
static const int num_builtin_syntax =
  sizeof(builtin_syntax) / sizeof(_builtin_syntax);

// N.B. codeGen() and codeGenEval() of SExprAST are totally different
Value *SExprASTNode::codeGen() {
  Value *head, *pair, *curr;
  unsigned int i;

  if (args.size() == 0) {
    return LSObjNew(context, ls_t_nil);
  }

  head = pair = LSObjNew(context, ls_t_pair);
  builder.CreateStore(builder.CreateBitCast(args[0]->codeGenNoBind(),
                                            Type::getInt8Ty(context)->getPointerTo()),
                      LSObjGetPointerAddr(context, pair, 0, 1));

  for (i = 1; i < args.size(); i++) {
    curr = LSObjNew(context, ls_t_pair);
    builder.CreateStore(builder.CreateBitCast(args[i]->codeGenNoBind(),
                                              Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, curr, 0, 1));
    builder.CreateStore(builder.CreateBitCast(curr,
                                              Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, pair, 0, 2));
    pair = curr;
  }

  if (last == NULL)
    builder.CreateStore(builder.CreateBitCast(LSObjNew(context, ls_t_nil),
                                              Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, pair, 0, 2));
  else
    builder.CreateStore(builder.CreateBitCast(last->codeGenNoBind(),
                                              Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, pair, 0, 2));

  return head;
}

Value *SExprASTNode::codeGenEval() {
  int i;
  Value *func = NULL, *addr, *val, *fptr, *free;
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
  // TODO: ( args . last)
  if (args.size() == 0)
    return codeGen();

  if (args[0]->getType() == SymbolAST) {
    SymbolASTNode *node = static_cast<SymbolASTNode *>(args[0]);
    // syntax
    for (i = 0; i < num_builtin_syntax; i++) {
      if (node->symbol == builtin_syntax[i].key)
        return builtin_syntax[i].handler(this);
    }

    // procs
    func = args[0]->codeGenEval();
  }
  else if (args[0]->getType() == SExprAST) {
    // possibly lambdas
    func = args[0]->codeGenEval();
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
  fptr = builder.CreateLoad(addr);
  free = builder.CreateLoad(LSObjGetPointerAddr(context, func, 0, 2));

  size = ConstantInt::get(Type::getInt32Ty(context), args.size() - 1);
  addr = builder.CreateAlloca(LSObjType->getPointerTo(), size);

  for (i = 1; i < numArgument(); i++) {
    val = args[i]->codeGenEval();
    builder.CreateStore(val, GEP1(context, addr, i - 1));
  }

  fptr = builder.CreateBitCast(fptr, LSFuncPtrType);
  free = builder.CreateBitCast(free, LSObjType->getPointerTo()->getPointerTo());
  return builder.CreateCall3(fptr, size, addr, free);
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

  if (sexpr->hasLast())
    throw Error(std::string("unexpected `.' in begin"));

  return v;
}

static Value *createFunction(SExprASTNode *def,
                             SExprASTNode *formals, int start,
                             const std::string &name) {
  Function *f;
  Function::arg_iterator ai;
  BasicBlock *bb, *prevb, *bbprolog;
  BasicBlock::iterator previ;
  Value *obj, *argval, *free;
  std::string fname;
  SymbolASTNode *arg;
  std::vector<Constant *> idx;
  Binding *refs;
  Binding::iterator it;
  int i, n;

  if (name == "")
    fname = ".anon";
  else
    fname = name;

  n = formals->numArgument() - start;
  f = Function::Create(LSFuncType, Function::PrivateLinkage, "_func_" + fname, module);

  obj = LSObjFunctionInit(context, f, fname);

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

  for (i = 0; i < n ; i++) {
    if (formals->getArgument(i + start)->getType() != SymbolAST)
      throw Error(std::string("argument names need to be symbol"));

    arg = static_cast<SymbolASTNode *>(formals->getArgument(i + start));

    eenv.addBinding(arg->symbol, builder.CreateLoad(GEP1(context, ai, i)));
  }

  for (i = 2; i < def->numArgument(); i++)
    argval = def->getArgument(i)->codeGenEval();

  builder.CreateRet(argval);

  // By now, we know what free args the function uses
  refs = eenv.getCurrentRefs();
  ai++;
  ai->setName("freelist");

  if (refs->size() != 0) {
    // replacing previous alloca with the real load in prolog
    bbprolog = BasicBlock::Create(context, "prolog");
    builder.SetInsertPoint(bbprolog);

    for(it = refs->begin(), i = 0; it != refs->end(); it++, i++) {
      argval = eenv.searchLocalBinding(it->first);
      if (argval == NULL)
        throw Error(std::string("internal closure error"));

      argval->replaceAllUsesWith(builder.CreateLoad(GEP1(context, ai, i)));
      delete argval;
    }

    builder.CreateBr(bb);
    f->getBasicBlockList().push_front(bbprolog);
  }

  // putting the freelist into generated function object in parent function
  builder.SetInsertPoint(prevb, previ);

  if (refs->size() == 0) {
    builder.CreateStore(Constant::getNullValue(Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, obj, 0, 2));
  }
  else {
    Value *newobj;

    newobj = LSObjNew(context, ls_t_func);
    obj->replaceAllUsesWith(newobj);
    static_cast<llvm::GlobalVariable *> (obj)->eraseFromParent();
    obj = newobj;

    builder.CreateStore(builder.CreateBitCast(f, Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, obj, 0, 1));


    free = builder.CreateCall(module->getFunction("lsrt_new_freelist"),
                              ConstantInt::get(Type::getInt32Ty(context), refs->size()));
    for(it = refs->begin(), i = 0; it != refs->end(); it++, i++) {
      builder.CreateCall3(module->getFunction("lsrt_fill_freelist"),
                          free,
                          ConstantInt::get(Type::getInt32Ty(context), i),
                          it->second);
    }
    builder.CreateStore(builder.CreateBitCast(free,
                                              Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, obj, 0, 2));
  }

  // it is now safe to exit the scope and discard the env lists
  eenv.lastScope();

  llvm::verifyFunction(*f);

  return obj;
}

static Value *handleDefine(SExprASTNode *sexpr) {
  SExprASTNode *formals;
  SymbolASTNode *sym;
  Value *val;
  int n;

  if (sexpr->numArgument() < 3)
    throw Error(std::string("too few arguments for define"));

  if (sexpr->getArgument(1)->getType() == SymbolAST) {
    // define a symbol
    if (sexpr->numArgument() != 3)
      throw Error(std::string("defining symbol takes 2 arguments"));

    // XXX: let's put define also in global scope
    sym = static_cast<SymbolASTNode *> (sexpr->getArgument(1));
    if (eenv.searchLocalBinding(sym->symbol))
      throw Error(std::string("can't define in this scope"));

    val = sym->codeGen();
    builder.CreateStore(builder.CreateBitCast(sexpr->getArgument(2)->codeGenEval(),
                                              Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, val, 0, 1));
  }
  else if (sexpr->getArgument(1)->getType() == SExprAST) {
    // define a proc
    formals = static_cast<SExprASTNode *> (sexpr->getArgument(1));
    n = formals->numArgument();
    if (n < 1 || formals->getArgument(1)->getType() != SymbolAST)
      throw Error(std::string("bad syntax for define"));

    sym = static_cast<SymbolASTNode *> (formals->getArgument(0));
    if (eenv.searchLocalBinding(sym->symbol))
      throw Error(std::string("can't define in this scope"));

    val = sym->codeGen();
    builder.CreateStore(builder.CreateBitCast(createFunction(sexpr, formals, 1,
                                                             sym->symbol),
                                              Type::getInt8Ty(context)->getPointerTo()),
                        LSObjGetPointerAddr(context, val, 0, 1));
  }
  else
    throw Error(std::string("bad syntax for define"));

  return LSObjNew(context, ls_t_void);
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

  return sexpr->getArgument(1)->codeGenNoBind();
}

static Value *handleIf(SExprASTNode *sexpr) {
  if (sexpr->numArgument() != 3 && sexpr->numArgument() != 4)
    throw Error(std::string("if takes 2 or 3 arguments"));

  // lsrt_test_exp takes any objects and returns a boolean
  Value *test = builder.CreateCall(module->getFunction("lsrt_test_expr"),
                                   sexpr->getArgument(1)->codeGenEval());
  test = builder.CreateICmpNE(test,
                              ConstantInt::get(Type::getInt32Ty(context), 0));

  Function *func = builder.GetInsertBlock()->getParent();

  BasicBlock *thenbb = BasicBlock::Create(context, "then", func);
  BasicBlock *elsebb = BasicBlock::Create(context, "else");
  BasicBlock *mergebb = BasicBlock::Create(context, "merge");

  builder.CreateCondBr(test, thenbb, elsebb);

  builder.SetInsertPoint(thenbb);
  Value *thenval = sexpr->getArgument(2)->codeGenEval();
  builder.CreateBr(mergebb);
  thenbb = builder.GetInsertBlock();

  Value *elseval;

  func->getBasicBlockList().push_back(elsebb);
  builder.SetInsertPoint(elsebb);
  if (sexpr->numArgument() == 4)
    elseval = sexpr->getArgument(3)->codeGenEval();
  else
    elseval = LSObjNew(context, ls_t_void);

  builder.CreateBr(mergebb);
  elsebb = builder.GetInsertBlock();

  func->getBasicBlockList().push_back(mergebb);
  builder.SetInsertPoint(mergebb);

  llvm::PHINode *phi = builder.CreatePHI(LSObjType->getPointerTo());

  phi->addIncoming(thenval, thenbb);
  phi->addIncoming(elseval, elsebb);

  return phi;
}
/* vim: set et ts=2 sw=2 cin: */
