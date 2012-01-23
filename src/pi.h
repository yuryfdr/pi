/* Copyright (C) 2011 Yury P. Fedorchenko (yuryfdr at users.sf.net)  */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef ___PI_H___
#define ___PI_H___
#undef IVSAPP
#include "inkview.h"

#include <iostream>
#include <functional>
#include <sstream>
#include "pbtk/pblistbox.h"
#include "pbtk/convert.h"
#include "math.h"

#ifdef __EMU__
#define KBDOPTS KBD_SCREENTOP
#else
#define KBDOPTS 0
#endif

void keyboard_entry(char *s);

struct undoop{
  enum method{EDIT=0,INSERT,DELETE}op;
  int pos;
  std::string str;
};


class PBTextDisplay : public PBListBox{
  public:
  bool changed,sh_ln;
  undoop curop;
  std::vector<undoop> undo;
  static PBListBoxItem* item;
  static char* buff;
  static int buff_size;
  std::string copy_buf;
  PBTextDisplay(const std::string& str,PBWidget* par);
  void updateNumbers();
  void draw(){
    PBListBox::draw();
  }
  int getPos(PBListBoxItem* itm);
  void editItem(bool ins=false);
  void undoOp();
  int handle(int type, int par1, int par2);
  struct encoder : public std::unary_function<void,PBListBoxItem*>{
    const unsigned short* enc;
    encoder(const std::string& _enc){
      if(_enc=="cp1251")enc=cp1251_to_unicode;
      else if(_enc=="cp866")enc=cp866_to_unicode;
      else enc=koi8_to_unicode;
    };
    void operator ()(PBListBoxItem* itm){
      itm->setText(to_utf8(itm->getText().c_str(),enc));
    };
  };
  struct decoder : public std::unary_function<void,PBListBoxItem*>{
    const unsigned short* enc;
    decoder(const std::string& _enc){
      if(_enc=="cp1251")enc=cp1251_to_unicode;
      else if(_enc=="cp866")enc=cp866_to_unicode;
      else enc=koi8_to_unicode;
    };
    void operator ()(PBListBoxItem* itm){
      itm->setText(utf8_to(itm->getText().c_str(),enc));
    };
  };
  void convert(const char* enc){
    encoder a(enc);
    forEachItem(a);
    update();
  }
  void decode(const char* enc){
    decoder a(enc);
    forEachItem(a);
    update();
  }
};

enum {
  ITM_OPEN,
  ITM_SAVE,
  ITM_ABOUT,
  ITM_EXIT,
  ITM_EDIT,
  ITM_COPY,
  ITM_INSB,
  ITM_INSA,
  ITM_DELE,
  ITM_NEWF,
  ITM_GOTO,
  ITM_SHNO,
  ITM_FONT,
  ITM_ORIE,
  ITM_RESN,
  ITM_UNDO,
  ITM_RSNT0,
  ITM_RSNT1,
  ITM_RSNT2,
  ITM_RSNT3,
  ITM_RSNT4,
  ITM_RSNT5,
  ITM_RSNT6,
  ITM_RSNT7,
  ITM_RSNT8,
  ITM_RSNT9,
  ITM_FIND,
  ITM_REPL,
  ITM_SUBM,
  ITM_1251,
  ITM_866,
  ITM_KOI8,
  ITM_1251T,
  ITM_866T,
  ITM_KOI8T
};

void SaveCFG();

#endif
