/* Copyright (C) 2011-2012 Yury P. Fedorchenko (yuryfdr at users.sf.net)  */
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
#include <fstream>
#include "pbtk/pbfilechooser.h"
#include "pbtk/convert.h"
#include "pi.h"
#include "search.h"
#include "pref.h"
#include <math.h>

PBTextDisplay::PBTextDisplay(const std::string & str, PBWidget * par):PBListBox(str, par),
changed(false), sh_ln(false)
{
}

void PBTextDisplay::updateNumbers()
{
  int total = _items.size();
  char str[total + 1];
  char format[32];
  sprintf(format, "%%%dd", int (log10(total) + 1));
  //printf("format '%s' %d\n ",format,total);
  for (lbitem_it it = _items.begin(); it != _items.end(); ++it) {
    if (sh_ln) {
      sprintf(str, format, (it - _items.begin()) + 1);
      (*it)->setMarker(str);
    } else
      (*it)->setMarker("");
  }
  update(true);
}

int PBTextDisplay::getPos(PBListBoxItem * itm)
{
  lbitem_it it = std::find(_items.begin(), _items.end(), itm);
  if (it == _items.end())
    return -1;
  return it - _items.begin();
}

void PBTextDisplay::editItem(bool ins)
{
  if (!item)
    return;
  std::string istr;
  if (ins)
    istr = copy_buf;
  else
    istr = item->getText();
  curop.str = istr;
  curop.op = undoop::EDIT;
  curop.pos = getPos(item);
  buff = new char[istr.size() + 512];
  buff_size = istr.size() + 512;
  strcpy(buff, istr.c_str());
  changed = true;
  OpenKeyboard("Edit", buff, buff_size, 0 | KBDOPTS, keyboard_entry);
}

void PBTextDisplay::undoOp(){
  if(undo.empty()){
    Message(ICON_ERROR,"Undo","Nothing to undo",60000);
    return;
  }
  int repeat;
  undoop p = *(undo.rbegin());
  switch(p.op){
    case undoop::EDIT:
      _items[p.pos]->setText(p.str);
      item=_items[p.pos];
    break;
    case undoop::INSERT:
      item=erase(_items[p.pos]);
    break;
    case undoop::DELETE:
      item=insertBefore(_items[p.pos],p.str);
    break;
    case undoop::INSERT_SUBTREE:
      repeat=p.repeat;
      for(int i=0;i<repeat;i++){
        undo.resize(undo.size()-1);
        p = *(undo.rbegin());

        item=erase(_items[p.pos]);
      }
    break;
    case undoop::EDIT_SUBTREE:
      repeat=p.repeat;
      for(int i=0;i<repeat;i++){
        undo.resize(undo.size()-1);
        p = *(undo.rbegin());

        item=_items[p.pos];
        item->setText(p.str);
      }
    break;
    case undoop::DELETE_SUBTREE:
      repeat=p.repeat;
      for(int i=0;i<repeat;i++){
        undo.resize(undo.size()-1);
        p = *(undo.rbegin());

        item=insertBefore(_items[p.pos],p.str);
      }
    break;
    case undoop::MOVE_SUBTREE:
      undo.resize(undo.size()-1);
      undoOp();
      undoOp();
      break;
  }
  selectItem(item);
  if(p.op!=undoop::MOVE_SUBTREE)
    undo.resize(undo.size()-1);

  if(undo.empty() && changed)changed=false;
  else changed=true;
  updateNumbers();
  update();
}

int PBTextDisplay::handle(int type, int par1, int par2)
{
  int ret = PBListBox::handle(type, par1, par2);
  if (!ret && (type == EVT_KEYUP || type == EVT_KEYDOWN)) {
    switch (par1) {
    case KEY_OK:{
        item = (PBListBoxItem *) getFocusedWidget();
        if (item == 0 || (item->y() + item->h() > y() + h()) || item->y() < y()) {
          return 0;
        }
        if (type == EVT_KEYUP) {
          editItem();
          return 1;
        }
      }
    }
  }
  if (!ret && ( /*type == EVT_POINTERDOWN || */ type == EVT_POINTERUP)
      && eventInside(par1, par2)) {
    for (lbitem_cit it = _items.begin(); it < _items.end(); ++it) {
      if ((*it)->eventInside(par1, par2)) {
        if ((*it)->canBeFocused()) {
          (*it)->setFocused(true);
          update();
          item = (*it);
          if (type == EVT_POINTERUP) {
            editItem();
          }
          return 1;
        }
      }
    }
  }
  return 0;
}

