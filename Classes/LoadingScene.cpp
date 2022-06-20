#include "LoadingScene.h"

LoadingScene::LoadingScene()
{

}
LoadingScene::~LoadingScene()
{

}

Scene* LoadingScene::createScene()
{
	auto scene = Scene::create();
	auto layer = LoadingScene::create();
	scene->addChild(layer);
	return scene;
}

bool LoadingScene::init()
{
	if (!Scene::init())
	{
		return false;
	}
	LoadingUI = GUIReader::getInstance()->widgetFromJsonFile("loadingUI/loadingUI.json");
	addChild(LoadingUI, 100);
	//auto key_listener = EventListenerKeyboard::create();
	//key_listener->onKeyReleased = CC_CALLBACK_2(LoadingScene::onKeyReleased, this);
	//Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	loading_flag = 1;
	if (loading_flag < 5)
		this->schedule(schedule_selector(LoadingScene::loading_func), 0.5f);

	return true;
}

void LoadingScene::loading_func(float dt)
{
	if (loading_flag == 1)
	{
		loading_flag = 2;
		auto title_loading = (Label*)(LoadingUI->getChildByName("Loading1"));
		title_loading->setVisible(true);
	}
	else
	{
		if (loading_flag == 2)
		{
			loading_flag = 3;
			auto title_loading = (Label*)(LoadingUI->getChildByName("Loading1"));
			title_loading->setVisible(false);
			title_loading = (Label*)(LoadingUI->getChildByName("Loading2"));
			title_loading->setVisible(true);
		}
		else
		{
			if (loading_flag == 3)
			{
				loading_flag = 4;
				auto title_loading = (Label*)(LoadingUI->getChildByName("Loading2"));
				title_loading->setVisible(false);
				title_loading = (Label*)(LoadingUI->getChildByName("Loading3"));
				title_loading->setVisible(true);
			}
			else
			{
				Director::getInstance()->pushScene(CCTransitionCrossFade::create(0.3f, GameClient::createScene()));
				loading_flag = 1; 
				auto title_loading = (Label*)(LoadingUI->getChildByName("Loading3"));
				title_loading->setVisible(false);
			}
		}
	}
}