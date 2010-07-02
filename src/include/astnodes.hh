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

#ifndef ASTNODES_HH_
#define ASTNODES_HH_

#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

#include <llvm/DerivedTypes.h>

#include "ast.hh"

// TODO: floating point
class NumberASTNode :public ASTNode {
public:
  int val;
  NumberASTNode(const std::string &_s)
    :ASTNode(NumberAST), val(std::atoi(_s.c_str())) {}

  void accept(ASTVisitor &v) {
    v.visitNumberAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitNumberAST(this);
  }

  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
};

class BignumASTNode :public ASTNode {
public:
  std::string val;
  BignumASTNode(const std::string &_s)
    :ASTNode(BignumAST), val(_s) {}

  void accept(ASTVisitor &v) {
    v.visitBignumAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitBignumAST(this);
  }

  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
};

class BooleanASTNode :public ASTNode {
public:
  bool boolean;
  BooleanASTNode(const std::string &_s)
    :ASTNode(BooleanAST), boolean(_s == "#t") {}

  void accept(ASTVisitor &v) {
    v.visitBooleanAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitBooleanAST(this);
  }

  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
};

class SymbolASTNode :public ASTNode {
public:
  std::string symbol;
  SymbolASTNode(const std::string &_s)
    :ASTNode(SymbolAST), symbol(_s) {}
  SymbolASTNode(SymbolASTNode &n)
    :ASTNode(n), symbol(n.symbol) {}

  void accept(ASTVisitor &v) {
    v.visitSymbolAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitSymbolAST(this);
  }

  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
  llvm::Value *codeGenNoBind();
  llvm::Value *codeGenEval();
private:
  llvm::Value *getGlobal();
};

class StringASTNode :public ASTNode {
public:
  std::string str;
  StringASTNode(const std::string &_s)
    :ASTNode(StringAST), str(_s) {}
  StringASTNode(StringASTNode &n)
    :ASTNode(n), str(n.str) {}

  void accept(ASTVisitor &v) {
    v.visitStringAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitStringAST(this);
  }

  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
};

class SExprASTNode :public ASTNode {
  std::vector<ASTNode *> args;
  ASTNode *last;
public:

  typedef std::vector<ASTNode *>::iterator arg_iterator;
  typedef std::vector<ASTNode *>::const_iterator const_arg_iterator;

  SExprASTNode()
    :ASTNode(SExprAST), last(NULL) {}

  SExprASTNode(SExprASTNode &n) :ASTNode(n) {
    arg_iterator i;
    for (i = n.arg_begin(); i != n.arg_end(); i++) {
      ASTNode *a = *i;
      a->get();
      args.push_back(a);
    }
    if (n.last) {
      n.last->get();
      last = n.last;
    }
  }

  arg_iterator arg_begin() {
    return args.begin();
  }

  const_arg_iterator arg_begin() const {
    return args.begin();
  }

  arg_iterator arg_end() {
    return args.end();
  }

  const_arg_iterator arg_end() const {
    return args.end();
  }

  int args_size() const {
    return args.size();
  }

  void accept(ASTVisitor &v) {
    v.visitSExprAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitSExprAST(this);
  }

  void addArgument(ASTNode *arg) {
    arg->get();
    args.push_back(arg);
  }

  int numArgument() const {
    return args.size();
  }

  ASTNode *getArgument(int i) {
    return args[i];
  }

  bool hasLast() const {
    return last != NULL;
  }

  ASTNode *getLast() {
    return last;
  }

  void setLast(ASTNode *r) {
    if (last != NULL && last != r) {
      PutASTNode(last);
      last = r;
      r->get();
    }
  }

  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
  llvm::Value *codeGenEval();
};
#endif

/* vim: set et ts=2 sw=2 cin: */
