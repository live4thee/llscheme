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

  obj = LSObjNew(context, 1);
  addr = LSObjGetValueAddr(context, obj, 0, 1);
  builder.CreateStore(ConstantInt::get(Type::getInt32Ty(context), val), addr);

  return obj;
}

Value *BooleanASTNode::codeGen() {
  return NULL;
}

// N.B. unoptimized symbol retrieve needs to be runtime
Value *SymbolASTNode::codeGen() {
  // SymbolASTNode -> symbol ls_object, in lexical scope
  // codegen: retrieve symbol binding
  return NULL;
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
  int i;

  if (exp[0]->getType() == SymbolAST) {
    SymbolASTNode *node = static_cast<SymbolASTNode *>(exp[0]);
    for (i = 0; i < num_builtin_syntax; i++) {
      if (node->symbol == builtin_syntax[i].key)
        return builtin_syntax[i].handler(this);
    }
  }
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
  return NULL;
}


// Syntax handlers
static Value *handleBegin(SExprASTNode *sexpr) {
  Value *v = NULL;
  int i;

  if (sexpr->numArgument() == 1) // (begin)
    v = LSObjNew(context, 0);

  for (i = 1; i < sexpr->numArgument(); i++) {
    v = sexpr->getArgument(i)->codeGen();
  }

  if (sexpr->hasRest())
    throw Error(std::string("unexpected `.' in begin"));

  return v;
}

static Value *handleDefine(SExprASTNode *sexpr) {
  (void) sexpr;
  return NULL;
}

static Value *handleLambda(SExprASTNode *sexpr) {
  (void) sexpr;
  // codegen: argc and args check as part of the code
  return NULL;
}

static Value *handleQuote(SExprASTNode *sexpr) {
  (void) sexpr;
  // turn ASTNode to ls_object
  // turn s-expression to nested pairs
  return NULL;
}

/* vim: set et ts=2 sw=2 cin: */