PBListBoxItem *PBTextDisplay::item = NULL;
char *PBTextDisplay::buff = NULL;
int PBTextDisplay::buff_size = 0;

imenu rsntFile[] = {
  {0, ITM_RSNT0, NULL, NULL},
  {0, ITM_RSNT1, NULL, NULL},
  {0, ITM_RSNT2, NULL, NULL},
  {0, ITM_RSNT3, NULL, NULL},
  {0, ITM_RSNT4, NULL, NULL},
  {0, ITM_RSNT5, NULL, NULL},
  {0, ITM_RSNT6, NULL, NULL},
  {0, ITM_RSNT7, NULL, NULL},
  {0, ITM_RSNT8, NULL, NULL},
  {0, ITM_RSNT9, NULL, NULL},
  {0, 0, NULL, NULL}
};

imenu encMenu[] = {
  {ITEM_ACTIVE, ITM_1251, "cp1251", NULL},
  {ITEM_ACTIVE, ITM_866, "cp866", NULL},
  {ITEM_ACTIVE, ITM_KOI8, "koi8-r", NULL},
  {0, 0, NULL, NULL}
};

imenu decMenu[] = {
  {ITEM_ACTIVE, ITM_1251T, "cp1251", NULL},
  {ITEM_ACTIVE, ITM_866T, "cp866", NULL},
  {ITEM_ACTIVE, ITM_KOI8T, "koi8-r", NULL},
  {0, 0, NULL, NULL}
};

imenu fileMenu[] = {
  {ITEM_ACTIVE, ITM_NEWF, "New", NULL},
  {ITEM_ACTIVE, ITM_OPEN, "Open", NULL},
  {ITEM_ACTIVE, ITM_SAVE, "Save", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_HIDDEN, ITM_RESN, "Last Opened", rsntFile},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_SUBMENU, ITM_SUBM, "Convert from", encMenu},
  {ITEM_SUBMENU, ITM_SUBM, "Convert to", decMenu},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_ABOUT, "About", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_EXIT, "Quit", NULL},
  {0, 0, NULL, NULL}
};

imenu optMenu[] = {
  {ITEM_ACTIVE, ITM_SHNO, "Show line #", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_OPLF, "Open Last File On Run", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_FONT, "Select Font", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_ORIE, "Orientation", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_OUT_ENABLE, "Enable Outline Mode", NULL},//outline options
  {ITEM_ACTIVE, ITM_INDENT2, "Indent 2", NULL},//outline options
  {ITEM_ACTIVE, ITM_INDENT4, "Indent 4", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_KEYS, "Key Bindings", NULL},
  {0, 0, NULL, NULL}
};

imenu searchMenu[] = {
  {ITEM_ACTIVE, ITM_GOTO, "Go to line", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_ACTIVE, ITM_FIND, "Find", NULL},
//  { ITEM_ACTIVE, ITM_REPL, "Replace", NULL },
  {0, 0, NULL, NULL}
};
imenu editMenu[] = {
  {ITEM_ACTIVE, ITM_COPY, "Copy Line", NULL},
  {ITEM_ACTIVE, ITM_INSB, "Insert Before", NULL},
  {ITEM_ACTIVE, ITM_INSA, "Insert After", NULL},
  {ITEM_ACTIVE, ITM_IMPORT, "Insert File After", NULL},
  {ITEM_ACTIVE, ITM_DELE, "Delete", NULL},
  {0, 0, NULL, NULL}
};
extern imenu outlineMenu[];
imenu mainMenu[] = {
  {ITEM_HEADER, 0, "π", NULL},
  {ITEM_ACTIVE, ITM_EDIT, "Edit Line", NULL},
  {ITEM_ACTIVE, ITM_NEWF, "New Line", NULL},
  {ITEM_SUBMENU, 0, "Edit", editMenu},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_SUBMENU, 0, "Edit Outline", outlineMenu },
  {ITEM_ACTIVE, ITM_UNDO, "Undo", NULL},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_SUBMENU, 0, "File", fileMenu},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_SUBMENU, 0, "Search", searchMenu},
  {ITEM_SEPARATOR, 0, NULL, NULL},
  {ITEM_SUBMENU, 0, "Options", optMenu},
  {0, 0, NULL, NULL}
};
void MainScreen::updateMenu()
{
  if (text.item) {
    mainMenu[1].type = ITEM_ACTIVE;
    mainMenu[2].type = ITEM_HIDDEN;
    mainMenu[3].type = ITEM_SUBMENU;
    mainMenu[4].type = ITEM_SEPARATOR;
    mainMenu[5].type = enable_outline ? ITEM_SUBMENU : ITEM_HIDDEN;
//    mainMenu[6].type = ITEM_ACTIVE;
  } else {
    mainMenu[1].type = ITEM_HIDDEN;
    mainMenu[2].type = ITEM_ACTIVE;
    mainMenu[3].type = ITEM_HIDDEN;
    mainMenu[4].type = ITEM_HIDDEN;
    mainMenu[5].type = ITEM_HIDDEN;
//    mainMenu[6].type = ITEM_HIDDEN;
  }
  if(indentation==2){
    optMenu[9].type=ITEM_BULLET;  
    optMenu[10].type=ITEM_ACTIVE;  
  }else{
    optMenu[9].type=ITEM_ACTIVE;  
    optMenu[10].type=ITEM_BULLET;  
  }
  optMenu[2].type = (open_last) ? ITEM_BULLET : ITEM_ACTIVE;
  optMenu[8].type = (enable_outline) ? ITEM_BULLET : ITEM_ACTIVE;
  mainMenu[6].type = (text.undo.empty())?ITEM_HIDDEN:ITEM_ACTIVE;
}

