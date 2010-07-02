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

#ifndef AST_HH_
#define AST_HH_

#include <llvm/DerivedTypes.h>

//--------------------------------------------------------------------
// types
//--------------------------------------------------------------------

enum ASTType {
  NumberAST, BooleanAST, SymbolAST, StringAST, SExprAST, VectorAST,
  UnknownAST = 255,
};

class ASTNode;
class NumberASTNode;
class BooleanASTNode;
class SymbolASTNode;
class StringASTNode;
class SExprASTNode;
class VectorASTNode;

//--------------------------------------------------------------------
// visitors and mutable visitors:
//--------------------------------------------------------------------

//   Apart from common visitors, we introduce here a special type
// of mutable visitors, to allow the transformation of AST. The
// new ASTNode is returned as a pointer instead of void, while the
// old ASTNode is saved in `lastGen' for further reference.
//
//   The caller is responsible for installing the new pointer, and
// taking care of the old replaced ASTNode (in lastGen) if necessary.
// Generally, the visitor is in charge to traverse.

template <typename T>
class _ASTVisitor {
public:
  virtual T visitNumberAST(NumberASTNode *node);
  virtual T visitBooleanAST(BooleanASTNode *node);
  virtual T visitSymbolAST(SymbolASTNode *node);
  virtual T visitStringAST(StringASTNode *node);
  virtual T visitSExprAST(SExprASTNode *node);
  virtual T visitVectorAST(VectorASTNode *node);
protected:
  _ASTVisitor() {}
};

typedef _ASTVisitor<void> ASTVisitor;
typedef _ASTVisitor<ASTNode *> ASTVisitorMutable;

//--------------------------------------------------------------------
// node base class
//--------------------------------------------------------------------

class ASTNode {
protected:
  const enum ASTType type;
  ASTNode *lastGen;

public:
  ASTNode(enum ASTType _t = UnknownAST) :type(_t), lastGen(NULL) {}
  virtual ~ASTNode() {}

  // lastGen never preserve during copy
  ASTNode(ASTNode &n) :type(n.type), lastGen(NULL) {}

  enum ASTType getType() const {
    return type;
  }

  ASTNode *getLastGen() const {
    return lastGen;
  }

  void setLastGen(ASTNode *l) {
    lastGen = l;
  }

  virtual void accept(ASTVisitor &v) = 0;
  virtual ASTNode *acceptMutable(ASTVisitorMutable &v) = 0;

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
};

#endif

/* vim: set et ts=2 sw=2 cin: */
