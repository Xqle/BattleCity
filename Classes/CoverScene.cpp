#pragma once
#include"CoverScene.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

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
		Director::getInstance()->pushScene(LoadingScene::createScene());
	}

	break;
	}
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