void MainScreen::exit_dlg_h(int bt)
{
  if (bt == 1) {
    SaveCFG();
    CloseApp();
  }
}

void MainScreen::fsimport_hndl(int isok, PBFileChooser * dlg)
{
  if (isok) {
    std::string s = dlg->getPath();
    ms->openFile(s.c_str(),true);
    ms->text.updateNumbers();
  }
}
void MainScreen::fopen_hndl(int isok, PBFileChooser * dlg)
{
  if (isok) {
    std::string s = dlg->getPath();
    if (ms->openFile(s.c_str()))
      addRecent(s.c_str());
    ms->text.updateNumbers();
  }
}

void MainScreen::addRecent(const char *s)
{
  for (int i = 0; i < 9; ++i) {
    if (rsntFile[i].text && strcmp(rsntFile[i].text, s) == 0) {
      if (i != 0)
        std::swap(rsntFile[i].text, rsntFile[0].text);
      return;
    }
  }
  if (rsntFile[9].text)
    free(rsntFile[9].text);
  for (int i = 8; i >= 0; --i) {
    if (rsntFile[i].text) {
      rsntFile[i + 1].text = rsntFile[i].text;
      rsntFile[i + 1].type = ITEM_ACTIVE;
    }
  }
  rsntFile[0].text = strdup(s);
  rsntFile[0].type = ITEM_ACTIVE;
  fileMenu[4].type = ITEM_SUBMENU;
}

void MainScreen::fsave_hndl(int isok, PBFileChooser * dlg)
{
  if (isok) {
    std::string s = dlg->getPath();
    if (ms->saveFile(s.c_str())) {
      addRecent(s.c_str());
      ms->text.changed = false;
    }
  }
}

void MainScreen::goto_ln_hndl(PBNumericSelector * ns, bool ok)
{
  if (ok) {
    int ln = ns->selected();
    if (ln == 0)
      ms->text.selectItem(0);
    else if (ln == -1)
      ms->text.selectItem(ms->text.itemsCount() - 1);
    ms->text.selectItem(ln - 1);
    ms->update();
  }
}

void MainScreen::fsh(char *r, char *b, char *i, char *t)
{
  std::string dfs(r);
  int dfss = atoi(dfs.substr(dfs.find_first_of(",") + 1).c_str());
  ifont *df = OpenFont(dfs.substr(0, dfs.find_first_of(",")).c_str(), dfss, 1);
  if (df) {
    ms->text.setWidgetFont(df);
    ms->text.update();
  }
}

