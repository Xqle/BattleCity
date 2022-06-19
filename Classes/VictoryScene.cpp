#include"VictoryScene.h"


int loading_flag = 1;
VictoryScene::VictoryScene()
{

}
VictoryScene::~VictoryScene()
{

}

Scene* VictoryScene::createScene()
{
	auto scene = Scene::create();
	auto layer = VictoryScene::create();
	scene->addChild(layer);
	return scene;
}

bool VictoryScene::init()
{
	if (!Scene::init())
	{
		return false;
	}
	LoadingUI = GUIReader::getInstance()->widgetFromJsonFile("victoryUI/victoryUI.json");
	addChild(LoadingUI, 100);
	auto key_listener = EventListenerKeyboard::create();
	//key_listener->onKeyReleased = CC_CALLBACK_2(LoadingScene::onKeyReleased, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	if (loading_flag < 5)
		this->schedule(schedule_selector(VictoryScene::loading_func), 0.1f);

	loading_flag = 1;
	return true;
}

void VictoryScene::loading_func(float dt)
{
	if (loading_flag == 1)
	{
		loading_flag = 2;
		auto title_loading = (ImageView*)(LoadingUI->getChildByName("star1"));
		title_loading->setVisible(true);
	}
	else
	{
		if (loading_flag == 2)
		{
			loading_flag = 3;
			auto title_loading = (ImageView*)(LoadingUI->getChildByName("star1"));
			title_loading->setVisible(false);
			title_loading = (ImageView*)(LoadingUI->getChildByName("star2"));
			title_loading->setVisible(true);
		}
		else
		{
			if (loading_flag == 3)
			{
				loading_flag = 4;
				auto title_loading = (ImageView*)(LoadingUI->getChildByName("star2"));
				title_loading->setVisible(false);
				title_loading = (ImageView*)(LoadingUI->getChildByName("star3"));
				title_loading->setVisible(true);
			}
			else
			{
				Director::getInstance()->replaceScene(CCTransitionCrossFade::create(0.3f, GameClient::createScene()));
			}
		}
	}
}