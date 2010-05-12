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

#include "scanner.hh"
#include "error.hh"

StringScanner::StringScanner(const std::string& input):
  _input(input), _length(input.length()), _index(0) {}

char StringScanner::curChar(void) const {
  if (_index < _length)
    return _input.at(_index);
  return -1;
}

void StringScanner::consume(void) {
  _index++;
}

void StringScanner::match(char x) {
  const char ch = curChar();
  if (x == ch) consume();
  else throw Error(std::string("expecting ") + x + "; found " + ch);
}
