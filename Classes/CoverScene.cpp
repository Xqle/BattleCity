#pragma once
#include"CoverScene.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "GameClient.h"
int Loadingflag = 1;
using namespace cocos2d::ui;

using namespace cocostudio;

CoverScene::CoverScene()
{

}
CoverScene::~CoverScene()
{

}

Scene* CoverScene::createScene()
{
	auto scene = Scene::create();
	auto layer = CoverScene::create();
	scene->addChild(layer);
	return scene;
}

void CoverScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_ENTER:
	{
		//auto coverpanel = ()coverUI->getChildByTag(16);
		auto title_start = (Label*)(coverUI->getChildByName("start_menu"));
		title_start->setVisible(false);

		auto title_loading = (Label*)(coverUI->getChildByName("Loading1"));
		title_loading->setVisible(true);

		if(Loadingflag<=4)
		this->schedule(schedule_selector(CoverScene::loading_func), 0.5f);
	}

	break;
	}
}
void CoverScene::loading_func(float dt)
{
	if (Loadingflag == 1)
	{

		Loadingflag++;
	}
	else
	{
		if (Loadingflag == 2)
		{
			auto title_loading = (Label*)(coverUI->getChildByName("Loading1"));
			title_loading->setVisible(false);
			title_loading = (Label*)(coverUI->getChildByName("Loading2"));
			title_loading->setVisible(true);
			Loadingflag++;
		}
		else
		{
			if (Loadingflag == 3)
			{
				auto title_loading = (Label*)(coverUI->getChildByName("Loading2"));
				title_loading->setVisible(false);
				title_loading = (Label*)(coverUI->getChildByName("Loading3"));
				title_loading->setVisible(true);
				Loadingflag++;
			}
			else
			{
				Loadingflag = 1;
				Director::getInstance()->pushScene(CCTransitionCrossFade::create(0.5f, GameClient::createScene()));

			}
		}
		
	}
	return;
}
bool CoverScene::init()
{
	if (!Scene::init())
	{
		return false;
	}
	coverUI = GUIReader::getInstance()->widgetFromJsonFile("coverUI/coverUI.json");
	addChild(coverUI, 100);
	auto key_listener = EventListenerKeyboard::create();
	key_listener->onKeyReleased = CC_CALLBACK_2(CoverScene::onKeyReleased, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);





	return true;
}

void CoverScene::playbtnContinueCallback(Ref* pSender, Widget::TouchEventType type)
{
	if (type == ui::Widget::TouchEventType::ENDED)
		Director::getInstance()->popScene();
}