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

#include "env.hh"
#include "error.hh"

static void clearBinding(Binding *b) {
  Binding::iterator i;

/*
  for (i = b->begin(); i != b->end(); i++) {
    llvm::Value *val = i->second;
    delete val;
    val = NULL;
  }
*/
  b->clear();
}


ExecutionEnv::ExecutionEnv() {
  Binding *tmp;

  global = new Binding;
  tmp = new Binding;
  lexical.push(tmp);
}

ExecutionEnv::~ExecutionEnv() {
  while (!lexical.empty()) {
    Binding *b = lexical.top();
    clearBinding(b);
    lexical.pop();
    delete b;
  }

  delete global;
}

void ExecutionEnv::addBinding(const std::string &name, llvm::Value *val) {
  lexical.top()->insert(std::pair<const std::string, llvm::Value *>(name, val));
}

void ExecutionEnv::addGlobalBinding(const std::string &name, llvm::Value *val) {
  global->insert(std::pair<const std::string, llvm::Value *>(name, val));
}

void ExecutionEnv::newScope() {
  lexical.push(new Binding);
}

void ExecutionEnv::lastScope() {
  if (lexical.empty())
    throw Error(std::string("Scope error"));

  Binding *b = lexical.top();

  clearBinding(b);
  lexical.pop();
  delete b;
}

llvm::Value *ExecutionEnv::searchBinding(const std::string &name) {
  Binding::iterator i;

  i = lexical.top()->find(name);
  if (i != lexical.top()->end())
    return i->second;

  i = global->find(name);
  if (i != global->end())
    return i->second;

  return NULL;
}

llvm::Value *ExecutionEnv::searchCurrentScopeBinding(const std::string &name) {
  Binding::iterator i;

  i = lexical.top()->find(name);
  if (i != lexical.top()->end())
    return i->second;

  return NULL;
}

Binding *ExecutionEnv::getGlobalBinding() {
  return global;
}

Binding *ExecutionEnv::getCurrentScopeBinding() {
  return lexical.top();
}
