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
#include "codegen.hh"

llvm::Module *module;
llvm::IRBuilder<> builder(llvm::getGlobalContext());

// need a runtime ls_object type that fits to llvm::Value

llvm::Value *NumberASTNode::codeGen() {
}

llvm::Value *BooleanASTNode::codeGen() {
}

// N.B. unoptimized symbol retrieve needs to be runtime
llvm::Value *SymbolASTNode::codeGen() {
  // SymbolASTNode -> symbol ls_object, in lexical scope
  // codegen: retrieve symbol binding
}

llvm::Value *StringASTNode::codeGen() {
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
typedef llvm::Value *syntaxHandler(SExprASTNode *);
static syntaxHandler handleBegin;
static syntaxHandler handleDefine;
static syntaxHandler handleLambda;
static syntaxHandler handleQuote;

static const struct {
  const std::string &key;
  syntaxHandler *handler;
} builtin_syntax[] = {
  { "begin", &handleBegin },
  { "define", &handleDefine },
  { "lambda", &handleLambda },
  { "quote", &handleQuote },
};

llvm::Value *SExprASTNode::codeGen() {
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
}


// Syntax handlers
static llvm::Value *handleBegin(SExprASTNode *sexpr) {
  // codegen: eval the remaining args
}

static llvm::Value *handleDefine(SExprASTNode *sexpr) {
}

static llvm::Value *handleLambda(SExprASTNode *sexpr) {
  // codegen: argc and args check as part of the code
}

static llvm::Value *handleQuote(SExprASTNode *sexpr) {
  // turn ASTNode to ls_object
  // turn s-expression to nested pairs
}
