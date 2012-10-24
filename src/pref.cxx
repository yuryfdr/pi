/* Copyright (C) 2012 Yury P. Fedorchenko (yuryfdr at users.sf.net)  */
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
#include "pref.h"

static int msecs;

void aus_timer(){
  std::cerr<<__PRETTY_FUNCTION__<<std::endl;
  if(!MainScreen::ms->fileName.empty())MainScreen::ms->saveFile(MainScreen::ms->fileName.c_str());
  SetWeakTimer("PIAUSTIMER", aus_timer, msecs);
}

void PiPref::preasave(){
  if(kbx_autosave.getText() != "-"){
    msecs = 60000*atoi( kbx_autosave.getText().c_str());
    ClearTimer(aus_timer);
    SetWeakTimer("PIAUSTIMER", aus_timer, msecs);
  }else{
    ClearTimer(aus_timer);
  }
}

void PiPref::quit(bool isok){
  for(std::map<int,std::string>::iterator it=keynames.begin();it!=keynames.end();++it){
    if(it->second == kbx_vo.getText())keysignals["ViewOutline"] = it->first;
    if(it->second == kbx_qs.getText())keysignals["QuickSave"] = it->first;
    if(it->second == kbx_ia.getText())keysignals["InsertAfter"] = it->first;
    if(it->second == kbx_om.getText())keysignals["OpenMenu"] = it->first;
  }
  preasave();
  PBDialog::quit(isok);
}


void PiPref::loadConfig(iconfig* cfg){
  keysignals["ViewOutline"] = ReadInt(cfg,"ViewOutline",0);
  keysignals["QuickSave"] = ReadInt(cfg,"QuickSave",0);
  keysignals["InsertAfter"] = ReadInt(cfg,"InsertAfter",0);
  keysignals["OpenMenu"] = ReadInt(cfg,"OpenMenu",0);
  kbx_autosave.setText( ReadString(cfg,"AutosaveInt","-") );
  preasave();
}
void PiPref::saveConfig(iconfig*cfg){
  WriteInt(cfg,"ViewOutline",keysignals["ViewOutline"]);
  WriteInt(cfg,"QuickSave",keysignals["QuickSave"]);
  WriteInt(cfg,"InsertAfter",keysignals["InsertAfter"]);
  WriteInt(cfg,"OpenMenu",keysignals["OpenMenu"]);
  WriteString(cfg,"AutosaveInt",kbx_autosave.getText().c_str());
}
