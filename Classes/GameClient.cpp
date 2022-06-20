#include "GameClient.h"
#include <algorithm>
#include <cstdlib>
#include "ui/CocosGUI.h"
using namespace cocos2d::ui;
using namespace cocostudio;

GameClient::GameClient()
{

}

GameClient::~GameClient()
{

}

bool GameClient::init()
{
	if (!Scene::init())
	{
		return false;
	}

	// һЩ����
	for (int i = 0; i < MAX_AI_NUM; i++)
	{
		m_draw[i] = DrawNode::create();
		this->addChild(m_draw[i], 2);
	}
	is_success = false;
	is_gameover = false;
	seffect_is_play = false;
	gameovertimer = 0;

	// CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(0.2f);
	// CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(0.2f);

	// ����
	m_visibleSize = Director::getInstance()->getVisibleSize();
	createBackGround();

	// ���
	m_tank = Tank::create(PLAYER_TAG, player_spawnpointX, player_spawnpointY, TANK_UP, 2);
	m_tankList.pushBack(m_tank); 
	this->addChild(m_tank);

	// �ػ�Ŀ��
	m_bird = Sprite::create("Chapter12/tank/tile.png", Rect(160, 0, 32, 32));	// �ػ�Ŀ��
	m_bird->setPosition(m_bird_spawnpointX, m_bird_spawnpointY);
	m_bird_rect = Rect(m_bird->getPositionX() - 16, m_bird->getPositionY() - 16, 32, 32);
	// m_bgList.pushBack(bird);
	this->addChild(m_bird, 2);

	// AI
	AI_init();


	// ��ײ���
	this->scheduleUpdate();

	// �����¼�
	memset(keys, 0, sizeof keys);
	auto key_listener = EventListenerKeyboard::create();
	key_listener->onKeyPressed = CC_CALLBACK_2(GameClient::onKeyPressed, this);
	key_listener->onKeyReleased = CC_CALLBACK_2(GameClient::onKeyReleased, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	//auto touch_listener = EventListenerTouchOneByOne::create();
	//touch_listener->onTouchEnded = CC_CALLBACK_2(GameClient)

	// ��������
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("minecraft.mp3", true);

	// UI
	auto gameUI = GUIReader::getInstance()->widgetFromJsonFile("gameUI/gameUI.json");
	addChild(gameUI, 100);

	auto scoreUI = GUIReader::getInstance()->widgetFromJsonFile("scoreUI/scoreUI.json");
	addChild(scoreUI, 100);


	// UI �µ�BUTTON
	Menubtn = (Button*)(gameUI->getChildByName("pause_button"));
	Replaybtn = (Button*)(gameUI->getChildByName("replay_button"));
	Pausebtn = (Button*)(gameUI->getChildByName("pause_button"));

	// add button event callback
	Menubtn->addTouchEventListener(CC_CALLBACK_2(GameClient::pressMenuButton, this));
	Replaybtn->addTouchEventListener(CC_CALLBACK_2(GameClient::pressReplayButton, this));
	Pausebtn->addTouchEventListener(CC_CALLBACK_2(GameClient::pressPauseButton, this));

	// ���üƷְ�

	// scoreUI �µ�text
	P1score = (Text*)(scoreUI->getChildByName("player1_score_label"));
	P1score->setString(std::to_string(m_score));
	//P1score->setString("123151");
	//scoreUI->getChildByName("Player2_score_text")->setVisible(false);

	// ������Ч
	if (cur_map_level == 3)
	{
		CCParticleSystem* particleSystem = CCParticleRain::create();
		particleSystem->setTexture(CCTextureCache::sharedTextureCache()->addImage("fire.png"));
		particleSystem->setStartSize(7.0);
		addChild(particleSystem);
	}


	return true;
}

Scene* GameClient::createScene()
{
	auto scene = Scene::create();
	auto layer = GameClient::create();
	scene->addChild(layer);
	return scene;
}

/****		�߼����		****/
void GameClient::update(float delta)
{
	// �ж���Ϸ����
	if (is_gameover || is_success)
	{
		gameovertimer += delta;
		if (!seffect_is_play)
		{
			seffect_is_play = true;
			CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
			if (is_success) CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Winner.mp3");
			else CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Loser.mp3");
		}
		if (gameovertimer > 8.0f)
		{
			if (is_success) success();
			else gameover();
		}
		return;
	}

	AI_update(delta);	// AI����
	invulnerable_timer += delta;	// �޵�ʱ��
	P1score->setString(std::to_string(m_score));	// ����
 
	// ̹������
	if (m_tank->getLevel() < std::min(enemy_kill / 2 + 1, MAX_LEVEL))
	{
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Upgrade.wav");	// ������Ч
		m_tank->setLevel(std::min(enemy_kill / 2 + 1, MAX_LEVEL));
		m_tank->MyDraw();
	}

	// ά��̹���б�
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		if (nowTank->getLife() <= 0)
		{
			m_deleteTankList.pushBack(nowTank);	// ����̹��
			// m_tankList.eraseObject(nowTank);
		}
	}

	// ̹���ƶ�
	char mvkey = maxValKey();
	if (mvkey == 'W') m_tank->MoveUP();
	else if (mvkey == 'A') m_tank->MoveLeft();
	else if (mvkey == 'S') m_tank->MoveDown();
	else if (mvkey == 'D') m_tank->MoveRight();

	bool tank_is_hit[20];
	memset(tank_is_hit, 0, sizeof tank_is_hit);

	// ̹���� ̹��/��Ʒ ����ײ���
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		// ̹�����ػ�Ŀ��
		if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_UP))
		{
			// ����1���Ĵ�����ת��
			nowTank->setHindered(TANK_UP);
			nowTank->setPositionY(nowTank->getPositionY() - 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
			// ����2���Ĵ�ֹͣת��
			// nowTank->Stay(TANK_UP);
		}
		else if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_DOWN))
		{
			// ����1���Ĵ�����ת��
			nowTank->setHindered(TANK_DOWN);
			nowTank->setPositionY(nowTank->getPositionY() + 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
			// ����2���Ĵ�ֹͣת��
			// nowTank->Stay(TANK_DOWN);
		}
		else if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_LEFT))
		{
			// ����1���Ĵ�����ת��
			nowTank->setHindered(TANK_LEFT);
			nowTank->setPositionX(nowTank->getPositionX() + 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
			// ����2���Ĵ�ֹͣת��
			// nowTank->Stay(TANK_LEFT);
		}
		else if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_RIGHT))
		{
			// ����1���Ĵ�����ת��
			nowTank->setHindered(TANK_RIGHT);
			nowTank->setPositionX(nowTank->getPositionX() - 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
			// ����2���Ĵ�ֹͣת��
			// nowTank->Stay(TANK_RIGHT);
		}

		// ̹����ש��
		for (int j = 0; j < m_bgList.size(); j++)
		{
			auto nowBrick = m_bgList.at(j);
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_UP))
			{
				// ����1���Ĵ�����ת��
				nowTank->setHindered(TANK_UP);
				nowTank->setPositionY(nowTank->getPositionY() - 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
				// ����2���Ĵ�ֹͣת��
				// nowTank->Stay(TANK_UP);
			}
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_DOWN))
			{
				// ����1���Ĵ�����ת��
				nowTank->setHindered(TANK_DOWN);
				nowTank->setPositionY(nowTank->getPositionY() + 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
				// ����2���Ĵ�ֹͣת��
				// nowTank->Stay(TANK_DOWN);
			}
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_LEFT))
			{
				// ����1���Ĵ�����ת��
				nowTank->setHindered(TANK_LEFT);
				nowTank->setPositionX(nowTank->getPositionX() + 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
				// ����2���Ĵ�ֹͣת��
				// nowTank->Stay(TANK_LEFT);
			}
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_RIGHT))
			{
				// ����1���Ĵ�����ת��
				nowTank->setHindered(TANK_RIGHT);
				nowTank->setPositionX(nowTank->getPositionX() - 1); // ������ɹ���̹�˳����ܣ��޷��ж���ɿ�ס
				// ����2���Ĵ�ֹͣת��
				// nowTank->Stay(TANK_RIGHT);
			}
		}

		// ̹����̹��
		for (int j = 0; j < m_tankList.size(); j++)
		{
			auto anotherTank = m_tankList.at(j);
			if ((nowTank->getLife() && anotherTank->getLife()) && (anotherTank->getID() != nowTank->getID()) && (nowTank->getRect().intersectsRect(anotherTank->getRect())))
			{
				// �����˶���̹�˲��������¶���
				if (nowTank->getDirection() == TANK_UP && nowTank->isMoving())
				{
					nowTank->Stay(TANK_UP);
				}
				if (nowTank->getDirection() == TANK_DOWN && nowTank->isMoving())
				{
					nowTank->Stay(TANK_DOWN);
				}
				if (nowTank->getDirection() == TANK_LEFT && nowTank->isMoving())
				{
					nowTank->Stay(TANK_LEFT);
				}
				if (nowTank->getDirection() == TANK_RIGHT && nowTank->isMoving())
				{
					nowTank->Stay(TANK_RIGHT);
				}
			}
		}

		// �ӵ��� ̹��/�ӵ�/ש�� ����ײ
		auto tank = m_tankList.at(i);
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);
			bool is_hit = false;

			// �ӵ����ػ�Ŀ��
			if (bullet->getRect().intersectsRect(m_bird_rect))
			{
				m_deleteBulletList.pushBack(bullet);		// �ӵ�����
				// �����ػ�Ŀ��ͼ��
				this->removeChild(m_bird);
				m_bird = Sprite::create("Chapter12/tank/tile.png", Rect(192, 0, 32, 32));	// �������ػ�Ŀ��
				m_bird->setPosition(m_bird_spawnpointX, m_bird_spawnpointY);
				this->addChild(m_bird, 2);
				is_gameover = true;
				is_hit = true;
			}

			// �ӵ���ש��
			for (int k = 0; k < m_bgList.size() && !is_hit; k++)
			{
				auto brick = m_bgList.at(k);
				if (brick->getGID() == WATER_TILE_GID) continue;		// ˮ���Դ���
				if (bullet->getRect().intersectsRect(brick->getRect()))
				{
					is_hit = true;
					m_deleteBulletList.pushBack(bullet);	// �ӵ�����
					if (brick->getGID() == WHITE_BRICK_GID && tank->getLevel() < 2) continue;		// �׿�Ҫһ���ȼ��ſ�����
					m_deleteBrickList.pushBack(brick);		// ש������
				}
			}

			// �ӵ���̹�� / �ӵ�
			for (int k = 0; k < m_tankList.size() && !is_hit; k++)		// tank�ӵ�������is_hitΪtrue
			{
				// �ӵ���̹��
				if (tank_is_hit[k]) continue;	// ��̹���ѱ������ӵ�����
				auto tank_another = m_tankList.at(k);
				if (tank->getID() == tank_another->getID()) continue;
				// AI��AI���໥Ӱ��
				if (tank->getID() != PLAYER_TAG && tank_another->getID() != PLAYER_TAG) continue;

				if (bullet->getRect().intersectsRect(tank_another->getRect()))
				{
					m_deleteBulletList.pushBack(bullet);		// �ӵ�����
					int tank_another_ID = tank_another->getID();
					if (tank_another_ID == PLAYER_TAG && invulnerable_timer > 3.0f)
					{
						m_deleteTankList.pushBack(tank_another); 	// ����޵�ʱ����˻�������̹�ˣ�ֱ������
						invulnerable_timer = 0;
					}
					else if (tank_another_ID != PLAYER_TAG)
					{
						m_deleteTankList.pushBack(tank_another);
						enemy_kill++;
					}
					tank_is_hit[k] = true;
					is_hit = true;	// tank������ӵ��Ѿ�����
				}

				if (k < i) continue;	// ��ֹ�����ӵ�����ʱ���ж�����

				// �ӵ����ӵ�
				for (int t = 0; t < tank_another->getBulletList().size() && !is_hit; t++)
				{
					auto bullet_another = tank_another->getBulletList().at(t);
					if (bullet->getRect().intersectsRect(bullet_another->getRect()))
					{
						m_deleteBulletList.pushBack(bullet);				// �ӵ�����
						m_deleteBulletList.pushBack(bullet_another);		// �ӵ�����
						is_hit = true;
					}
				}
			}
		}

		// ���ɾ���ӵ��б�
		while (m_deleteBulletList.size())
		{
			auto bullet = m_deleteBulletList.at(0);
			bullet->Blast();
			m_deleteBulletList.eraseObject(bullet);
			tank->getBulletList().eraseObject(bullet);
		}

		// ���ɾ��ש���б�
		while (m_deleteBrickList.size())
		{
			auto brick = m_deleteBrickList.at(0);
			brick->Blast();
			// ������Ч
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Block_destroyed.wav");
			// ɾ���ķ����Ӧ��mapnode״̬Ҫ��Ϊ��ͨ��
			int x = brick->getPositionX(), y = brick->getPositionY();
			m_map[x2i(x)][y2j(y)].status = ACCESS;	
			m_deleteBrickList.eraseObject(brick);
			m_bgList.eraseObject(brick);
		}

		// ���ɾ��̹���б�
		while (m_deleteTankList.size())
		{
			auto tank = m_deleteTankList.at(0);
			tank->Blast();
			if (tank->getID() != PLAYER_TAG)
			{
				// ������Ч
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Battle City SFX (13).wav");
				AI_ingame_num--;		// �����AI���������Ϸ��AI����
				AI_remain_num--;		// �����AI������ʣ��AI����	
				add_score(m_tank->getLevel() * 10);				// �ӷ�
				// AIȫ��ɱ�꣬��Ϸʤ��
				if (AI_remain_num == 0) 
					is_success = true;	
			}
			else if (tank->getID() == PLAYER_TAG)
			{
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Died.wav");	// ������Ч
				if (--player_life > 0)
				{
					m_tank = Tank::create(PLAYER_TAG, player_spawnpointX, player_spawnpointY, TANK_UP, 2);
					m_tankList.pushBack(m_tank);
					enemy_kill = 0;
					this->addChild(m_tank);
				}
				else is_gameover = true;	// �����������Ϸ����
			}
			m_deleteTankList.eraseObject(tank);
			m_tankList.eraseObject(tank);
		}

		m_deleteBulletList.clear();
		m_deleteBrickList.clear();
		m_deleteTankList.clear();
	}
}

void GameClient::success()
{
	this->unscheduleUpdate();
	this->unscheduleAllCallbacks();
	this->unscheduleAllSelectors();
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto tank = m_tankList.at(i);
		tank->unscheduleUpdate();
		tank->stopAllActions();
	}

	if(cur_map_level == 3)
		// ��Ϸʤ��
		Director::getInstance()->replaceScene(CCTransitionCrossFade::create(0.5f, VictoryScene::createScene()));
	else
	{
		// ��һ��
		cur_map_level++;
		Director::getInstance()->popScene();
	}
}

void GameClient::gameover()
{
	// ��Ϸ����
	this->unscheduleUpdate();
	this->unscheduleAllCallbacks();
	this->unscheduleAllSelectors();

	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto tank = m_tankList.at(i);
		tank->unscheduleUpdate();
		tank->stopAllActions();
	}

	// ��ӡ��You Lose!��
	cocos2d::Label* tip;
	tip = Label::createWithTTF("You Lose!", "fonts/Marker Felt.ttf", 48);
	tip->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2 + 120));
	this->addChild(tip, 10);

	// ����ReplayButton��ReplayLabel
	auto ReplayButton = Button::create("btn_normal.png", "btn_pressed.png");
	auto ReplayLabel = Label::createWithTTF("Replay", "fonts/Marker Felt.ttf", 48);
	ReplayButton->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2));
	ReplayLabel->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2));
	this->addChild(ReplayButton, 10);
	this->addChild(ReplayLabel, 11);

	// Replay��ť����
	ReplayButton->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type)
		{
			switch (type)
			{
			case ui::Widget::TouchEventType::BEGAN:
				break;
			case ui::Widget::TouchEventType::ENDED:
				log("ReplayButton is clicked!");
				// ���뽥��ת��
				cur_map_level = 1;
				m_score = 0;
				Director::getInstance()->popScene();
				break;
			default:
				break;
			}
		}
	);

	// ���Quit��ť
	auto QuitButton = Button::create("btn_normal.png", "btn_pressed.png");
	auto QuitLabel = Label::createWithTTF("Quit", "fonts/Marker Felt.ttf", 48);
	QuitButton->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2 - 80));
	QuitLabel->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2 - 80));
	this->addChild(QuitButton, 10);
	this->addChild(QuitLabel, 11);

	// Quit��ť����
	QuitButton->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type)
		{
			switch (type)
			{
			case ui::Widget::TouchEventType::BEGAN:
				break;
			case ui::Widget::TouchEventType::ENDED:
				log("QuitButton is clicked!");
				exit(0);	// �˳���Ϸ
				break;
			default:
				break;
			}
		}
	);
}
/****						****/