void MainScreen::HandleMainMenuItem(int index)
{
  static char buff[1024];
  if(ITM_TOCF<=index){
    ms->outlineMenuHandler(index);
    return;
  }
  switch (index) {
  case ITM_FIND:
    if (!ms->sd) {
      ms->sd = new SearchDialog("Find", &ms->text, ms->text.getSelectedIndex());
    } else {
      ms->sd->searchFrom(ms->text.getSelectedIndex());
    }
    ms->sd->run();
    break;
  case ITM_1251T:
    ms->text.decode("cp1251");
    break;
  case ITM_866T:
    ms->text.decode("cp866");
    break;
  case ITM_KOI8T:
    ms->text.decode("koi8r");
    break;
  case ITM_1251:
    ms->text.convert("cp1251");
    break;
  case ITM_866:
    ms->text.convert("cp866");
    break;
  case ITM_KOI8:
    ms->text.convert("koi8r");
    break;
  case ITM_RSNT0:
  case ITM_RSNT1:
  case ITM_RSNT2:
  case ITM_RSNT3:
  case ITM_RSNT4:
  case ITM_RSNT5:
  case ITM_RSNT6:
  case ITM_RSNT7:
  case ITM_RSNT8:
  case ITM_RSNT9:
    if (rsntFile[index - ITM_RSNT0].text) {
      //ms->fopen_hndl(0,rsntFile[index-ITM_RSNT0].text);
      if (ms->openFile(rsntFile[index - ITM_RSNT0].text)) {
        ms->addRecent(rsntFile[index - ITM_RSNT0].text);
        ms->text.updateNumbers();
      }
      ms->update();
    }
    break;
  case ITM_FONT:{
      std::stringstream str;
      str << ms->text.getFont()->name;
      if (str.str().find(".ttf") == std::string::npos)
        str << ".ttf";
      str << "," << ms->text.getFont()->size;
      //std::cerr<<str.str()<<std::endl;
      OpenFontSelector("Select", str.str().c_str(), 1, fsh);
    }
    break;
  case ITM_ORIE:
    OpenRotateBox(orie_hndl);
    break;
  case ITM_UNDO:
    ms->text.undoOp();
    break;
  case ITM_EDIT:
    if (ms->select_mode) {
      std::cerr << ms->fileName << "," << ms->text.getSelectedIndex() + 1 << std::endl;
      exit(0);
    } else {
      ms->text.editItem();
    }
    break;
  case ITM_COPY:
    if (PBTextDisplay::item) {
      ms->text.copy_buf = PBTextDisplay::item->getText();
      ms->update();
    }
    break;
  case ITM_DELE:
    if (PBTextDisplay::item) {
      ms->text.curop.op = undoop::DELETE;
      ms->text.curop.str = PBTextDisplay::item->getText();
      ms->text.curop.pos = ms->text.getPos(PBTextDisplay::item);
      ms->text.undo.push_back(ms->text.curop);
      //
      PBTextDisplay::item = ms->text.erase(PBTextDisplay::item);
      ms->text.changed = true;
      ms->text.updateNumbers();
      ms->update();
    }
    break;
  case ITM_INSB:
    if (PBTextDisplay::item) {
      PBListBoxItem *itm = ms->text.insertBefore(PBTextDisplay::item, "");
      ms->text.updateNumbers();
      if (itm) {
        ms->update();
        ms->text.curop.op = undoop::INSERT;
        ms->text.curop.str = itm->getText();
        ms->text.curop.pos = ms->text.getPos(itm);
        ms->text.undo.push_back(ms->text.curop);
        PBTextDisplay::item = itm;
        ms->text.editItem(true);
      }
    }
    break;
  case ITM_INSA:
    if (PBTextDisplay::item) {
      PBListBoxItem *itm = ms->text.insertAfter(PBTextDisplay::item, "");
      ms->text.updateNumbers();
      if (itm) {
        ms->update();
        ms->text.curop.op = undoop::INSERT;
        ms->text.curop.str = itm->getText();
        ms->text.curop.pos = ms->text.getPos(itm);
        ms->text.undo.push_back(ms->text.curop);
        PBTextDisplay::item = itm;
        ms->text.editItem(true);
      }
    }
    break;
  case ITM_NEWF:
    {
      ms->text.clear();
      ms->text.undo.clear();
      PBListBoxItem *itm = ms->text.addItem("");
      ms->text.updateNumbers();
      if (itm) {
        ms->update();
        PBTextDisplay::item = itm;
        ms->text.editItem();
      }
    }
    break;
  case ITM_EXIT:
    if (ms->text.changed) {
      Dialog(ICON_QUESTION, "Quit?", "File changed! Quit?", "Yes", "NO", MainScreen::exit_dlg_h);
    } else {
      std::cerr << ms->fileName << "," << ms->text.getSelectedIndex() + 1 << std::endl;
      SaveCFG();
      exit(0);
    }
    break;
  case ITM_OPEN:{
      if (ms->fileName.empty())
        getcwd(buff, 1023);
      else
        strcpy(buff, ms->fileName.c_str());
      OpenFileChooser("Open", buff, "*.txt\n*.cfg\n*", 0, (pb_dialoghandler) fopen_hndl);
      break;
    }
  case ITM_IMPORT:{
      if (ms->fileName.empty())
        getcwd(buff, 1023);
      else
        strcpy(buff, ms->fileName.c_str());
      OpenFileChooser("Import", buff, "*.txt\n*.cfg\n*", 0, (pb_dialoghandler) fsimport_hndl);
      break;
    }
  case ITM_SAVE:{
      if (ms->fileName.empty())
        getcwd(buff, 1023);
      else
        strcpy(buff, ms->fileName.c_str());
      OpenFileChooser("Save", buff, "*", 1, (pb_dialoghandler) fsave_hndl);
      //OpenKeyboard("Save",buff,1024,1,fsave_hndl);
      break;
    }
  case ITM_ABOUT:
    Message(ICON_INFORMATION, "π", "π(pi) - Pocketbook edItor v 1.0.2\n"
            "Yury P. Fedorchenko © 2011-2012\n"
            "OK or click - edit line\n" "long OK or long click - menu", 50000);
    break;
  case ITM_GOTO:
    //OpenKeyboard("Go to Line",buff,1024,KBD_NUMERIC | KBDOPTS,goto_hndl);
    //OpenPageSelector(goto_n_hndl);
    if (!ns) {
      ns = new PBNumericSelector("Go to Line:");
      ns->onSelect.connect(sigc::mem_fun(ms, &MainScreen::goto_ln_hndl));
    }
    //ns->selected(ms->text.getSelectedIndex()+1);
    ns->reset();
    ns->run();
    break;
  case ITM_SHNO:
    ms->showNumbers(!ms->text.sh_ln);
    break;
  case ITM_OUT_ENABLE:
    ms->enableOutline(!ms->enable_outline);
    break;
  case ITM_OPLF:
    ms->openLast(!ms->open_last);
    break;
  case ITM_KEYS:
    ms->pipref->run();
    break;
  }
}

