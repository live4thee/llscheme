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
#include "driver.hh"

#include <llvm/Instructions.h>

static void clearBinding(Binding *b) {
  b->clear();
}


ExecutionEnv::ExecutionEnv() {
  Binding *tmp;

  global = new Binding;
  tmp = new Binding;
  lexical.push_back(tmp);
  tmp = new Binding;
  refs.push_back(tmp);
}

ExecutionEnv::~ExecutionEnv() {
  while (!lexical.empty()) {
    Binding *b = lexical.back();
    clearBinding(b);
    lexical.pop_back();
    delete b;
    b = refs.back();
    clearBinding(b);
    refs.pop_back();
    delete b;
  }

  delete global;
}

void ExecutionEnv::addBinding(const std::string &name, llvm::Value *val) {
  lexical.back()->insert(std::pair<const std::string, llvm::Value *> (name, val));
}

void ExecutionEnv::addGlobalBinding(const std::string &name, llvm::Value *val) {
  global->insert(std::pair<const std::string, llvm::Value *>(name, val));
}

void ExecutionEnv::newScope() {
  lexical.push_back(new Binding);
  refs.push_back(new Binding);
}

void ExecutionEnv::lastScope() {
  if (lexical.empty())
    throw Error(std::string("Scope error"));

  Binding *b = lexical.back();

  clearBinding(b);
  lexical.pop_back();
  delete b;
  b = refs.back();
  clearBinding(b);
  refs.pop_back();
  delete b;
}

llvm::Value *ExecutionEnv::searchBinding(const std::string &name) {
  Binding::iterator i;
  llvm::Value *val;

  val = searchLocalBinding(name);
  if (val)
    return val;

  val = searchGlobalBinding(name);
  if (val)
    return val;

  return NULL;
}

llvm::Value *ExecutionEnv::searchGlobalBinding(const std::string &name) {
  Binding::iterator i;

  i = global->find(name);
  if (i != global->end())
    return i->second;

  return NULL;
}

llvm::Value *ExecutionEnv::searchLocalBinding(const std::string &name) {
  Binding::iterator i;
  std::deque<Binding *>::reverse_iterator p, q;
  Binding *b;
  llvm::Value *val;

  for(p = lexical.rbegin(), q = refs.rbegin();
      p != lexical.rend(); p++, q++) {
    b = *p;
    i = b->find(name);
    if (i != b->end()) {
      if (p != lexical.rbegin())
        goto makeref;
      else
        return i->second;
    }
  }

  return NULL;

 makeref:
  // we have a match, but not in our scope...
  // make a temporary value (and not insert) and insert it in the current
  // scope, it will later be replaced with actual load instruction, and we
  // need to take care of the temporary alloca there
  // the `refs' is a ref to the upper level, which is used by the code
  // generator to construct free[] list in the function closure
  val = i->second;
  do {
    p--; q--;
    b = *q;
    b->insert(std::pair<const std::string, llvm::Value *> (name, val));

    val = new llvm::AllocaInst(LSObjType->getPointerTo());
    b = *p;
    b->insert(std::pair<const std::string, llvm::Value *> (name, val));
  } while(p != lexical.rbegin());

  return val;
}

Binding *ExecutionEnv::getCurrentRefs() {
  return refs.back();
}

