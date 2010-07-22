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

class NumberASTNode :public ASTNode {
  std::string val;

public:
  NumberASTNode(const std::string &_s)
    :ASTNode(NumberAST), val(_s) {}

  void accept(ASTVisitor &v) {
    v.visitNumberAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitNumberAST(this);
  }

  void finePrint(std::stringstream &ss) const {
    ss << val;
  }

  llvm::Value *codeGen();
};

class BooleanASTNode :public ASTNode {
  bool boolean;

public:
  BooleanASTNode(const std::string &_s)
    :ASTNode(BooleanAST), boolean(_s == "#t") {}

  void accept(ASTVisitor &v) {
    v.visitBooleanAST(this);
  }

  ASTNode *acceptMutable(ASTVisitorMutable &v) {
    return v.visitBooleanAST(this);
  }

  void finePrint(std::stringstream &ss) const {
    if (boolean) ss << "#t";
    else ss << "#f";
  }

  llvm::Value *codeGen();
};

class SymbolASTNode :public ASTNode {
public:
  const std::string symbol;
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

  void finePrint(std::stringstream &ss) const {
    ss << symbol;
  }

  llvm::Value *codeGen();
  llvm::Value *codeGenNoBind();
  llvm::Value *codeGenEval();

private:
  llvm::Value *getGlobal();
};

class StringASTNode :public ASTNode {
  const std::string str;

public:
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

  void finePrint(std::stringstream &ss) const {
    ss << '"' << str << '"';
  }

  llvm::Value *codeGen();
};

class SExprASTNode :public ASTNode {
  std::vector<ASTNode *> args;
  bool is_dotted;

public:
  typedef std::vector<ASTNode *>::iterator arg_iterator;
  typedef std::vector<ASTNode *>::const_iterator const_arg_iterator;

  SExprASTNode()
    :ASTNode(SExprAST), is_dotted(false) {}

  SExprASTNode(SExprASTNode &n) :ASTNode(n) {
    arg_iterator i;
    for (i = n.arg_begin(); i != n.arg_end(); i++) {
      ASTNode *a = *i;
      args.push_back(a);
    }
    is_dotted = n.is_dotted;
  }

  virtual ~SExprASTNode() {
    arg_iterator i;
    for (i = this->arg_begin(); i != this->arg_end(); i++) {
      delete (*i);
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
    args.push_back(arg);
  }

  int numArgument() const {
    return args.size();
  }

  ASTNode *operator[](size_t i) {
    return args[i];
  }

  bool isDotted() const {
    return is_dotted;
  }

  SExprASTNode* tagDotted() {
    is_dotted = true;
    return this;
  }

  void finePrint(std::stringstream &ss) const {
    int size = this->args_size();
    int i;
    ss << '(';

    if (size > 0) {
      args[0]->finePrint(ss);

      for (i = 1; i < size - 1; i++) {
        ss << ' ';
        args[i]->finePrint(ss);
      }

      if (this->isDotted())
        ss << " . ";

      args[i]->finePrint(ss);
    }

    ss << ')';
  }

  llvm::Value *codeGen();
  llvm::Value *codeGenEval();
};
#endif

/* vim: set et ts=2 sw=2 cin: */
