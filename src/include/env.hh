// Copyright (C) 2010 Qing He <qing.x.he@gmail.com>
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

#ifndef ENV_HH
#define ENV_HH

#include <string>
#include <map>
#include <stack>
#include "ast2.hh"

typedef std::map<const std::string, ASTNode *> Binding;

class ExecutionEnv {
  Binding *global;
  std::stack<Binding *> lexical;
public:
  ExecutionEnv();
  virtual ~ExecutionEnv();

  void addBinding(const std::string &name, ASTNode *val);
  void addGlobalBinding(const std::string &name, ASTNode *val);

  void newScope();
  void oldScope();

  ASTNode *searchBinding(const std::string &name);
};
#endif

// Local Variables:
// c-basic-offset: 2
// End: