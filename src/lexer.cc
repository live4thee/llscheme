// Copyright (C) 2010 David Lee <live4thee@gmail.com>,
//                    Qing He <qing.x.he@gmail.com>
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

#include <algorithm>
#include <iostream>

#include "lexer.hh"
#include "error.hh"

Lexer::Lexer(const std::string& input):
  Scanner(input) {}

const char* Lexer::getTokenName(int tokenType) const {
  return tokenNames[tokenType];
}


Token Lexer::nextToken(void) {
  char ch;
  while ((ch = curChar()) != -1) {
    switch (ch) {
    case ' ': case '\t': case '\n': case '\r':
      consume(); continue;
    case ',':
      consume(); return Token(COMMA, ",");
    case '(':
      consume(); return Token(LPAREN, "(");
    case ')':
      consume(); return Token(RPAREN, ")");
    case '+':
    case '-':
      return getPeculiarIdentifier();
    default:
      if (isDigit()) return getSimpleNumber();
      if (isInitial()) return getIdentifier();
      throw Error(std::string("invalid character: ") + ch);
    }
  }

  return Token(EOF_TYPE, "<EOF>");
}

Token Lexer::getIdentifier(void) {
  std::string id = "";
  do {
    id += curChar();
    consume();
  } while (isSubsequent());

  return Token(ID, id);
}

Token Lexer::getPeculiarIdentifier(void) {
  std::string id = "";
  const char ch = curChar();
  
  // N.B. r5rs states that peculiar identifier -> + | - | ...
  // does ... means literal "..." ?? If yes, need to add
  id += ch;
  consume();
  if (!isDelimiter())
    throw Error(std::string("invalid identifier start with ") + ch);

  return Token(ID, id);
}

Token Lexer::getSimpleNumber(void) {
  std::string number = "";
  
  do {
    number += curChar();
    consume();
  } while (isDigit());
  
  return Token(NUMBER, number);
}

bool Lexer::isLetter(void) {
  const char ch = curChar();
  return ((ch >= 'a' && ch <= 'z') ||
	  (ch >= 'A' && ch <= 'Z'));
}

bool Lexer::isSpecialInitial(void) {
  const char ch = curChar();
  static const char special[] = {
    '!', '$', '%', '&', '*', '/', ':', '<', '=',
    '>', '?', '^', '_', '~'
  };
  const int N = sizeof(special) / sizeof(char);
  const char *s;

  s = std::find(special, special + N, ch);
  return (s != special + N);
}

bool Lexer::isInitial(void) {
  return isLetter() || isSpecialInitial();
}

bool Lexer::isSubsequent(void) {
  const char ch = curChar();

  if (ch >= '0' && ch <= '9')
    return true;
  else if (ch == '+' || ch == '-' || ch == '.' || ch == '@')
    return true;

  return isInitial();
}

bool Lexer::isDelimiter(void) {
  const char ch = curChar();
  static const char delimiters[] = {
    ' ', '\t', '\r', '\n', '(', ')', '"', ';'
  };
  const int N = sizeof(delimiters) / sizeof(char);
  const char *s;

  s = std::find(delimiters, delimiters + N, ch);
  return (s != delimiters + N);
}

bool Lexer::isDigit(int radix) {
  const char ch = curChar();

  return (ch >= '0' && ch <= '9');
}

const char* Lexer::tokenNames[] = {
  "n/a", "<EOF>", "LPAREN", "ID", "NUMBER", "COMMA", "RPAREN"
};
