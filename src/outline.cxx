/* Copyright (C) 2011-2012 Yury P. Fedorchenko (yuryfdr at users.sf.net)  
   thanks to bogomil for outline code */
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
#include "pi.h"
#include "math.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <inkview.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////
/* is c the start of a utf8 sequence? */
#define isutf(c) (((c)&0xC0)!=0x80)

static const u_int32_t offsetsFromUTF8[6] = {
  0x00000000UL, 0x00003080UL, 0x000E2080UL,
  0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const char trailingBytesForUTF8[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

/* returns length of next utf-8 sequence */
int u8_seqlen(char *s)
{
  return trailingBytesForUTF8[(unsigned int)(unsigned char)s[0]] + 1;
}

/* charnum => byte offset */
int u8_offset(char *str, int charnum)
{
  int offs = 0;

  while (charnum > 0 && str[offs]) {
    (void)(isutf(str[++offs]) || isutf(str[++offs]) || isutf(str[++offs]) || ++offs);
    charnum--;
  }
  return offs;
}

/* byte offset => charnum */
int u8_charnum(char *s, int offset)
{
  int charnum = 0, offs = 0;

  while (offs < offset && s[offs]) {
    (void)(isutf(s[++offs]) || isutf(s[++offs]) || isutf(s[++offs]) || ++offs);
    charnum++;
  }
  return charnum;
}

//////////////////////////////////////////////////////////////////////////
imenu outlineMenu[] = {
  {ITEM_ACTIVE, ITM_TOCF, "View Outline", NULL}
  ,
  {ITEM_SEPARATOR, 0, NULL, NULL}
  ,
  {ITEM_ACTIVE, ITM_INSERT_SIBLING_LINE, "Insert/Paste Sibling Line After", NULL}
  ,
  {ITEM_ACTIVE, ITM_INSERT_CHILD_LINE, "Insert/Paste Child Line After", NULL}
  ,
  //{ ITEM_SEPARATOR, 0, NULL, NULL },
  {ITEM_ACTIVE, ITM_COPY_SUBTREE, "Copy Subtree", NULL}
  ,
  {ITEM_ACTIVE, ITM_CUT_SUBTREE, "Cut Subtree", NULL}
  ,
  {ITEM_ACTIVE, ITM_INSERT_SIBLING_SUBTREE, "Insert/Paste Sibling Subtree Before", NULL}
  ,
  {ITEM_ACTIVE, ITM_DELETE_SUBTREE, "Delete Subtree", NULL}
  ,
  //{ ITEM_SEPARATOR, 0, NULL, NULL },
  {ITEM_ACTIVE, ITM_EXPORT_SUBTREE, "Export Subtree to File", NULL}
  ,
  {ITEM_ACTIVE, ITM_IMPORT_SIBLING_SUBTREE, "Import from File as Sibling Subtree", NULL}
  ,
  {ITEM_ACTIVE, ITM_IMPORT_CHILD_SUBTREE, "Import from File as Child Subtree", NULL}
  ,
  {ITEM_SEPARATOR, 0, NULL, NULL}
  ,
  {ITEM_ACTIVE, ITM_PROMOTE_SUBTREE, "Promote Subtree", NULL}
  ,
  {ITEM_ACTIVE, ITM_DEMOTE_SUBTREE, "Demote Subtree", NULL}
  ,
  {ITEM_ACTIVE, ITM_MOVEUP_SUBTREE, "Move Up Subtree", NULL}
  ,
  {ITEM_ACTIVE, ITM_MOVEDOWN_SUBTREE, "Move Down Subtree", NULL}
  ,
  {0, 0, NULL, NULL}
};

int MainScreen::_exportFile(PBListBoxItem * itm, std::ofstream & out)
{
  int level = determineLevel(itm->getText().c_str());
  std::string indent = getLineIndent(level);

  out << itm->getText().erase(0, indent.size());

  int pos = text.getPos(itm) + 1;
  if (pos != -1)
    itm = (PBListBoxItem *) text.getItem(pos);
  else
    itm = NULL;

  while (itm != NULL) {
    int lvl = determineLevel(itm->getText().c_str());

    if (lvl <= level)
      break;

    out << std::endl << itm->getText().erase(0, indent.size());

    itm = (PBListBoxItem *) text.getItem(++pos);
  }
  out.flush();

  return 1;
}

int MainScreen::exportFile(const char *nm)
{
  std::ofstream out(nm);
  if (out.fail()) {
    char errstr[256];
    sprintf(errstr, "Can't export file:%s", nm);
    Message(ICON_ERROR, "Error", errstr, 50000);
    return 0;
  }

  PBListBoxItem *itm = (PBListBoxItem *) text.getFocusedWidget();
  if (itm != NULL) {
    _exportFile(itm, out);
  }

  return 1;
}

void MainScreen::fexport_hndl(int isok, PBFileChooser * dlg)
{
  if (isok) {
    std::string s = dlg->getPath();
    ms->exportFile(s.c_str());
  }
}

int MainScreen::_importFile(PBListBoxItem * curr, const char *nm, bool as_child, bool insertBefore)
{
  std::ifstream in(nm);
  if (in.fail()) {
    char errstr[256];
    sprintf(errstr, "Can't import file:%s", nm);
    Message(ICON_ERROR, "Error", errstr, 50000);
    return 0;
  }

  int level = determineLevel(curr->getText().c_str());
  std::string indent = getLineIndent(level + (as_child));

  int numop = 0;
  while (!in.fail() && !in.eof()) {
    std::string line;

    std::getline(in, line);
    line.insert(0, indent);

    PBListBoxItem *itm =
        (insertBefore ? ms->text.insertBefore(curr, line) : ms->text.insertAfter(curr, line));
    ms->text.updateNumbers();
    if (itm) {
      ms->update();
      ms->text.curop.op = undoop::INSERT;
      ms->text.curop.str = itm->getText();
      ms->text.curop.pos = ms->text.getPos(itm);
      ms->text.undo.push_back(ms->text.curop);
      PBTextDisplay::item = itm;
    }

    itm->setCanBeFocused(true);

    if (!insertBefore)
      curr = itm;

    numop += 1;
  }

  if (numop > 0) {
    ms->text.curop.op = undoop::INSERT_SUBTREE;
    ms->text.curop.repeat = numop;
    ms->text.undo.push_back(ms->text.curop);
  }

  return 1;
}

int MainScreen::importFile(const char *nm, bool as_child, bool insertBefore)
{
  std::ifstream in(nm);
  if (in.fail()) {
    char errstr[256];
    sprintf(errstr, "Can't import file:%s", nm);
    Message(ICON_ERROR, "Error", errstr, 50000);
    return 0;
  }

  PBListBoxItem *curr = (PBListBoxItem *) text.getFocusedWidget();
  return _importFile(curr, nm, as_child, insertBefore);
}

void MainScreen::fimport_hndl(int isok, PBFileChooser * dlg)
{
  if (isok) {
    std::string s = dlg->getPath();
    ms->importFile(s.c_str());
  }
}

void MainScreen::fimport_child_hndl(int isok, PBFileChooser * dlg)
{
  if (isok) {
    std::string s = dlg->getPath();
    ms->importFile(s.c_str(), true);
  }
}

void MainScreen::deleteSubtree()
{
  PBListBoxItem *itm = (PBListBoxItem *) ms->text.getFocusedWidget();
  if (itm != NULL) {
    int pos = ms->text.getPos(itm);
    int level = MainScreen::ms->determineLevel(itm->getText().c_str());
    std::string indent = MainScreen::ms->getLineIndent(level);

    ms->text.curop.op = undoop::DELETE;
    ms->text.curop.str = itm->getText();
    ms->text.curop.pos = ms->text.getPos(itm);
    ms->text.undo.push_back(ms->text.curop);
    ms->text.erase(itm);

    int numop = 1;
    itm = (PBListBoxItem *) ms->text.getItem(pos);
    while (itm != NULL) {
      int lvl = MainScreen::ms->determineLevel(itm->getText().c_str());

      if (lvl <= level)
        break;

      ms->text.curop.op = undoop::DELETE;
      ms->text.curop.str = itm->getText();
      ms->text.curop.pos = ms->text.getPos(itm);
      ms->text.undo.push_back(ms->text.curop);

      ms->text.erase(itm);

      itm = (PBListBoxItem *) ms->text.getItem(pos);
      numop += 1;
    }

    ms->text.curop.op = undoop::DELETE_SUBTREE;
    ms->text.curop.repeat = numop;
    ms->text.undo.push_back(ms->text.curop);

    ms->text.changed = true;
    ms->text.updateNumbers();
    ms->update();
  }
}

void MainScreen::moveSubtree(bool moveDown)
{
  // Get previous with the same or lower level
  PBListBoxItem *prev = (PBListBoxItem *) text.getFocusedWidget();
  int level = determineLevel(prev->getText().c_str());
  std::string indent = getLineIndent(level);
  int pos = text.getPos(prev);

  PBListBoxItem *itm = NULL;
  if (pos != -1)
    itm = (PBListBoxItem *) text.getItem(pos + (moveDown ? 1 : -1));
  else
    itm = NULL;

  int ocur = 1;
  while (itm != NULL) {
    int lvl = determineLevel(itm->getText().c_str());

    if (lvl <= level) {
      if ((ocur == 1 && !moveDown) || (ocur == 2 && moveDown))
        break;
      else
        ocur += 1;
    }

    pos += (moveDown ? 1 : -1);
    itm = (PBListBoxItem *) text.getItem(pos);
  }

  // Cut Subtree
  std::ofstream out("/tmp/pibuffer.txt");
  ms->_exportFile(prev, out);
  ms->deleteSubtree();

  // Paste Subtree
  _importFile(itm, "/tmp/pibuffer.txt", false, true);

  ms->text.curop.op = undoop::MOVE_SUBTREE;
  ms->text.curop.str = "";
  ms->text.curop.pos = ms->text.getPos(NULL);
  ms->text.undo.push_back(ms->text.curop);
}

void MainScreen::changeLevel(bool doDemote)
{
  int numop = 0;
  PBListBoxItem *itm = (PBListBoxItem *) text.getFocusedWidget();
  if (itm != NULL) {
    int level = determineLevel(itm->getText().c_str());

    if (!doDemote && level < 2)
      return;

    std::string line = itm->getText();

    ms->text.curop.op = undoop::EDIT;
    ms->text.curop.str = line;
    ms->text.curop.pos = ms->text.getPos(itm);
    ms->text.undo.push_back(ms->text.curop);

    if (!doDemote)
      itm->setText(line.erase(0, getLineIndent(2).size()));
    else
      itm->setText(line.insert(0, getLineIndent(2)));

    //PBTextDisplay::item = itm;

    numop = 1;
    int pos = text.getPos(itm) + 1;
    itm = (pos != -1 ? (PBListBoxItem *) text.getItem(pos) : NULL);

    while (itm != NULL) {
      int lvl = determineLevel(itm->getText().c_str());

      if (lvl <= level)
        break;

      line = itm->getText();

      ms->text.curop.op = undoop::EDIT;
      ms->text.curop.str = line;
      ms->text.curop.pos = ms->text.getPos(itm);
      ms->text.undo.push_back(ms->text.curop);

      if (!doDemote)
        itm->setText(line.erase(0, getLineIndent(2).size()));
      else
        itm->setText(line.insert(0, getLineIndent(2)));

      //PBTextDisplay::item = itm;

      itm = (PBListBoxItem *) text.getItem(++pos);
      numop += 1;
    }
  }

  if (numop > 0) {
    ms->text.curop.op = undoop::EDIT_SUBTREE;
    ms->text.curop.repeat = numop;
    ms->text.undo.push_back(ms->text.curop);

    ms->text.updateNumbers();
    ms->update();
  }
}

int MainScreen::determineLevel(const char *line)
{
  int spaces = 0, level = 0;
  while (line[0] == 32 && line[0] != 0) {
    line += 1;
    spaces += 1;
  }
  if (spaces > 0 && spaces <= indentation)
    level = 2;
  else if (spaces > indentation && spaces <= 2 * indentation)
    level = 3;
  else if (spaces > 2 * indentation && spaces <= 3 * indentation)
    level = 4;
  else if (spaces > 3 * indentation && spaces <= 4 * indentation)
    level = 5;
  else if (spaces > 4 * indentation && spaces <= 5 * indentation)
    level = 6;
  else if (spaces > 5 * indentation && spaces <= 6 * indentation)
    level = 7;
  else if (spaces > 6 * indentation && spaces <= 7 * indentation)
    level = 8;
  else if (spaces > 7 * indentation && spaces <= 8 * indentation)
    level = 9;
  else if (spaces > 8 * indentation && spaces <= 9 * indentation)
    level = 10;
  else
    level = 1;

  return level;
}

std::string MainScreen::getLineIndent(int level)
{
  std::string indnt = "";
  for (int i = 1; i < level; i++)
    indnt += (indentation == 2 ? "  " : "    ");

  return indnt;
}

void MainScreen::outlineMenuHandler(int index)
{
  static char buff[1024];
  switch (index) {
  case ITM_TOCF:
    {
      int pos = 0;
      PBListBoxItem *p = (PBListBoxItem *) ms->text.getFocusedWidget();
      if (p != NULL) {
        pos = ms->text.getPos(p);
      }
      ms->open_toc(pos);
    }
    break;

  case ITM_INSERT_SIBLING_LINE:{
      PBListBoxItem *prev = (PBListBoxItem *) ms->text.getFocusedWidget();
      if (prev != NULL) {
        int level = MainScreen::ms->determineLevel(prev->getText().c_str());
        std::string indent = MainScreen::ms->getLineIndent(level);
        PBListBoxItem *itm =
            ms->text.insertAfter((const PBListBoxItem *)ms->text.getFocusedWidget(), indent);
        ms->text.updateNumbers();
        if (itm) {
          ms->update();
          ms->text.curop.op = undoop::INSERT;
          ms->text.curop.str = itm->getText();
          ms->text.curop.pos = ms->text.getPos(itm);
          ms->text.undo.push_back(ms->text.curop);
          PBTextDisplay::item = itm;
          ms->text.editItem();
        }
      }
    }
    break;
  case ITM_INSERT_CHILD_LINE:{
      PBListBoxItem *prev = (PBListBoxItem *) ms->text.getFocusedWidget();
      if (prev != NULL) {
        int level = MainScreen::ms->determineLevel(prev->getText().c_str());
        std::string indent = MainScreen::ms->getLineIndent(level + 1);
        PBListBoxItem *itm =
            ms->text.insertAfter((const PBListBoxItem *)ms->text.getFocusedWidget(), indent);
        ms->text.updateNumbers();
        if (itm) {
          ms->update();
          ms->text.curop.op = undoop::INSERT;
          ms->text.curop.str = itm->getText();
          ms->text.curop.pos = ms->text.getPos(itm);
          ms->text.undo.push_back(ms->text.curop);
          PBTextDisplay::item = itm;
          ms->text.editItem();
        }
      }
    }
    break;
  case ITM_COPY_SUBTREE:
    ms->exportFile("/tmp/pibuffer.txt");
    break;
  case ITM_CUT_SUBTREE:
    ms->exportFile("/tmp/pibuffer.txt");
    ms->deleteSubtree();
    break;
  case ITM_DELETE_SUBTREE:
    ms->deleteSubtree();
    break;
  case ITM_INSERT_SIBLING_SUBTREE:
    ms->importFile("/tmp/pibuffer.txt", false, true);
    break;
  case ITM_INSERT_CHILD_SUBTREE:
    ms->importFile("/tmp/pibuffer.txt", true, true);
    break;
  case ITM_EXPORT_SUBTREE:{
      if (ms->fileName.empty())
        getcwd(buff, 1023);
      else
        strcpy(buff, ms->fileName.c_str());
      OpenFileChooser("Save", buff, "*", 1, (pb_dialoghandler) fexport_hndl);
      break;
    }
  case ITM_IMPORT_SIBLING_SUBTREE:{
      if (ms->fileName.empty())
        getcwd(buff, 1023);
      else
        strcpy(buff, ms->fileName.c_str());
      OpenFileChooser("Save", buff, "*", 1, (pb_dialoghandler) fimport_hndl);
      break;
    }
  case ITM_IMPORT_CHILD_SUBTREE:{
      if (ms->fileName.empty())
        getcwd(buff, 1023);
      else
        strcpy(buff, ms->fileName.c_str());
      OpenFileChooser("Save", buff, "*", 1, (pb_dialoghandler) fimport_child_hndl);
      break;
    }
  case ITM_PROMOTE_SUBTREE:
    ms->changeLevel();
    break;
  case ITM_DEMOTE_SUBTREE:
    ms->changeLevel(true);
    break;
  case ITM_MOVEUP_SUBTREE:
    ms->moveSubtree();
    break;
  case ITM_MOVEDOWN_SUBTREE:
    ms->moveSubtree(true);
    break;
  case ITM_INDENT2:
    ms->indentation = 2;
    break;
  case ITM_INDENT4:
    ms->indentation = 4;
    break;
  default:
    printf("Index %d not handled\n", index);
  }
}

void toc_handler(long long pos);

void MainScreen::add_toc_entry(int level, int pos, int actualpos, char *str)
{
  toc[pos].level = level;
  toc[pos].page = 0;
  toc[pos].position = pos;
  toc[pos].text = str;

  MainScreen::posindex[pos] = actualpos;
}

int MainScreen::file_exists(const char *str) const
{
  if (access(str, F_OK | R_OK) == 0)
    return 1;
  return 0;
}

void MainScreen::free_toc()
{
  if (toc != NULL) {
    for (int i = 0; i < numfiles; i++) {
      free(toc[i].text);
    }
    free(toc);
    toc = NULL;
  }
}

std::vector < std::string > &split(std::string & s, char delim, std::vector < std::string > &elems)
{
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector < std::string > split(std::string & s, char delim)
{
  std::vector < std::string > elems;
  return split(s, delim, elems);
}

void MainScreen::open_toc(long long position, bool showLineNumbers)
{
#ifdef __EMU__
  const int PAGELINES = 19;
#else
  const int PAGELINES = 25;
#endif
  int i = 0, level = 1, lastl = -1, lastc = 0, inx = 1, line = 0, skipl = 0;

  std::vector < int >levels;

  numfiles = 0;
  if (toc != NULL)
    free_toc();

  toc = new tocentry[1024] ();

  char num[10];
  char format[32];
  sprintf(format, "%%%dd. ", int (log10(text.itemsCount()) + 1));

  for (int it = 0; it < text.itemsCount(); it++) {

    std::string strLine = text.getItem(it)->getText();

    if (showLineNumbers) {
      sprintf(num, format, it + 1);
      strLine.insert(0, num);
    }

    int spaces = 0;
    const char *p = strLine.c_str();

    // Skip whitespace
    while (p[0] == 32 && p[0] != 0) {
      p += 1;
      spaces += 1;
    }
    while ((p[0] == '-' || p[0] == '*' || p[0] == '$' || p[0] == '&') && p[0] != 0)
      p += 1;
    while (p[0] == 32 && p[0] != 0)
      p += 1;

    if (p[0] == 0) {
      if (position >= inx++)
        position -= 1;
      continue;
    }
    // Determine the line level
    if (p[0] == '_' && lastc != '_' && lastc != '!') {
      level = lastl + 1;
      lastc = p[0];
      p += 1;
    } else if (p[0] == '_' && lastc == '_') {
      level = lastl;
      lastc = '_';
      p += 1;
    } else if (levels.size() == 0) {
      level = 1;
      levels.push_back(spaces);
    } else if (levels.back() == spaces) {
      level = levels.size();
    } else if (levels.back() < spaces) {
      levels.push_back(spaces);
      level = levels.size();
    } else {
      for (int i = levels.size() - 1; i >= 0; i--) {
        if (levels[i] <= spaces) {
          level = i + 1;
          break;
        }
      }
    }

    // Skip subtree if marked
    if (p[0] == '!' || p[0] == '#') {
      skipl = level;
      lastc = '!';
      if (position >= inx++)
        position -= 1;
      continue;
    } else if (lastc == '!') {
      if (skipl > 0 && level > skipl) {
        if (position >= inx++)
          position -= 1;

        continue;
      } else {
        lastc = 0;
        skipl = 0;
      }
    }
    // Add TOC entry
    if (p[0] == '@') {
      for (; line < PAGELINES; line++)
        add_toc_entry(1, i++, inx, strdup(""));
      if (position >= inx)
        position += 1;
    } else {
      if (p[0] != '_' && strLine.find_first_of('_') != std::string::npos) {
        uint lastpos = 0, pos = strLine.find_first_of('_');

        while (pos != std::string::npos) {
          if (strLine[lastpos] != '!') {
            while (strLine[0] == 32 && strLine[0] != 0) {
              strLine.erase(0, 1);
              pos -= 1;
            }
            add_toc_entry(level + 2 * (lastpos != 0), i++, inx,
                          strdup(strLine.substr(lastpos, pos - lastpos).c_str()));
            if (position >= inx)
              position += 1;
          }

          if (lastpos == 0) {
            add_toc_entry(level + 1, i++, inx, strdup("..."));
            if (position >= inx)
              position += 1;
          }

          lastpos = pos + 1;
          pos = strLine.find_first_of('_', pos + 1);
        }

        if (strLine[lastpos] != '!') {
          while (strLine[0] == 32 && strLine[0] != 0)
            strLine.erase(0, 1);
          add_toc_entry(level + 2, i++, inx,
                        strdup(strLine.substr(lastpos, strLine.size() - lastpos).c_str()));
        } else {
          if (position >= inx)
            position -= 1;
        }

        inx += 1;
      } else {
        add_toc_entry(level, i++, inx++, strdup(p));
      }

      if (level == 1)
        line++;
    }

    lastl = level;
    lastc = p[0];
    if (line >= PAGELINES)
      line = 0;
  }

  numfiles = i;

  OpenContents(toc, i, position, toc_handler);
}

/*  int handle(int type,int par1,int par2){
     fprintf(stderr, "handle %d %d %d\n",type,par1,par2);
    if(EVT_SHOW == type){
      setFocused(true);
      update();
      return 1;
    }
    if(EVT_KEYREPEAT==type){
      if(KEY_OK==par1){
        ms->updateMenu();
        OpenMenu(mainMenu, 0, 50, 50, HandleMainMenuItem);
        return 1;
      }
    }
    if(EVT_POINTERLONG==type){
        ms->updateMenu();
        OpenMenu(mainMenu, 0, par1, par2, HandleMainMenuItem);
    }
    //return PBWidget::handle(type,par1,par2);
    int ret = PBWidget::handle(type,par1,par2);
    if (!ret && (type == EVT_KEYUP ||type == EVT_KEYDOWN)){
      switch (par1){
        case KEY_BACK:{
          std::cerr<<"Save:"<<ms->fileName<<std::endl;
          saveFile(fileName.c_str());
          std::cerr<<ms->fileName<<","<<ms->text.getSelectedIndex()+1<<std::endl;
          PageSnapshot();
          exit(0);
        }
        case KEY_PLUS:{
          std::cerr<<"Save:"<<ms->fileName<<std::endl;
          saveFile(fileName.c_str());
          PageSnapshot();
          return 1;
        }
        case KEY_MINUS:{
          int pos=0;
          PBListBoxItem* p = (PBListBoxItem*)ms->text.getFocusedWidget();
          if(p!=NULL){
            pos=ms->text.getPos(p);
          }
          ms->open_toc(pos, true);
          return 1;
        }
        case KEY_NEXT:{
          PBListBoxItem* p = (PBListBoxItem*)ms->text.getFocusedWidget();
          if(p!=NULL){
            int pos=ms->text.getPos(p);
            ms->goto_n_hndl(pos+20);
          }
          return 1;
        }
        case KEY_PREV:{
          PBListBoxItem* p = (PBListBoxItem*)ms->text.getFocusedWidget();
          if(p!=NULL){
            int pos=ms->text.getPos(p);
            if(pos > 20){
              ms->goto_n_hndl(pos - 20);
            }
          }
          return 1;
        }
        case KEY_MENU:{
          int selectedIndex=ms->text.getSelectedIndex();
          ms->open_toc(selectedIndex);
          return 1;
        }
        case KEY_ZOOMIN:{
          PBListBoxItem* prev = (PBListBoxItem*)ms->text.getFocusedWidget();
          if(prev!=NULL){
            int level=determineLevel(prev->getText().c_str());
            std::string indent=getLineIndent(level);
            PBListBoxItem* itm = ms->text.insertAfter((const PBListBoxItem*)ms->text.getFocusedWidget(),indent);
            ms->text.updateNumbers();
            if(itm){
              ms->update();
              ms->text.curop.op=undoop::INSERT;
              ms->text.curop.str=itm->getText();
              ms->text.curop.pos=ms->text.getPos(itm);
              ms->text.undo.push_back(ms->text.curop);
              PBTextDisplay::item = itm;
              ms->text.editItem();
            }
          }
          return 1;
        }
        case KEY_ZOOMOUT:{
          PBListBoxItem* prev = (PBListBoxItem*)ms->text.getFocusedWidget();
          if(prev!=NULL){
            int level=determineLevel(prev->getText().c_str());
            std::string indent=getLineIndent(level+1);
            PBListBoxItem* itm = ms->text.insertAfter((const PBListBoxItem*)ms->text.getFocusedWidget(),indent);
            ms->text.updateNumbers();
            if(itm){
              ms->update();
              ms->text.curop.op=undoop::INSERT;
              ms->text.curop.str=itm->getText();
              ms->text.curop.pos=ms->text.getPos(itm);
              ms->text.undo.push_back(ms->text.curop);
              PBTextDisplay::item = itm;
              ms->text.editItem();
            }
          }
          return 1;
        }
      }
    }

    return ret;
  }
*/

void MainScreen::toc_handler(long long pos)
{
  char *p = NULL, *title = NULL, *link = NULL, *q = NULL;
  FILE *f = NULL;
  char cmdLine[256];
  int newcpage = 0;
  char *DataFile = NULL;

  if (pos >= 0 && pos < MainScreen::ms->numfiles) {
    p = (char *)MainScreen::ms->toc[(int)pos].text;

    while ((p[0] == 32 || p[0] == '-' || p[0] == '*' || p[0] == '#' || p[0] == '$' || p[0] == '&')
           && p[0] != 0)
      p += 1;

    if (!strncmp(p, "BOOK:", 5)) {
      p += 5;
      while (p[0] == 32 && p[0] != 0)
        p += 1;

      title = p;
      if ((link = strpbrk(p, "|")) == NULL) {
        title = NULL;
        link = p;
      } else {
        q = link;
        link += 1;
        q[0] = 0;
        while (link[0] == 32 && link[0] != 0)
          link += 1;
      }

      if ((p = strpbrk(link, ":")) != NULL) {
        q = p;
        p += 1;
        q[0] = 0;
        newcpage = atoi(p) - 1;
      }

      DataFile = GetAssociatedFile(link, 0);
      f = iv_fopen(DataFile, "rb+");
      if (f != NULL) {
        iv_fseek(f, 4, SEEK_SET);
        iv_fwrite(&newcpage, sizeof(int), 1, f);
        iv_fclose(f);
      } else {
      }

      OpenBook(link, NULL, 0);
      OpenContents(MainScreen::ms->toc, MainScreen::ms->numfiles, pos, toc_handler);

      //MainScreen::ms->free_toc();
      //MainScreen::ms->goto_n_hndl(pos+1);

      return;
    } else if (!strncmp(p, "LNK:", 4)) {
      p += 4;
      while (p[0] == 32 && p[0] != 0)
        p += 1;
      title = p;
      if ((link = strpbrk(p, "|")) == NULL) {
        title = NULL;
        link = p;
      } else {
        q = link;
        link += 1;
        q[0] = 0;
        while (link[0] == 32 && link[0] != 0)
          link += 1;
      }

      if ((p = strpbrk(link, ":")) != NULL) {
        q = p;
        p += 1;
        q[0] = 0;
      }

      MainScreen::ms->free_toc();
      MainScreen::ms->text.selectItem(pos + 1);

      if (MainScreen::ms->file_exists(link)) {

        strcpy(cmdLine, "/mnt/ext1/system/bin/pi.app ");
        if (p != NULL) {
          strcat(cmdLine, "-l ");
          strcat(cmdLine, p);
          strcat(cmdLine, " ");
        }
        strcat(cmdLine, "\"");
        strcat(cmdLine, link);
        strcat(cmdLine, "\"");

        system(cmdLine);

        OpenContents(MainScreen::ms->toc, MainScreen::ms->numfiles, pos, toc_handler);

        return;
      }
    }

    if (MainScreen::ms->fileName.c_str() != NULL)
      fprintf(stderr, "%s: %s\n", MainScreen::ms->fileName.c_str(),
              MainScreen::ms->toc[(int)pos].text);
    else
      fprintf(stderr, "%s\n", MainScreen::ms->toc[(int)pos].text);
  } else
    return;

  MainScreen::ms->free_toc();
  MainScreen::ms->text.selectItem(MainScreen::ms->posindex[pos]);
}
