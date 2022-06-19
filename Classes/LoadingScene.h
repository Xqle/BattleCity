#pragma once
#ifndef __CoverScene__H__
#define __CoverScene__H__
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "GameClient.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d;
using namespace cocostudio;

class LoadingScene : public Scene
{
public:
    virtual bool init();
    static Scene* createScene();
    CREATE_FUNC(LoadingScene);
    LoadingScene();
    ~LoadingScene();
    void loading_func(float dt);

    cocos2d::ui::Widget* LoadingUI;

private:

};


#endif 