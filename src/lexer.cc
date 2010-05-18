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
#include <sstream>
#include <cctype>

#include "lexer.hh"
#include "error.hh"

// TODO:
//   * customizable configs: strict standards or allow extension.
//     e.g. identifiers '123abc' '..@' allowed?
//   * detach public and private interfaces so updating the private
//     part doesn't cause a global recompiliation
//   * review how <delimiters> works
//   * scanner read ahead and better error message
//   * more readable lexer?

const char* Lexer::getTokenName(int tokenType) const {
  return tokenNames[tokenType];
}

Token Lexer::nextToken(void) {
  char ch;
  while ((ch = curChar()) != -1) {
    switch (ch) {
    case ' ': case '\t': case '\n': case '\r':
      consume(); continue;
    case '(':
      consume(); return Token(LPAREN, "(");
    case ')':
      consume(); return Token(RPAREN, ")");
    case '\'':
      consume(); return Token(QUOTE, "'");
    case '+':
    case '-':
      return getPeculiarIdentifier();
    case '.':
      return getDot();
    case ';':
      skipComment(); continue;
    case '"':
      return getString();
    case '#':
      return getPoundSpecial();
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
  if (!isDelimiter())
    throw Error(std::string("invalid identifier syntax ") + curChar());

  return Token(ID, id);
}

Token Lexer::getPeculiarIdentifier(void) {
  std::string id = "";
  const char ch = curChar();

  id += ch;
  consume();
  if (!isDelimiter())
    throw Error(std::string("invalid identifier start with ") + ch);

  return Token(ID, id);
}

Token Lexer::getSimpleNumber(void) {
  std::string number = "";

  // TODO: handling real number with '.'
  do {
    number += curChar();
    consume();
  } while (isDigit());

  if (!isDelimiter())
    throw Error(std::string("invalid number ") + curChar());

  return Token(NUMBER, number);
}

Token Lexer::getDot(void) {
  consume();
  if (isDelimiter())
    return Token(PERIOD, ".");

  if (curChar() == '.') {
    consume();
    if (curChar() == '.') {
      consume();
      if (isDelimiter())
        return Token(ID, "...");
    }
  }

  throw Error(std::string("invalid token starting with ."));
}

void Lexer::skipComment(void) {
  do {
    consume();
  } while(curChar() != '\r' && curChar() != '\n');
  consume();
}

Token Lexer::getString(void) {
  std::string str = "";

  // TODO: multi line string, \"
  consume();
  while (curChar() != '"') {
    str += curChar();
    consume();
  }
  consume();

  return Token(STRING, str);
}

Token Lexer::getPoundSpecial()
{
  std::string special = "#";

  consume();
  switch(curChar()) {
  case 't':
  case 'f':
    special += curChar();
    consume();
    if (!isDelimiter())
      throw Error(std::string("invalid special # " + curChar()));
    return Token(BOOL, special);
  default:
    throw Error(std::string("invalid special #" + curChar()));
  }
}

bool Lexer::isLetter(void) {
  const char ch = curChar();
  return std::isalpha(ch);
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

  if (std::isdigit(ch))
    return true;
  else if (ch == '+' || ch == '-' || ch == '.' || ch == '@')
    return true;

  return isInitial();
}

bool Lexer::isDelimiter(void) {
  const char ch = curChar();
  static const char delimiters[] = {
    '\t', '\n', '\r', ' ', '"', '(', ')', ';'
  };
  const int N = sizeof(delimiters) / sizeof(char);
  const char *s;

  s = std::find(delimiters, delimiters + N, ch);
  return (s != delimiters + N);
}

bool Lexer::isDigit(radixType rdx) {
  const char ch = curChar();

  switch (rdx) {
  case RDX_DEC: return std::isdigit(ch);
  case RDX_HEX: return std::isxdigit(ch);
  }

  std::stringstream ss;
  ss << rdx;
  throw Error(std::string("invalid radix: ") + ss.str());
}

const char* Lexer::tokenNames[] = {
  "n/a", "<EOF>", "LPAREN", "ID", "NUMBER", "RPAREN",
  "PERIOD", "STRING", "BOOL", "QUOTE"
};
