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

#ifndef LEXER_HXX_
#define LEXER_HXX_

#include <string>
#include "token.hxx"

class Lexer {
public:
  Lexer(const std::string& input);

  char curChar(void) const;
  void consume(void);
  void match(char x);

  virtual Token nextToken(void) = 0;
  virtual const char* getTokenName(int tokenType) const = 0;

private:
  const std::string _input;
  const size_t _length;
  size_t _index;
};

#endif