/****		A* �㷨���			****/
int GameClient::aStar(mapNode** map, mapNode* origin, mapNode* destination, int tag)
{
	openList* open = new openList;
	open->next = nullptr;
	open->openNode = origin;
	closedList* close = new closedList;
	close->next = nullptr;
	close->closedNode = nullptr;

	// aStar_offset = (tag - AI_TAG) % 3;
	//ѭ������4����������ڽڵ�
	while (checkNeighboringNodes(map, open, open->openNode, destination))
	{
		//��OPEN����ѡȡ�ڵ����CLOSED��
		insertNodeToClosedList(close, open);
		// Ѱ·ʧ��
		if (open == nullptr)
		{
			log("No Way!\n");
			break;
		}
		//���յ���OPEN���У�����Ѱ·�ɹ�
		if (open->openNode->status == DESTINATION)
		{
			mapNode* tempNode = open->openNode;
			//����moveOnPath�����������ƾ�����·�����ƶ�
			moveOnPath(tempNode, tag);
			break;
		}
	}
	return 0;
}
void GameClient::moveOnPath(mapNode* tempNode, int tag)
{
	//�����洢·������Ľṹ��
	struct pathCoordinate { int x; int y; };
	//����·������ṹ������
	pathCoordinate* path = new pathCoordinate[MAP_WIDTH * MAP_HEIGHT];
	//���ø��ڵ���Ϣ����洢·������
	int loopNum = 0;
	while (tempNode != nullptr)
	{
		path[loopNum].x = tempNode->xCoordinate;
		path[loopNum].y = tempNode->yCoordinate;
		loopNum++;
		tempNode = tempNode->parent;
	}
	//��Ц������������Ϊ�����߶����
	auto tank = (Tank*)this->getChildByTag(tag);
	int fromX = tank->getPositionX();
	int fromY = tank->getPositionY();
	//�������������洢��������
	Vector<FiniteTimeAction*> actionVector;
	//�ӽṹ������β����ʼɨ��
	int dir = tank->getDirection();
	for (int j = loopNum - 2; j >= 0; j--)
	{
		//����ͼ��������ת��Ϊ��Ļʵ������
		int realX = (path[j].x + 0.5) * UNIT;
		int realY = m_visibleSize.height - (path[j].y + 0.5) * UNIT;

		// ת��
		if (realX - fromX > 0 && dir != TANK_RIGHT)
		{
			dir = TANK_RIGHT;
			actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnRight, this, tank)));
		}
		else if (realX - fromX < 0 && dir != TANK_LEFT)
		{
			dir = TANK_LEFT;
			actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnLeft, this, tank)));
		}
		else if (realY - fromY > 0 && dir != TANK_UP)
		{
			dir = TANK_UP;
			actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnUp, this, tank)));

		}
		else if (realY - fromY < 0 && dir != TANK_DOWN)
		{
			dir = TANK_DOWN;
			actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnDown, this, tank)));
		}
		//�����ƶ����������붯������
		auto moveAction = MoveTo::create(0.2, Vec2(realX, realY));
		actionVector.pushBack(moveAction);

		//���ƴ���㵽��һ����ͼ��Ԫ���߶�
		m_draw[tag - AI_TAG]->drawLine(Vec2(fromX, fromY), Vec2(realX, realY), Color4F(1.0, 1.0, 1.0, 1.0));
		//����ǰ���걣��Ϊ��һ�λ��Ƶ����
		fromX = realX;
		fromY = realY;
	}

	if((tag - AI_TAG) % MAX_INGAME_AI_NUM == 0)
		actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnRight, this, tank)));
	else if ((tag - AI_TAG) % MAX_INGAME_AI_NUM == 1)
		actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnDown, this, tank)));
	else if ((tag - AI_TAG) % MAX_INGAME_AI_NUM == 2)
		actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnLeft, this, tank)));

	//������������
	auto actionSequence = Sequence::create(actionVector);
	//Ц������ִ���ƶ���������
	tank->runAction(actionSequence);
}
// ����m_map_t��ֵ����ֹ��ͬAI��A*�㷨�໥Ӱ��
void GameClient::mapcopy()
{
	for (int i = 0; i < MAP_WIDTH; i++)
		for (int j = 0; j < MAP_HEIGHT; j++)
			m_map_t[i][j] = m_map[i][j];
}
/****							****/


