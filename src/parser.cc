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
#include "parser.hh"
#include "error.hh"
#include "ast2.hh"

Parser::Parser(Lexer* input, int k) {
  this->input = input;
  n_lookahead = k;
  c_index = 0;      // circular index of next token position to fill

  // fill in the circular lookahead token buffer
  for (int i = 0; i < k; ++i) {
    lookahead.push_back(input->nextToken());
  }
}

Parser::~Parser() {
}

void Parser::consume(void) {
#ifdef DEBUG
  std::cout << "consumed `" << lookahead[c_index].text << "'" << std::endl;
#endif
  lookahead[c_index] = input->nextToken();
  c_index = (c_index + 1) % n_lookahead;
}

// 1 <= idx <= k
const Token& Parser::peekToken(int idx) const {
  int t = (c_index + idx - 1) % n_lookahead;
  return lookahead[t];
}

int Parser::peekTokenType(int idx) const {
  return peekToken(idx).type;
}

void Parser::match(int tkType) {
  if (peekTokenType(1) == tkType) consume();
  else throw Error(std::string(std::string("expecting ") +
			       input->getTokenName(tkType) +
			       "; found " + peekToken(1).text));
}

// TODO:
// 1. clean up other members like in Parser class;
// 2. display line/column number when parser failed.
ASTNode *Parser::exp(void)
{
  int tk1 = peekTokenType(1);
  ASTNode *ast;

  switch (tk1) {
  case BOOL:
    ast = new BooleanASTNode(peekToken(1).text);
    consume();
    return ast;
  case STRING:
    ast = new StringASTNode(peekToken(1).text);
    consume();
    return ast;
  case NUMBER:
    ast = new NumberASTNode(peekToken(1).text);
    consume();
    return ast;
  case ID:
    ast = new SymbolASTNode(peekToken(1).text);
    consume();
    return ast;
  }

  if (tk1 == LPAREN) {
    SExprASTNode *sexpr = new SExprASTNode();
    consume();

    while (peekTokenType(1) != RPAREN) {
      if (peekTokenType(1) == PERIOD) {
        consume();
        sexpr->setRest(exp());
	match(RPAREN);
	return sexpr;
      }

      sexpr->addArgument(exp());
    }
    match(RPAREN);
    return sexpr;
  }

  if (tk1 == QUOTE) {
    SExprASTNode *sexpr = new SExprASTNode();
    consume();
    sexpr->addArgument(new SymbolASTNode("quote"));
    sexpr->addArgument(exp());

    return sexpr;
  }

  throw Error(std::string("unexpected token"));
}

void Parser::form(void)
{
  int tk1 = peekTokenType(1);

  switch (tk1) {
  case BOOL:
  case STRING:
  case NUMBER:
  case ID:
    match(tk1);			// parsed a constant
    return;
  }

  if (tk1 == LPAREN) {		// start with '('
    int tk2 = peekTokenType(2);
    if (tk2 == ID) {
      const Token& tk = peekToken(2);
      if (tk.text == "if") {
	ifexp(); return;
      } else if (tk.text == "define") {
	define(); return;
      } else if (tk.text == "lambda") {
	lambda(); return;
      } else if (tk.text == "quote") {
	match(LPAREN); quote(); match(RPAREN); return;
      }
    }

    apply();
  } else if (tk1 == QUOTE) {	// parsed a quote
    quote();
  } else throw Error(std::string("unexpected token: ") +
		     input->getTokenName(tk1));
}

void Parser::ifexp(void)
{
  match(LPAREN);
  match(ID);			// text = `if'
  form();			// cond
  form();			// then

  int tk1 = peekTokenType(1);
  if (tk1 != RPAREN)
    form();			// optional else

  match(RPAREN);
}

// (define (foo x) ...)
// (define foo ...)
void Parser::define(void)
{
  int tk3 = peekTokenType(3);

  match(LPAREN);
  match(ID);			// text = `define'

  if (tk3 == LPAREN) {
    match(LPAREN);
    match(ID);			// function name
    args();
    match(RPAREN);
  } else {
    match(ID);			// function name
  }

  form();
  match(RPAREN);
}

void Parser::lambda(void)
{
  match(LPAREN);
  match(ID);			// text = `lambda'

  match(LPAREN);		// function args
  args();
  match(RPAREN);

  form();			// function body

  match(RPAREN);
}

// Args can be empty, list or dotted list.
// args ::= ID* ('.' ID)?
void Parser::args(void)
{
  int tk;

  while ((tk = peekTokenType(1)) == ID)
    match(ID);

  if (tk == RPAREN)
    return;

  match(PERIOD);		// dotted params
  match(ID);
}

void Parser::quote(void)
{
  int tk, cnt;

  /* It's a little bit weird here since we donot have a preprocessor. */
  if (peekToken(1).text == "quote") match(ID);
  else match(QUOTE);

  tk = peekTokenType(1);	// quoted an atom
  if (tk != LPAREN && tk != RPAREN) {
    match(tk);
    return;
  }

  if (tk == RPAREN)
    throw Error(std::string("unexpeced `)'"));

  cnt = 1;			// number of unmatched LPAREN
  match(LPAREN);
  while (cnt > 0) {		// quoted a list
    tk = peekTokenType(1);

    if (tk == RPAREN) --cnt;
    else if (tk == LPAREN) ++cnt;
    else if (tk == EOF_TYPE) match(RPAREN);

    match(tk);
  }
}

void Parser::apply(void)
{
  int tk;

  match(LPAREN);
  form();

  // optional arguments
  while ((tk = peekTokenType(1)) != RPAREN)
    form();

  match(RPAREN);
}

/* vim: set et ts=2 sw=2 cin: */
