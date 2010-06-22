// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
//
// Copyright (C)
//         2010 Qing He <qing.x.he@gmail.com>
//         2010 David Lee <live4thee@gmail.com>
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
#include <deque>

#include <llvm/Value.h>

typedef std::map<const std::string, llvm::Value *> Binding;

class ExecutionEnv {
  Binding *global;
  std::deque<Binding *> lexical;
  std::deque<Binding *> refs;
public:
  ExecutionEnv();
  virtual ~ExecutionEnv();

  void addBinding(const std::string &name, llvm::Value *val);
  void addGlobalBinding(const std::string &name, llvm::Value *val);

  void newScope();
  void lastScope();

  llvm::Value *searchBinding(const std::string &name);
  llvm::Value *searchGlobalBinding(const std::string &name);
  llvm::Value *searchLocalBinding(const std::string &name);

  Binding *getCurrentRefs();
};

#endif

/* vim: set et ts=2 sw=2 cin: */
