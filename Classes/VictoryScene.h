#pragma once
#ifndef __VictoryScene__H__
#define __VictoryScene__H__
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "GameClient.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d;
using namespace cocostudio;

class VictoryScene : public Scene
{
public:
    virtual bool init();
    static Scene* createScene();
    CREATE_FUNC(VictoryScene);
    VictoryScene();
    ~VictoryScene();
    void loading_func(float dt);

    cocos2d::ui::Widget* LoadingUI;

private:
    int victory_flag = 1;
};


#endif 