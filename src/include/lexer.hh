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

#ifndef LEXER_HH_
#define LEXER_HH_

#include <string>
#include "scanner.hh"

enum tokenType {EOF_TYPE = 1, LPAREN, ID, NUMBER, COMMA, RPAREN};

class Lexer {
public:
  Lexer(Scanner *s) { scanner = s; }

  const char* getTokenName(int tokenType) const;
  Token nextToken(void);

private:
  Token getIdentifier(void);
  Token getPeculiarIdentifier(void);
  Token getSimpleNumber(void);
  bool isLetter(void);
  bool isInitial(void);
  bool isSpecialInitial(void);
  bool isSubsequent(void);
  bool isDelimiter(void);
  bool isDigit(int radix = 10);

  static const char* tokenNames[];

  Scanner *scanner;
  char curChar(void) { return scanner->curChar(); }
  void consume(void) { return scanner->consume(); }

};

#endif