/****		AI���			****/
void GameClient::AI_init()
{
	AI_update_delta = 0;
	AI_next_offset = 0;
	AI_remain_num = MAX_AI_NUM;
	AI_ingame_num = 0;
	/*for (int i = 0; i < MAX_INGAME_AI_NUM; i++)
	{
		m_map[AI_destinationX[i]][AI_destinationY[i]].status = DESTINATION_1 + i;
		m_destination[i] = &m_map[AI_destinationX[i]][AI_destinationY[i]];
	}*/
}

void GameClient::AI_fill()
{
	// if (AI_remain_num < MAX_INGAME_AI_NUM) return;	// ʣ��AI��������

	// ����AI��������������ֱ��������û��AI
	while (AI_ingame_num < MAX_INGAME_AI_NUM && AI_ingame_num < AI_remain_num)
	{
		auto AI_tank = Tank::create(AI_TAG + AI_next_offset, AI_spawnpointX[AI_next_offset % MAX_INGAME_AI_NUM], AI_spawnpointY, 1, 1);
		AI_tank->setTag(AI_TAG + AI_next_offset);
		m_tankList.pushBack(AI_tank);
		AI_next_offset++;
		AI_ingame_num++;
		this->addChild(AI_tank);

		// A*�㷨Ѱ·
		int x = AI_tank->getPositionX(), y = AI_tank->getPositionY();
		// log("%d %d %d %d\n", x, y, x2i(x), y2j(y));
		mapcopy();	// ����m_map_t����ֹ��ͬAI��A*�㷨�໥Ӱ��f��g��ֵ

		int idx = (AI_next_offset - 1) % MAX_INGAME_AI_NUM;
		auto m_origin = &m_map_t[x2i(x)][y2j(y)];
		m_map_t[AI_destinationX[idx]][AI_destinationY[idx]].status = DESTINATION;
		m_destination[idx] = &m_map_t[AI_destinationX[idx]][AI_destinationY[idx]];
		aStar(m_map_t, m_origin, m_destination[idx], AI_tank->getID());
	}
}

