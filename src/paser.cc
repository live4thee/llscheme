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

#include "parser.hh"
#include "error.hh"

Parser::Parser(Lexer* input, int k) {
  this->input = input;
  n_lookahead = k;
  c_index = 0;      // circular index of next token position to fill

  // fill in the circular lookahead token buffer
  for (int i = 0; i < k; ++i) {
    lookahead.push_back(input->nextToken());
  }
}

Parser::  ~Parser() {
}

void Parser::consume(void) {
  lookahead[c_index] = input->nextToken();
  c_index = (c_index + 1) % n_lookahead;
}

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
