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

#ifndef AST2_HH_
#define AST2_HH_

#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

#include <llvm/DerivedTypes.h>

// We need some type identification system similar to RTTI...

enum ASTType {
  NumberAST, BooleanAST, SymbolAST, StringAST, SExprAST, UnknownAST
};

class ASTNode {
protected:
  enum ASTType type;
public:
  ASTNode(enum ASTType _t = UnknownAST) :type(_t) {}

  virtual void finePrint(std::stringstream &ss) const = 0;
  // codeGen generates code to represent the object, while
  // codeGenEval does an additional evaluation, for example:
  //  - they behave the same for numbers, booleans and strings
  //  - for symbol, codeGen returns the symbol object, while
  //    codeGenEval returns the object bound to the symbol
  //  - for s-expr, codeGenEval evaluates the s-expr, while
  //    codeGen generates pair representation
  virtual llvm::Value *codeGen() = 0;
  virtual llvm::Value *codeGenNoBind() { return codeGen(); }
  virtual llvm::Value *codeGenEval() { return codeGen(); }
  virtual enum ASTType getType() const { return type; }
  virtual ~ASTNode() {}
};

// TODO: floating point and big number
class NumberASTNode :public ASTNode {
public:
  int val;
  NumberASTNode(const std::string &_s)
    :ASTNode(NumberAST), val(std::atoi(_s.c_str())) {}
  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
};

class BooleanASTNode :public ASTNode {
public:
  bool boolean;
  BooleanASTNode(const std::string &_s)
    :ASTNode(BooleanAST), boolean(_s == "#t") {}
  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
};

class SymbolASTNode :public ASTNode {
public:
  std::string symbol;
  SymbolASTNode(const std::string &_s)
    :ASTNode(SymbolAST), symbol(_s) {}
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
  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
};

class SExprASTNode :public ASTNode {
  std::vector<ASTNode *> exp;
  ASTNode *rest;
public:
  SExprASTNode()
    :ASTNode(SExprAST), rest(0) {}

  void finePrint(std::stringstream &ss) const;
  llvm::Value *codeGen();
  llvm::Value *codeGenEval();
  void addArgument(ASTNode *arg) {
    exp.push_back(arg);
  }

  void setRest(ASTNode *r) {
    rest = r;
  }

  int numArgument() const {
    return exp.size();
  }

  bool hasRest() const {
    return rest != NULL;
  }

  std::vector<ASTNode *> &getArguments() {
    return exp;
  }

  ASTNode *getArgument(int i) {
    return exp[i];
  }

  ASTNode *getRest() {
    return rest;
  }
};
#endif

/* vim: set et ts=2 sw=2 cin: */