void GameClient::AI_update(float delta)
{
	AI_update_delta += delta;
	if (AI_update_delta > 1.0f)
	{
		AI_update_delta = 0;
		// ����AI
		AI_fill();
		// �Զ�����
		for (int i = 0; i < m_tankList.size(); i++)
		{
			auto tank = m_tankList.at(i);
			if (tank->getID() == PLAYER_TAG) continue;
			if(tank->getID() == AI_TAG)	
				log("%lf %lf\n", tank->getPositionX(), tank->getPositionY());

			tank->Fire();
		}
	}
}
/****						****/


// ��ʼ����ͼ
void GameClient::createBackGround()
{
	auto map = TMXTiledMap::create("Chapter12/tank/map.tmx");
	// ����������
	for (int i = 1; i <= MAX_MAP_LEVEL; i++)
		map->getLayer("brick" + std::to_string(i))->setVisible(false);
	
	m_mapLayer = map->getLayer("brick" + std::to_string(cur_map_level));
	// m_mapLayer->setVisible(false);
	this->addChild(map, 10);

	//���ݵ�ͼ���߷�������ռ�
	m_map = new mapNode * [MAP_WIDTH];
	m_map_t = new mapNode * [MAP_WIDTH];
	for (int i = 0; i < MAP_WIDTH; i++)
	{
		m_map[i] = new mapNode[MAP_HEIGHT];
		m_map_t[i] = new mapNode[MAP_HEIGHT];
	}
	//����ɨ���ͼ����ÿһ����Ԫ
	for (int i = 0; i < MAP_WIDTH; i++)
	{
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			//����ǰλ��Ϊǽ����Ƭ����Ϊ����ͨ��
			int gid = m_mapLayer->getTileGIDAt(Vec2(i, j));
			if (gid == 1 || gid == 3 || gid == 7)
			{
				// �Ը÷���Ϊ���ĵ�9���񶼲��ɵ���
				int dx[9] = { 0, 0, 1, 0, -1, 1, 1, -1 ,-1 };
				int dy[9] = { 0, 1, 0, -1, 0, 1, -1, 1, -1 };
				for (int k = 0; k < 9; k++)
				{
					int ti = i + dx[k], tj = j + dy[k];
					mapNode temp = { NOT_ACCESS, ti, tj, 0, 0, 0, nullptr };
					if(ti < MAP_WIDTH && tj < MAP_HEIGHT)
						m_map[ti][tj] = temp;
				}
				// ���ש��
				auto brick = Brick::create(Vec2(i2x(i), j2y(j), gid);
				m_bgList.pushBack(brick);
				this->addChild(brick, 2);
			}
			//��������Ϊ����ͨ��
			else if(m_map[i][j].status != NOT_ACCESS)
			{
				mapNode temp = { ACCESS, i, j, 0, 0, 0, nullptr };
				m_map[i][j] = temp;
			}
		}
	}
}


