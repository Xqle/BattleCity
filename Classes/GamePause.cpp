#include"GamePause.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

using namespace cocos2d::ui;

using namespace cocostudio;

Gamepause::Gamepause()
{

}
Gamepause::~Gamepause()
{
}

Scene* Gamepause::createScene()
{
	auto scene = Scene::create();
	auto layer = Gamepause::create();
	scene->addChild(layer);
	return scene;
}



bool Gamepause::init()
{
	if (!Scene::init())
	{
		return false;
	}



	auto pauseUI = GUIReader::getInstance()->widgetFromJsonFile("pauseUI/pauseUI.json");
	addChild(pauseUI, 100);

	auto Playbtn = (Button*)(pauseUI->getChildByName("panel_background")->getChildByName("playButton"));
	Playbtn->addTouchEventListener(CC_CALLBACK_2(Gamepause::playbtnContinueCallback, this));
	return true;
}

void Gamepause::playbtnContinueCallback(Ref* pSender, Widget::TouchEventType type)
{
	if (type == ui::Widget::TouchEventType::ENDED)
		Director::getInstance()->popScene();
}