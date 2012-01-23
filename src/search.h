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
#include "pi.h"
#include "pbtk/pbinput.h"
#include "pbtk/pbdialog.h"
#include "pbtk/pbcombobox.h"
#include "math.h"

class SearchDialog : public PBDialog {
  PBTextDisplay *text;
  PBLabel lb_find;
  PBLabel lb_repl;
  PBComboBox inp_find;
  PBInput inp_repl;
  PBButton bt_repl;
  PBButton bt_find;
  PBButton bt_close;
  PBCheckButton bt_back;

  void onFind(PBWidget*);
  void onRepl(PBWidget*);
  void onClose(PBWidget*);
  void onBackward(PBWidget*);
  int start_pos;
  bool backward;
  public:
  SearchDialog(const std::string& nm,PBTextDisplay* lb,int from=0,const bool replace=false);
  void placeWidgets();
  void searchFrom(int f){start_pos=f;}
};

