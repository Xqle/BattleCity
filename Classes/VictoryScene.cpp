#include"VictoryScene.h"


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

	// auto key_listener = EventListenerKeyboard::create();
	//key_listener->onKeyReleased = CC_CALLBACK_2(LoadingScene::onKeyReleased, this);
	// Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	auto title_loading = (ImageView*)(LoadingUI->getChildByName("star3"));
	title_loading->setVisible(false);
	title_loading = (ImageView*)(LoadingUI->getChildByName("star1"));
	title_loading->setVisible(false);
	title_loading = (ImageView*)(LoadingUI->getChildByName("star2"));
	title_loading->setVisible(false);

	auto total_score = (Text*)(LoadingUI->getChildByName("total_score_text"));
	total_score->setString(std::to_string(m_score));

	addChild(LoadingUI, 100);
	victory_flag = 1;
	if (victory_flag < 5)
		this->schedule(schedule_selector(VictoryScene::loading_func), 1.0f);

	return true;
}

void VictoryScene::loading_func(float dt)
{
	if (victory_flag == 1)
	{
		victory_flag = 2;
		auto title_loading = (ImageView*)(LoadingUI->getChildByName("star1"));
		title_loading->setVisible(true);
	}
	else
	{
		if (victory_flag == 2 && m_score >= 480)
		{
			victory_flag = 3;
			//auto title_loading = (ImageView*)(LoadingUI->getChildByName("star1"));
			//title_loading->setVisible(false);
			auto title_loading = (ImageView*)(LoadingUI->getChildByName("star2"));
			title_loading->setVisible(true);
		}
		else
		{
			if (victory_flag == 3 && m_score >= 640)
			{
				victory_flag = 4;
				//auto title_loading = (ImageView*)(LoadingUI->getChildByName("star2"));
				//title_loading->setVisible(false);
				auto title_loading = (ImageView*)(LoadingUI->getChildByName("star3"));
				title_loading->setVisible(true);
			}
		}
	}
}