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

#include <iostream>
#include <sstream>
#include "parser.hh"

int main(void)
{
  std::stringstream ss;
  ss << std::cin.rdbuf();

  Scanner *scanner = new StringScanner(ss.str());
  Lexer *lexer = new Lexer(scanner);
  Parser parser = Parser(lexer, 4);

#if 0
  Token token = lexer->nextToken();

  while (token.type != EOF_TYPE) {
    std::cout << token.type << ", " << token.text << std::endl;
    token = lexer->nextToken();
  }
#endif

  // parser a sexp
  while (parser.peekTokenType(1) != EOF_TYPE) {
    parser.form();
  }

  delete lexer;
  delete scanner;

  return 0;
}
