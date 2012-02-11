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
#ifndef ___PI_H___
#define ___PI_H___
#undef IVSAPP
//#include "inkview.h"

#include <iostream>
#include <functional>
#include <sstream>
#include <fstream>
#include "pbtk/pblistbox.h"
#include "pbtk/convert.h"
#include "pbtk/selector.h"
#include "pbtk/pbfilechooser.h"
#include "math.h"

#ifdef __EMU__
#define KBDOPTS KBD_SCREENTOP
#else
#define KBDOPTS 0
#endif

void keyboard_entry(char *s);

struct undoop {
  enum method{EDIT=0,INSERT,DELETE,EDIT_SUBTREE,INSERT_SUBTREE,DELETE_SUBTREE,MOVE_SUBTREE}op;
  int pos;
  int repeat;
  std::string str;
};

class PBTextDisplay:public PBListBox {
 public:
  bool changed, sh_ln;
  undoop curop;
  std::vector < undoop > undo;
  static PBListBoxItem *item;
  static char *buff;
  static int buff_size;
   std::string copy_buf;
   PBTextDisplay(const std::string & str, PBWidget * par);
  void updateNumbers();
  void draw() {
    PBListBox::draw();
  } int getPos(PBListBoxItem * itm);
  void editItem(bool ins = false);
  void undoOp();
  int handle(int type, int par1, int par2);
  struct encoder:public std::unary_function < void, PBListBoxItem * > {
    const unsigned short *enc;
     encoder(const std::string & _enc) {
      if (_enc == "cp1251")
        enc = cp1251_to_unicode;
      else if (_enc == "cp866")
        enc = cp866_to_unicode;
      else
        enc = koi8_to_unicode;
    };
    void operator () (PBListBoxItem * itm) {
      itm->setText(to_utf8(itm->getText().c_str(), enc));
    };
  };
  struct decoder:public std::unary_function < void, PBListBoxItem * > {
    const unsigned short *enc;
     decoder(const std::string & _enc) {
      if (_enc == "cp1251")
        enc = cp1251_to_unicode;
      else if (_enc == "cp866")
        enc = cp866_to_unicode;
      else
        enc = koi8_to_unicode;
    };
    void operator () (PBListBoxItem * itm) {
      itm->setText(utf8_to(itm->getText().c_str(), enc));
    };
  };
  void convert(const char *enc) {
    encoder a(enc);
    forEachItem(a);
    update();
  }
  void decode(const char *enc) {
    decoder a(enc);
    forEachItem(a);
    update();
  }
};

enum {
  ITM_OPEN,//file
  ITM_SAVE,
  ITM_IMPORT,
  ITM_NEWF,//file,edit
  ITM_ABOUT,
  ITM_EXIT,
  ITM_EDIT,//edit
  ITM_COPY,
  ITM_INSB,
  ITM_INSA,
  ITM_DELE,
  ITM_UNDO,
  ITM_GOTO,//search
  ITM_FIND,
  ITM_REPL,
  ITM_SHNO,//options
  ITM_OPLF,
  ITM_FONT,
  ITM_ORIE,
  ITM_OUT_ENABLE,
  ITM_KEYS,
  ITM_RESN,//resent files
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
  ITM_SUBM,//not handled submenu
  ITM_1251,//convert
  ITM_866,
  ITM_KOI8,
  ITM_1251T,
  ITM_866T,
  ITM_KOI8T,
  ITM_TOCF,// outline
  ITM_INSERT_CHILD_LINE,
  ITM_INSERT_SIBLING_LINE,
  ITM_DELETE_SUBTREE,
  ITM_EXPORT_SUBTREE,
  ITM_IMPORT_SIBLING_SUBTREE,
  ITM_IMPORT_CHILD_SUBTREE,
  ITM_COPY_SUBTREE,
  ITM_CUT_SUBTREE,
  ITM_INSERT_SIBLING_SUBTREE,
  ITM_INSERT_CHILD_SUBTREE,
  ITM_PROMOTE_SUBTREE,
  ITM_DEMOTE_SUBTREE,
  ITM_MOVEUP_SUBTREE,
  ITM_MOVEDOWN_SUBTREE,
  ITM_INDENT2,//outline options
  ITM_INDENT4
};

class SearchDialog;
class PiPref;
void SaveCFG();
class MainScreen:public PBWidget {
 public:
  PBTextDisplay text;
// private:
  std::string fileName;

 public:
  bool select_mode;
  bool open_last;
  bool enable_outline;
  // for outline mode
  int numfiles;
  tocentry *toc;
  long long posindex[1024];
  int indentation;
 //Dialogs
  static PBNumericSelector *ns;
  SearchDialog *sd;
  PiPref* pipref;
 public:
  void add_toc_entry(int, int, int, char*);
  int file_exists(const char*)const;
  void free_toc();
  void open_toc(long long position, bool showLineNumbers = false);
  static void toc_handler(long long);
  void deleteSubtree();
  void moveSubtree(bool moveDown=false);
  void changeLevel(bool doDemote=false);
  int determineLevel(const char* line);
  std::string getLineIndent(int level);
  int _exportFile(PBListBoxItem* itm, std::ofstream& out);
  int exportFile(const char* nm);
  static void fexport_hndl(int isok,PBFileChooser* dlg);
  int _importFile(PBListBoxItem* curr, const char* nm, bool as_child=false, bool insertBefore=false);
  int importFile(const char* nm, bool as_child=false, bool insertBefore=false);
  static void fsimport_hndl(int isok,PBFileChooser* dlg);
  static void fimport_hndl(int isok,PBFileChooser* dlg);
  static void fimport_child_hndl(int isok,PBFileChooser* dlg);

  //
  static void exit_dlg_h(int bt);
  static void fopen_hndl(int isok, PBFileChooser * dlg);
  static void addRecent(const char *s);
  static void fsave_hndl(int isok, PBFileChooser * dlg);


  void goto_ln_hndl(PBNumericSelector * ns, bool ok);

  static void orie_hndl(int s) {
    SetOrientation(s);
    ms->update();
  } static void fsh(char *r, char *b, char *i, char *t);

  static void HandleMainMenuItem(int index);
  void openMenu(int,int);
  static void outlineMenuHandler(int);//outline menu
  //options
  void showNumbers(bool s);
  void enableOutline(bool s);
  void openLast(bool s);

  MainScreen(const char *nm = NULL, int no = 0, bool selectmode = false);
  //file
  int openFile(const char *nm,bool insert=false);

  struct fsaver:public std::unary_function < void, PBListBoxItem * > {
    std::ofstream & out;
    fsaver(std::ofstream & _out):out(_out) {
    };
    void operator () (PBListBoxItem * itm) {
      out << itm->getText() << std::endl;
    };
  };
  int saveFile(const char *nm);
  //
  void placeWidgets() {
    setSize(0, 0, ScreenWidth(), ScreenHeight());
    text.setSize(0, 0, ScreenWidth(), ScreenHeight());
  }
  int handle(int type, int par1, int par2);
  void updateMenu();
  static MainScreen *ms;
  static void setChanged(bool is) {
    MainScreen::ms->text.changed = is;
    MainScreen::ms->text.updateNumbers();
  }
};

#endif
