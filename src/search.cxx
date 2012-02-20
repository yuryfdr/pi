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
#undef IVSAPP
#include "search.h"
#include <functional>

#ifdef __EMU__
#define KBDOPTS KBD_SCREENTOP
#else
#define KBDOPTS 0
#endif

struct item_finder:public std::unary_function < bool, PBListBoxItem * > {
  const std::string what;
  bool cs;
  item_finder(const std::string & s, bool c):what(s), cs(c) {
  } bool operator() (PBListBoxItem * item) {
    if (cs) {
      if (utfcasestr(item->getText().c_str(), what.c_str()))
        return true;
    } else if (item->getText().find(what) != std::string::npos)
      return true;
    return false;
  }
};

void SearchDialog::onFind(PBWidget *)
{
  int in = -1;
  bool casesens = bt_sens.checked();
  if (backward)
    in = text->findItemRev(start_pos, item_finder(inp_find.getText(), casesens));
  else
    in = text->findItem(start_pos, item_finder(inp_find.getText(), casesens));
  if (in >= 0) {
    start_pos = (backward) ? (in - 1) : (in + 1);
    text->selectItem(in);
    PBListBoxItem *itm = text->getItem(in);
    int SH = ScreenHeight();
    text->update();
    if (itm->y() > SH / 2) {
      y(50);
    } else {
      y(SH / 2 + 50);
    }
  } else {
    //start_pos=0;
    Message(ICON_INFORMATION, "Not Found", "Text not found", 6000);
  }
  update();
}

void SearchDialog::onRepl(PBWidget *)
{
}

void SearchDialog::onBackward(PBWidget *)
{
  backward = bt_back.checked();
//  printf("bw %d\n",backward);
}

void SearchDialog::onClose(PBWidget *)
{
  quit(false);
}

SearchDialog::SearchDialog(const std::string & nm, PBTextDisplay * lb, int from,
                           const bool replace)
                           :PBDialog(nm), text(lb), lb_find("Search:", this), lb_repl("Replace", this), 
                           inp_find("", this), inp_repl("", this), bt_repl("Replace", this),
                           bt_find(((replace) ? "Replace" : "Find"), this), bt_close("Close", this),
                           bt_back("Backward", this), bt_sens("Ignore Case",this), 
                           start_pos(from), backward(false)
{
  addWidget(&lb_find);
  addWidget(&inp_find);
  if (replace) {
    addWidget(&lb_repl);
    addWidget(&inp_repl);
    addWidget(&bt_repl);
  }
  addWidget(&bt_back);
  addWidget(&bt_sens);
  addWidget(&bt_find);
  addWidget(&bt_close);
  bt_find.onPress.connect(sigc::mem_fun(this, &SearchDialog::onFind));
  bt_close.onPress.connect(sigc::mem_fun(this, &SearchDialog::onClose));
  bt_back.onPress.connect(sigc::mem_fun(this, &SearchDialog::onBackward));
  y(50);
}

void SearchDialog::placeWidgets()
{
  int SW = ScreenWidth();
  int up = y();
  setSize(10, up, SW - 20, 130 + captionHeight());
  up += 10 + captionHeight();
  lb_find.setSize(20, up, 70, 30);
  inp_find.setSize(100, up, SW - 130, 30);
  up += 40;
  bt_back.setSize(20, up, w() / 2 - 20, 30);
  bt_sens.setSize(x() + w() / 2, up, w() / 2 - 20, 30);
  //lb_repl.setSize(20,up,70,30);
  //inp_repl.setSize(100,up,SW-110,30);
  up += 40;
  bt_find.setSize(20, up, (SW - 70) / 3, 30);
  bt_repl.setSize(30 + (SW - 70) / 3, up, (SW - 70) / 3, 30);
  bt_close.setSize(40 + 2 * (SW - 70) / 3, up, (SW - 70) / 3, 30);
}
