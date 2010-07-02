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

#include "codestream.hh"

CodeStreamStream::CodeStreamStream(std::istream* is)
: m_is(is) {
  m_cursor.cur_line = 1;
  m_cursor.cur_column = 1;
  m_ch = m_is->get();
}

char CodeStreamStream::getchar(void) const {
  return m_ch;
}

void CodeStreamStream::consume(void) {
  if (!m_is->eof()) {
    if (m_ch == '\n') {
      m_cursor.cur_line++;
      m_cursor.cur_column = 1;
    } else {
      m_cursor.cur_column++;
    }

    m_ch = m_is->get();
  }
}

const Cursor& CodeStreamStream::cursor(void) const {
  return m_cursor;
}

/* vim: set et ts=2 sw=2 cin: */