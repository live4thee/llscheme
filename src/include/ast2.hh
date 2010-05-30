// Copyright (C) 2010 Qing He <qing.x.he@gmail.com>
// Copyright (C) 2010 David Lee <live4thee@gmail.com>
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

// TODO: rename to ls_object as the internal data representation as well as
//       the AST representation
// TODO: rewrite in C to has an ABI

// We need some type identification system similar to RTTI...

enum ASTType {
  NumberAST, SymbolAST, SExprAST, UnknownAST
};

class ASTNode {
protected:
  enum ASTType type;
public:
  ASTNode(enum ASTType _t = UnknownAST) :type(_t) {}
  virtual enum ASTType getType() { return type; }
  virtual ~ASTNode() {}
};

class NumberASTNode :public ASTNode {
  int val;
public:
  NumberASTNode(int _v)
    :ASTNode(NumberAST), val(_v) {}
};

class SymbolASTNode :public ASTNode {
  std::string symbol;
public:
  SymbolASTNode(const std::string &_s)
    :ASTNode(SymbolAST), symbol(_s) {}
};

class SExprASTNode :public ASTNode {
  std::vector<ASTNode *> exp;
  ASTNode *rest;
public:
  SExprASTNode()
    :ASTNode(SExprAST), rest(0) {}

  void addArgument(ASTNode *arg) {
    exp.push_back(arg);
  }

  void setRest(ASTNode *r) {
    rest = r;
  }

  int numArgument() {
    return exp.size();
  }

  bool hasRest() {
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

// Local Variables:
// c-basic-offset: 2
// End:
