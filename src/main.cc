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
#include <memory>
#include <cstdlib>

#include <unistd.h>

#include "parser.hh"
#include "astnodes.hh"
#include "driver.hh"

#define PROGRAM_NAME "llscheme"

static bool printTrace = false;
static bool dumpASTNodes = false;

static void usage(int exit_status)
{
  std::cerr << "usage: " PROGRAM_NAME "[ OPTION ] \n"
            << " '-d': just dump the AST nodes and exit\n"
            << " '-p': print trace information when running executable\n"
            << std::endl;
  std::exit(exit_status);
}

int main(int argc, char *argv[])
{
  int opt = -1;
  while ((opt = getopt(argc, argv, "dp")) != -1) {
    switch (opt) {
    case 'd': dumpASTNodes = true; break;
    case 'p': printTrace = true; break;
    default:
      usage(EXIT_FAILURE);
    }
  }

  std::auto_ptr<CodeStream> cs(new CodeStreamStream(&std::cin));
  std::auto_ptr<Lexer> lexer(new Lexer(cs.get()));
  Parser parser(lexer.get(), 4);

  std::auto_ptr<SExprASTNode> ast(new SExprASTNode());
  ast->addArgument(new SymbolASTNode("begin"));

  // parser a sexp
  while (parser.peekTokenType(1) != Lexer::EOF_TYPE) {
    if (printTrace) {
      SExprASTNode *ast2 = new SExprASTNode();
      ast2->addArgument(new SymbolASTNode("display"));
      ast2->addArgument(parser.exp());
      ast->addArgument(ast2);
    }
    else
      ast->addArgument(parser.exp());
  }

  std::stringstream ss;
  if (dumpASTNodes) {
    ast->finePrint(ss);
    std::cerr << ss.rdbuf() << "\n";
  } else {
    // dump to stdout
    codegen(ast.get(), STDOUT_FILENO);
  }

  return 0;
}

/* vim: set et ts=2 sw=2 cin: */
