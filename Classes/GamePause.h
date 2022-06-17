#pragma once
#ifndef __Gamepause__H__
#define __Gamepause__H__
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d;
using namespace cocostudio;

class Gamepause : public Scene
{
public:
    virtual bool init();
    static Scene* createScene();
    CREATE_FUNC(Gamepause);
    Gamepause();
    ~Gamepause();
    void playbtnContinueCallback(Ref* pSender, Widget::TouchEventType type);

private:

};

#endif 