void MainScreen::openLast(bool s)
{
  ms->open_last = s;
  optMenu[2].type = (s) ? ITEM_BULLET : ITEM_ACTIVE;
}
void MainScreen::enableOutline(bool s)
{
  ms->enable_outline = s;
  optMenu[8].type = (s) ? ITEM_BULLET : ITEM_ACTIVE;
  mainMenu[4].type = (s) ? ITEM_SUBMENU : ITEM_HIDDEN;
}

void MainScreen::showNumbers(bool s)
{
  ms->text.sh_ln = s;
  optMenu[0].type = (s) ? ITEM_BULLET : ITEM_ACTIVE;
  ms->text.updateNumbers();
  ms->update();
}

MainScreen::MainScreen(const char *nm, int no, bool selectmode):PBWidget("", NULL),
text("ta", this), sd(NULL)
{
  addWidget(&text);
  placeWidgets();
  select_mode = selectmode;
  ms = this;
  if (nm) {
    openFile(nm);
    if (no > 0) {
      showNumbers(true);
      text.selectItem(no - 1);
      update();
    }
  } else
    text.addItem("Long OK or Long tap for menu");
}

int MainScreen::openFile(const char *nm,bool insert)
{
  std::ifstream in(nm);
  if (in.fail()) {
    char errstr[256];
    sprintf(errstr, "Can't open file:%s", nm);
    Message(ICON_ERROR, "Error", errstr, 50000);
    return 0;
  }
  fileName = nm;
  if(!insert)text.clear();
  while (!in.fail() && !in.eof()) {
    std::string line;
    std::getline(in, line);
    if(insert){
      PBListBoxItem *itm = text.insertAfter(text.getSelectedItem(),line);
      itm->setCanBeFocused(true);
      itm->setFocused(true);
    }else{
      PBListBoxItem *itm = text.addItem(line);
      itm->setCanBeFocused(true);
    }
  }
  return 1;
}

int MainScreen::saveFile(const char *nm)
{
  std::ofstream out(nm);
  if (!out.fail()) {
    fsaver a(out);
    text.forEachItem(a);
  }
  if (out.fail()) {
    char errstr[256];
    sprintf(errstr, "Can't save file:%s", nm);
    Message(ICON_ERROR, "Error", errstr, 50000);
    return 0;
  } else
    fileName = nm;
  return 1;
}

