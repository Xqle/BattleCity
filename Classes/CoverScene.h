#pragma once
#ifndef __CoverScene__H__
#define __CoverScene__H__
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "LoadingScene.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d;
using namespace cocostudio;

class CoverScene : public Scene
{
public:
    virtual bool init();
    static Scene* createScene();
    CREATE_FUNC(CoverScene);
    CoverScene();
    ~CoverScene();
    void playbtnContinueCallback(Ref* pSender, Widget::TouchEventType type);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
    

    cocos2d::ui::Widget* coverUI;

private:

};

#endif 

