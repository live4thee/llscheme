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
#include <sstream>
#include "parser.hh"
#include "ast2.hh"

extern int codegen(ASTNode *);

int main(int argc, char *argv[])
{
  std::stringstream ss, ss1;
  ss << std::cin.rdbuf();
  int display = 0;

  Scanner *scanner = new StringScanner(ss.str());
  Lexer *lexer = new Lexer(scanner);
  Parser parser = Parser(lexer, 4);
  SExprASTNode *ast = new SExprASTNode();

#if 0
  Token token = lexer->nextToken();

  while (token.type != EOF_TYPE) {
    std::cout << token.type << ", " << token.text << std::endl;
    token = lexer->nextToken();
  }
#endif

  if (argc == 2 && std::string(argv[1]) == "-p")
    display = 1;

  ast->addArgument(new SymbolASTNode("begin"));

  // parser a sexp
  while (parser.peekTokenType(1) != EOF_TYPE) {
    SExprASTNode *ast2;
    if (display) {
      ast2 = new SExprASTNode();
      ast2->addArgument(new SymbolASTNode("display"));
      ast2->addArgument(parser.exp());
      ast->addArgument(ast2);
    }
    else
      ast->addArgument(parser.exp());
  }

  if (argc == 2 && std::string(argv[1]) == "-d") {
    ast->finePrint(ss1);
    std::cout << ss1.rdbuf() << "\n";
  } else {
    codegen(ast);
  }

  delete ast;
  delete lexer;
  delete scanner;

  return 0;
}