void MainScreen::openMenu(int x,int y){
  ms->updateMenu();
  OpenMenu(mainMenu, 0, x, y, HandleMainMenuItem);
}

int MainScreen::handle(int type, int par1, int par2)
{
  if (EVT_SHOW == type) {
    setFocused(true);
    update();
    return 1;
  }
  if (EVT_KEYREPEAT == type) {
    if (KEY_OK == par1) {
      openMenu(10, 10);
      return 1;
    }
  }
  if (EVT_POINTERLONG == type) {
    openMenu(par1, par2);
  }
  if (EVT_KEYUP == type){
    if(pipref->handleKeys(par1))return 1;
  }
  return PBWidget::handle(type, par1, par2);
}

MainScreen *MainScreen::ms = NULL;
PBNumericSelector *MainScreen::ns = NULL;

void keyboard_entry(char *s)
{
  if (s && PBTextDisplay::item) {
    PBTextDisplay::item->setText(s);
    MainScreen::ms->setChanged(true);
    PBTextDisplay::item->getParent()->update();
    MainScreen::ms->text.undo.push_back(MainScreen::ms->text.curop);
  }
  if (PBTextDisplay::buff) {
    delete[]PBTextDisplay::buff;
    PBTextDisplay::buff = NULL;
  }
}

int lnno = 0;
const char *fname = NULL;
bool select_mode = false;

iconfig *config;

void SaveCFG()
{
  std::stringstream str;
  str << MainScreen::ms->text.getFont()->name;
  if (str.str().find(".ttf") == std::string::npos)
    str << ".ttf";
  str << "," << MainScreen::ms->text.getFont()->size;
  WriteString(config, "Font", str.str().c_str());
  WriteInt(config, "Orient", GetOrientation());
  WriteInt(config, "LineNumbers", MainScreen::ms->text.sh_ln);
  WriteInt(config, "OpenLast", MainScreen::ms->open_last);
  WriteInt(config, "EnableOutline", MainScreen::ms->enable_outline);
  WriteInt(config, "Indentation", MainScreen::ms->indentation);
  for (int i = 9; i >= 0; --i) {
    if (rsntFile[i].text) {
      std::stringstream str1;
      str1 << "RF" << i;
      WriteString(config, str1.str().c_str(), rsntFile[i].text);
    }
  }
  MainScreen::ms->pipref->saveConfig(config);
  SaveConfig(config);
  CloseConfig(config);
}

int main_handler(int type, int par1, int par2)
{
  if (EVT_INIT == type) {
    config = OpenConfig(CONFIGPATH "/pi.cfg", NULL);
    if (config) {
      MainScreen::ms = new MainScreen(fname, lnno, select_mode);
      MainScreen::ms->pipref = new PiPref();
      MainScreen::ms->fsh(ReadString(config, "Font", "LiberationMono, 22"), NULL, NULL, NULL);
      MainScreen::ms->orie_hndl(ReadInt(config, "Orient", 0));
      MainScreen::ms->showNumbers(ReadInt(config, "LineNumbers", 0));
      MainScreen::ms->openLast(ReadInt(config, "OpenLast", 0));
      MainScreen::ms->enableOutline(ReadInt(config, "EnableOutline", 0));
      MainScreen::ms->indentation=ReadInt(config, "Indentation", 2);
      for (int i = 9; i >= 0; --i) {
        std::stringstream str;
        str << "RF" << i;
        char *s = ReadString(config, str.str().c_str(), NULL);
        if (s && *s) {
          MainScreen::ms->addRecent(s);
          if(i==0 && MainScreen::ms->open_last && !fname){
            MainScreen::ms->openFile(s);
            MainScreen::ms->text.updateNumbers();
          }
        }
      }
      MainScreen::ms->pipref->loadConfig(config);
    }
    return 1;
  }
  MainScreen::ms->handle(type, par1, par2);
  return 0;
}

int main(int argc, char **argv)
{
  OpenScreen();
  if (argc == 2) {
    fname = argv[1];
  } else {
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-l") == 0 && i < argc - 1) {
        lnno = atoi(argv[i + 1]);
        ++i;
      } else if (strcmp(argv[i], "-f") == 0 && i < argc - 1) {
        fname = argv[i + 1];
        ++i;
      } else if (strcmp(argv[i], "-s") == 0) {
        select_mode = true;
      }
      if (i == argc - 1 && !fname)
        fname = argv[i];
    }
  }
  InkViewMain(main_handler);
  return 0;
}
