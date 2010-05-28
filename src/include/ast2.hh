// Copyright (C) 2010 David Lee <live4thee@gmail.com>
// Copyright (C) 2010 Qing He <qing.x.he@gmail.com>
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

// We need some type identification system similar to RTTI...

enum ASTType {
  NumberAST, SymbolAST, SExprAST,
};

class ASTNode {
  enum ASTType type;
public:
  virtual enum ASTType getType() { return type; }
  virtual ~ASTNode() {}
};

class NumberASTNode :public ASTNode {
  int val;
public:
  NumberASTNode(int _v)
    :val(_v), type(NumberAST) {}
};

class SExprASTNode :public ASTNode {
  std::vector<ASTNode *> exp;
  ASTNode *rest;
public:
  SExprASTNode()
    :rest(NULL), type(SExprASTNode) {}

  void addArgument(ASTNode *arg) {
    exp.push_back(arg);
  }

  void setRest(ASTNode *r) {
    reset = r;
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
