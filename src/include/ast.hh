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

#ifndef AST_HH_
#define AST_HH_

// We need some type identification system similar to RTTI...

enum ASTType {
  NumberAST, SymbolAST, LambdaAST, FormalAST, ExprAST, QuoteAST, ProcCallAST, DefineAST,
  ArgAST, IfExprAST,
};

class ASTNode {
  enum ASTType type;
public:
  virtual enum ASTType getType() = 0;
  virtual ~ASTNode() {}
};

class NumberASTNode :public ASTNode {
  int val;
public:
  NumberASTNode(int _v)
    :val(_v), type(NumberAST) {}
};

class SymbolASTNode :public ASTNode {
  std::string symbol;
public:
  SymbolASTNode(std::string str)
    :symbol(str), type(SymbolAST) {}
};

// (x y . z)  =>  Args[]<-x,y, rest arg<-z
class FormalASTNode :public ASTNode {
  std::vector<SymbolASTNode *> args;
  //TODO: how to represent rest args?
public:
  FormalASTNode()
    :type(FormalAST) {}

  void addArgument(SymbolASTNode *arg);
  int numArgument();
  SymbolASTNode *getArgument(int i);
};

class LambdaASTNode :public ASTNode {
  FormalASTNode *args;
  ASTNode *expr;  //expression
public:
  LambdaAST(FormalASTNode *formal, ASTNode *block)
    :args(formal), expr(block), type(LambdaAST) {}
};

class ArgASTNode :public ASTNode {
  std::vector<ASTNode *> args;
public:
  ArgASTNode()
    :type(ArgAST) {}

  void addArgument(ASTNode *arg);
  int numArgument();
  ASTNode *getArgument(int i);
};

class ProcCallASTNode :public ASTNode {
  ASTNode *callee;   // can be symbol or expression
  ArgASTNode *args;
public:
  ProcCallASTNode(ASTNode *_c, ArgASTNode *_a)
    :callee(_c), args(_a), type(ProcCallAST) {}
};
  
#endif
