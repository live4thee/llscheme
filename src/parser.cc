// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
//
// Copyright (C)
//         2010 David Lee <live4thee@gmail.com>
//         2010 Qing He <qing.x.he@gmail.com>
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

#include <iostream>
#include <stack>
#include "parser.hh"
#include "error.hh"
#include "astnodes.hh"

Parser::Parser(Lexer* input) {
  this->input = input;
}

Parser::~Parser() {
}

void Parser::fillToken(int count) {
  for (int i = 0; i < count; i++) {
    lookahead.push_back(input->nextToken());
  }
}

void Parser::consume(void) {
  if (lookahead.empty())
    fillToken(1);

  lookahead.pop_front();
}

const Token& Parser::peekToken(int idx) {
  if (lookahead.size() < idx)
    fillToken(idx - lookahead.size());

  return lookahead[idx - 1];
}

int Parser::peekTokenType(int idx) {
  return peekToken(idx).type;
}

void Parser::match(int tkType) {
  if (peekTokenType(1) == tkType) consume();
  else throw Error(std::string(std::string("expecting ") +
			       input->getTokenName(tkType) +
			       "; found " + peekToken(1).text));
}

// TODO:
// display line/column number when parser failed.
ASTNode *Parser::parseSExp(void)
{
  int tktype;
  std::stack<SExprASTNode *> stk;

#define CONSUME_TOK(nodeType, tkString) do {            \
  ASTNode* ast = new nodeType(tkString); consume();     \
  if (stk.empty()) return ast;                          \
  else stk.top()->addArgument(ast);                     \
} while (0)

#define REDUCE_STK(stk) do {                                      \
  if (stk.size() == 1) return stk.top();                          \
  else {                                                          \
    ASTNode* t = stk.top(); stk.pop(); stk.top()->addArgument(t); \
  }                                                               \
} while (0)

  while ((tktype = peekTokenType(1)) != Lexer::EOF_TYPE) {
    const std::string& tktext = peekToken(1).text;

    switch (tktype) {
      case Lexer::BOOL: CONSUME_TOK(BooleanASTNode, tktext); break;
      case Lexer::STRING: CONSUME_TOK(StringASTNode, tktext); break;
      case Lexer::NUMBER: CONSUME_TOK(NumberASTNode, tktext); break;
      case Lexer::SYMBOL: CONSUME_TOK(SymbolASTNode, tktext); break;

        // The form 'something will be expanded to (quote something)
        // We manually reduce the stack here 'cause a RPAREN is omitted.
      case Lexer::QUOTE:
        stk.push(new SExprASTNode()); consume();
        stk.top()->addArgument(new SymbolASTNode("quote"));
        stk.top()->addArgument(this->parseSExp());
        REDUCE_STK(stk);
        break;

      case Lexer::LPAREN:
        stk.push(new SExprASTNode()); consume();
        break;

      case Lexer::PERIOD:
        consume();
        if (stk.empty()) throw Error(std::string("unexpected token: '.'"));
        stk.top()->tagDotted()->addArgument(this->parseSExp());
        this->match(Lexer::RPAREN);
        REDUCE_STK(stk);
        break;

      case Lexer::RPAREN:
        consume();
        switch (stk.size()) {
          case 0: throw Error("unexpected token: ')'"); break;
          default: REDUCE_STK(stk);
        }
        break;

      default:
        throw Error(std::string("unexpected token: '") + tktext + "'");
    }
  }
}

/* vim: set et ts=2 sw=2 cin: */
