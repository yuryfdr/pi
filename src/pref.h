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
#ifndef _PI_PREF_H_
#define _PI_PREF_H_

#include "pi.h"
#include <map>

class PiPref : public PBDialog{
  std::map<std::string,int> keysignals;
  std::map<int,std::string> keynames;
  void init_keynames(){
    keynames[0]="None";
    keynames[KEY_POWER]="Power";
    keynames[KEY_DELETE]="Delete";
    keynames[KEY_MINUS]="-";
    keynames[KEY_PLUS]="+";
    keynames[KEY_MENU]="Menu";
    keynames[KEY_BACK]="Back";
    keynames[KEY_MUSIC]="Music";
    keynames[KEY_NEXT2]="Next2";
    keynames[KEY_PREV2]="Prev2";
#ifndef _OLD_DEV_
    keynames[KEY_HOME]="Home";
    keynames[KEY_ZOOMOUT]="ZoomIn";
    keynames[KEY_ZOOMIN]="ZoomOut";
#endif
  }
  PBLabel lb_vo,lb_qs,lb_ia,lb_om;
  PBComboBox kbx_vo,kbx_qs,kbx_ia,kbx_om;
  PBButton bt_close;
  PBLabel lb_asv;
  PBComboBox kbx_autosave;
  void on_close(PBButton*){
    quit(true);
  }
  void preasave();
 public:
  PiPref():PBDialog("Quick Keys")
    ,lb_vo("View Outline",this),lb_qs("Quick Save",this)
    ,lb_ia("Insert After",this),lb_om("Open Menu",this)
    ,kbx_vo("",this),kbx_qs("",this),kbx_ia("",this),kbx_om("",this)
    ,bt_close("Close",this)
    ,lb_asv("Autosave Int",this),kbx_autosave("",this){
    init_keynames();
    std::vector<std::string> kv;
    for(std::map<int,std::string>::iterator it=keynames.begin();it!=keynames.end();++it){
      kv.push_back(it->second);
    }
    addWidget(&lb_vo),addWidget(&lb_qs),addWidget(&lb_ia),addWidget(&lb_om);
    addWidget(&kbx_vo);kbx_vo.editable(false);kbx_vo.setItems(kv);
    addWidget(&kbx_qs);kbx_qs.editable(false);kbx_qs.setItems(kv);
    addWidget(&kbx_ia);kbx_ia.editable(false);kbx_ia.setItems(kv);
    addWidget(&kbx_om);kbx_om.editable(false);kbx_om.setItems(kv);
    addWidget(&bt_close);bt_close.onPress.connect(sigc::mem_fun(this,&PiPref::on_close));
    addWidget(&lb_asv);
    addWidget(&kbx_autosave);
    std::vector<std::string> ausi;
    ausi.push_back("-");
    ausi.push_back("1 min");
    ausi.push_back("5 min");
    ausi.push_back("10 min");
    ausi.push_back("30 min");
    kbx_autosave.editable(false);kbx_autosave.setItems(ausi);
  }
  void placeWidgets(){
    setSize((ScreenWidth()-400)/2,100,400,10);
    int yy = y()+captionHeight()+5;
    lb_vo.setSize(x()+5,yy,190,25);kbx_vo.setSize(x()+w()/2,yy,190,25);yy+=30;
    lb_qs.setSize(x()+5,yy,190,25);kbx_qs.setSize(x()+w()/2,yy,190,25);yy+=30;
    lb_ia.setSize(x()+5,yy,190,25);kbx_ia.setSize(x()+w()/2,yy,190,25);yy+=30;
    lb_om.setSize(x()+5,yy,190,25);kbx_om.setSize(x()+w()/2,yy,190,25);yy+=30;
    lb_asv.setSize(x()+5,yy,190,25);kbx_autosave.setSize(x()+w()/2,yy,190,25);yy+=30;
    bt_close.setSize(x()+w()/2,yy,190,25);yy+=30;
    setSize(x(),y(),w(),yy-100+captionHeight());
  }
  virtual void run(pb_dialoghandler* n=NULL){
    kbx_vo.setText(keynames[keysignals["ViewOutline"]]);
    kbx_qs.setText(keynames[keysignals["QuickSave"]]);
    kbx_ia.setText(keynames[keysignals["InsertAfter"]]);
    kbx_om.setText(keynames[keysignals["OpenMenu"]]);
    PBDialog::run(NULL);
  }
  virtual void quit(bool isok);
  void loadConfig(iconfig* cfg);
  void saveConfig(iconfig*cfg);
  //
  void view_outline(){
    MainScreen::ms->HandleMainMenuItem(ITM_TOCF);
  }
  void quick_save(){
    MainScreen::ms->saveFile(MainScreen::ms->fileName.c_str());
  }
  void insert_after(){
    MainScreen::ms->HandleMainMenuItem(ITM_INSA);
  }
  void open_menu(){
    MainScreen::ms->openMenu(10,10);
  }
  int handleKeys(int key){
    if(keysignals["ViewOutline"] == key){view_outline();return 1;}
    if(keysignals["QuickSave"] == key){quick_save();return 1;}
    if(keysignals["InsertAfter"] == key){insert_after();return 1;}
    if(keysignals["OpenMenu"] == key){open_menu();return 1;}
    return 0;
  }
};

#endif // _PI_PREF_H_