/****		�������		****/
char GameClient::maxValKey()
{
	char ch = ' ';
	if (keys['W'] > keys[ch]) ch = 'W';
	if (keys['A'] > keys[ch]) ch = 'A';
	if (keys['S'] > keys[ch]) ch = 'S';
	if (keys['D'] > keys[ch]) ch = 'D';
	return ch;
}
void GameClient::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		{
			keys['A'] = keys[maxValKey()] + 1;
			//m_tank->MoveLeft();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		{
			keys['W'] = keys[maxValKey()] + 1;
			//m_tank->MoveUP();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		{
			keys['S'] = keys[maxValKey()] + 1;
			//m_tank->MoveDown();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		{
			keys['D'] = keys[maxValKey()] + 1;
			//m_tank->MoveRight();
		}
		break;
	}
}
void GameClient::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		{
			keys['A'] = 0;
			m_tank->Stay(TANK_LEFT);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		{
			keys['W'] = 0;
			m_tank->Stay(TANK_UP);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		{
			keys['S'] = 0;
			m_tank->Stay(TANK_DOWN);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		{
			keys['D'] = 0;
			m_tank->Stay(TANK_RIGHT);
		}
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_K:
	{
		m_tank->Fire();
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F:
	{
		m_tank->Flash();		// �����ƶ���ʽ������
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_L:
	{
		m_tank->Fire_high();	// ���ͽ�����ʽ
	}
	break;
	}
}
/****						****/


/****		�������		****/
void GameClient::pressPauseButton(Ref* pSender, Widget::TouchEventType type)
{
	Size visibleSize = Director::getInstance()->getVisibleSize();
	RenderTexture* renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height);

	//������ǰ��������ӽڵ���Ϣ������renderTexture�С�
	//�������ƽ�ͼ��
	renderTexture->begin();
	this->getParent()->visit();
	renderTexture->end();

	Director::getInstance()->pushScene(CCTransitionCrossFade::create(0.5f, Gamepause::createScene()));
	//if (type == ui::Widget::TouchEventType::ENDED)
	//	this->unscheduleUpdate();
}
void GameClient::pressMenuButton(Ref* pSender, Widget::TouchEventType type)
{

}
void GameClient::pressReplayButton(Ref* pSender, Widget::TouchEventType type)
{
	if (type == ui::Widget::TouchEventType::ENDED)
	{
		//reset score board
		m_score = 0;
		cur_map_level = 1;
		Director::getInstance()->popScene();
		// Director::getInstance()->pushScene(CCTransitionCrossFade::create(0.5f, this->createScene()));
	}
}
/****						****/