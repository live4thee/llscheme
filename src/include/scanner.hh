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

#ifndef SCANNER_HH_
#define SCANNER_HH_

#include <string>
#include "token.hh"

class Scanner {
public:
  virtual ~Scanner() {};

  virtual char curChar(void) const = 0;
  virtual void consume(void) = 0;
  virtual void match(char x) = 0;
};

class StringScanner: public Scanner {
public:
  StringScanner(const std::string& input);

  char curChar(void) const;
  void consume(void);
  void match(char x);

private:
  const std::string _input;
  const size_t _length;
  size_t _index;
};

#endif
