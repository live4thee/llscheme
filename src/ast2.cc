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

#include <cstdlib>
#include "ast2.hh"

NumberASTNode::NumberASTNode(const std::string &_s)
  :ASTNode(NumberAST)
{
  val = std::atoi(_s.c_str());
}

BooleanASTNode::BooleanASTNode(const std::string &_s)
  :ASTNode(BooleanAST)
{
  if (_s == "#t")
    boolean = 1;
  else
    boolean = 0;
}

// TODO: line breaking with proper column setting

void NumberASTNode::finePrint(std::stringstream &ss) {
  ss << val;
}

void BooleanASTNode::finePrint(std::stringstream &ss) {
  if (boolean)
    ss << "#t";
  else
    ss << "#f";
}

void SymbolASTNode::finePrint(std::stringstream &ss) {
  ss << symbol;
}

void StringASTNode::finePrint(std::stringstream &ss) {
  ss << '"' << str << '"';
}

void SExprASTNode::finePrint(std::stringstream &ss) {
  int size = exp.size();
  int i;
  ss << '(';

  if (size != 0) {
    exp[0]->finePrint(ss);
    for (i = 1; i < size; i++) {
      ss << ' ';
      exp[i]->finePrint(ss);
    }
  }

  if (rest) {
    ss << " . ";
    rest->finePrint(ss);
  }

  ss << ')';
}
  
