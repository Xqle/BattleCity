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

	// 一些变量
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

	// 背景
	m_visibleSize = Director::getInstance()->getVisibleSize();
	createBackGround();

	// 玩家
	m_tank = Tank::create(PLAYER_TAG, player_spawnpointX, player_spawnpointY, TANK_UP, 2);
	m_tankList.pushBack(m_tank); 
	this->addChild(m_tank);

	// 守护目标
	m_bird = Sprite::create("Chapter12/tank/tile.png", Rect(160, 0, 32, 32));	// 守护目标
	m_bird->setPosition(m_bird_spawnpointX, m_bird_spawnpointY);
	m_bird_rect = Rect(m_bird->getPositionX() - 16, m_bird->getPositionY() - 16, 32, 32);
	// m_bgList.pushBack(bird);
	this->addChild(m_bird, 2);

	// AI
	AI_init();


	// 碰撞检测
	this->scheduleUpdate();

	// 键盘事件
	memset(keys, 0, sizeof keys);
	auto key_listener = EventListenerKeyboard::create();
	key_listener->onKeyPressed = CC_CALLBACK_2(GameClient::onKeyPressed, this);
	key_listener->onKeyReleased = CC_CALLBACK_2(GameClient::onKeyReleased, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	//auto touch_listener = EventListenerTouchOneByOne::create();
	//touch_listener->onTouchEnded = CC_CALLBACK_2(GameClient)

	// 背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("minecraft.mp3", true);

	// UI
	auto gameUI = GUIReader::getInstance()->widgetFromJsonFile("gameUI/gameUI.json");
	addChild(gameUI, 100);

	auto scoreUI = GUIReader::getInstance()->widgetFromJsonFile("scoreUI/scoreUI.json");
	addChild(scoreUI, 100);


	// UI 下的BUTTON
	Menubtn = (Button*)(gameUI->getChildByName("pause_button"));
	Replaybtn = (Button*)(gameUI->getChildByName("replay_button"));
	Pausebtn = (Button*)(gameUI->getChildByName("pause_button"));

	// add button event callback
	Menubtn->addTouchEventListener(CC_CALLBACK_2(GameClient::pressMenuButton, this));
	Replaybtn->addTouchEventListener(CC_CALLBACK_2(GameClient::pressReplayButton, this));
	Pausebtn->addTouchEventListener(CC_CALLBACK_2(GameClient::pressPauseButton, this));

	// 重置计分板

	// scoreUI 下的text
	P1score = (Text*)(scoreUI->getChildByName("player1_score_label"));
	P1score->setString(std::to_string(m_score));
	//P1score->setString("123151");
	//scoreUI->getChildByName("Player2_score_text")->setVisible(false);

	// 粒子特效
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

/****		逻辑相关		****/
void GameClient::update(float delta)
{
	// 判断游戏结束
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

	AI_update(delta);	// AI补充
	invulnerable_timer += delta;	// 无敌时间
	P1score->setString(std::to_string(m_score));	// 分数
 
	// 坦克升级
	if (m_tank->getLevel() < std::min(enemy_kill / 2 + 1, MAX_LEVEL))
	{
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Upgrade.wav");	// 升级音效
		m_tank->setLevel(std::min(enemy_kill / 2 + 1, MAX_LEVEL));
		m_tank->MyDraw();
	}

	// 维护坦克列表
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		if (nowTank->getLife() <= 0)
		{
			m_deleteTankList.pushBack(nowTank);	// 消除坦克
			// m_tankList.eraseObject(nowTank);
		}
	}

	// 坦克移动
	char mvkey = maxValKey();
	if (mvkey == 'W') m_tank->MoveUP();
	else if (mvkey == 'A') m_tank->MoveLeft();
	else if (mvkey == 'S') m_tank->MoveDown();
	else if (mvkey == 'D') m_tank->MoveRight();

	bool tank_is_hit[20];
	memset(tank_is_hit, 0, sizeof tank_is_hit);

	// 坦克与 坦克/物品 的碰撞检测
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		// 坦克与守护目标
		if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_UP))
		{
			// 方法1：履带持续转动
			nowTank->setHindered(TANK_UP);
			nowTank->setPositionY(nowTank->getPositionY() - 1); // 避免检测成功后坦克持续受，无法行动造成卡住
			// 方法2：履带停止转动
			// nowTank->Stay(TANK_UP);
		}
		else if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_DOWN))
		{
			// 方法1：履带持续转动
			nowTank->setHindered(TANK_DOWN);
			nowTank->setPositionY(nowTank->getPositionY() + 1); // 避免检测成功后坦克持续受，无法行动造成卡住
			// 方法2：履带停止转动
			// nowTank->Stay(TANK_DOWN);
		}
		else if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_LEFT))
		{
			// 方法1：履带持续转动
			nowTank->setHindered(TANK_LEFT);
			nowTank->setPositionX(nowTank->getPositionX() + 1); // 避免检测成功后坦克持续受，无法行动造成卡住
			// 方法2：履带停止转动
			// nowTank->Stay(TANK_LEFT);
		}
		else if (nowTank->getLife() && (nowTank->getRect().intersectsRect(m_bird_rect)) && (nowTank->getDirection() == TANK_RIGHT))
		{
			// 方法1：履带持续转动
			nowTank->setHindered(TANK_RIGHT);
			nowTank->setPositionX(nowTank->getPositionX() - 1); // 避免检测成功后坦克持续受，无法行动造成卡住
			// 方法2：履带停止转动
			// nowTank->Stay(TANK_RIGHT);
		}

		// 坦克与砖块
		for (int j = 0; j < m_bgList.size(); j++)
		{
			auto nowBrick = m_bgList.at(j);
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_UP))
			{
				// 方法1：履带持续转动
				nowTank->setHindered(TANK_UP);
				nowTank->setPositionY(nowTank->getPositionY() - 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				// 方法2：履带停止转动
				// nowTank->Stay(TANK_UP);
			}
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_DOWN))
			{
				// 方法1：履带持续转动
				nowTank->setHindered(TANK_DOWN);
				nowTank->setPositionY(nowTank->getPositionY() + 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				// 方法2：履带停止转动
				// nowTank->Stay(TANK_DOWN);
			}
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_LEFT))
			{
				// 方法1：履带持续转动
				nowTank->setHindered(TANK_LEFT);
				nowTank->setPositionX(nowTank->getPositionX() + 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				// 方法2：履带停止转动
				// nowTank->Stay(TANK_LEFT);
			}
			if (nowTank->getLife() && (nowTank->getRect().intersectsRect(nowBrick->getRect())) && (nowTank->getDirection() == TANK_RIGHT))
			{
				// 方法1：履带持续转动
				nowTank->setHindered(TANK_RIGHT);
				nowTank->setPositionX(nowTank->getPositionX() - 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				// 方法2：履带停止转动
				// nowTank->Stay(TANK_RIGHT);
			}
		}

		// 坦克与坦克
		for (int j = 0; j < m_tankList.size(); j++)
		{
			auto anotherTank = m_tankList.at(j);
			if ((nowTank->getLife() && anotherTank->getLife()) && (anotherTank->getID() != nowTank->getID()) && (nowTank->getRect().intersectsRect(anotherTank->getRect())))
			{
				// 正在运动的坦克才作出如下动作
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

		// 子弹与 坦克/子弹/砖块 的碰撞
		auto tank = m_tankList.at(i);
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);
			bool is_hit = false;

			// 子弹与守护目标
			if (bullet->getRect().intersectsRect(m_bird_rect))
			{
				m_deleteBulletList.pushBack(bullet);		// 子弹消除
				// 更换守护目标图像
				this->removeChild(m_bird);
				m_bird = Sprite::create("Chapter12/tank/tile.png", Rect(192, 0, 32, 32));	// 死亡的守护目标
				m_bird->setPosition(m_bird_spawnpointX, m_bird_spawnpointY);
				this->addChild(m_bird, 2);
				is_gameover = true;
				is_hit = true;
			}

			// 子弹与砖块
			for (int k = 0; k < m_bgList.size() && !is_hit; k++)
			{
				auto brick = m_bgList.at(k);
				if (brick->getGID() == WATER_TILE_GID) continue;		// 水可以穿过
				if (bullet->getRect().intersectsRect(brick->getRect()))
				{
					is_hit = true;
					m_deleteBulletList.pushBack(bullet);	// 子弹消除
					if (brick->getGID() == WHITE_BRICK_GID && tank->getLevel() < 2) continue;		// 白块要一定等级才可命中
					m_deleteBrickList.pushBack(brick);		// 砖块消除
				}
			}

			// 子弹与坦克 / 子弹
			for (int k = 0; k < m_tankList.size() && !is_hit; k++)		// tank子弹击中则is_hit为true
			{
				// 子弹与坦克
				if (tank_is_hit[k]) continue;	// 该坦克已被其他子弹命中
				auto tank_another = m_tankList.at(k);
				if (tank->getID() == tank_another->getID()) continue;
				// AI和AI不相互影响
				if (tank->getID() != PLAYER_TAG && tank_another->getID() != PLAYER_TAG) continue;

				if (bullet->getRect().intersectsRect(tank_another->getRect()))
				{
					m_deleteBulletList.pushBack(bullet);		// 子弹消除
					int tank_another_ID = tank_another->getID();
					if (tank_another_ID == PLAYER_TAG && invulnerable_timer > 3.0f)
					{
						m_deleteTankList.pushBack(tank_another); 	// 玩家无敌时间过了或者其他坦克，直接消除
						invulnerable_timer = 0;
					}
					else if (tank_another_ID != PLAYER_TAG)
					{
						m_deleteTankList.pushBack(tank_another);
						enemy_kill++;
					}
					tank_is_hit[k] = true;
					is_hit = true;	// tank的这颗子弹已经打完
				}

				if (k < i) continue;	// 防止两颗子弹命中时被判断两次

				// 子弹与子弹
				for (int t = 0; t < tank_another->getBulletList().size() && !is_hit; t++)
				{
					auto bullet_another = tank_another->getBulletList().at(t);
					if (bullet->getRect().intersectsRect(bullet_another->getRect()))
					{
						m_deleteBulletList.pushBack(bullet);				// 子弹消除
						m_deleteBulletList.pushBack(bullet_another);		// 子弹消除
						is_hit = true;
					}
				}
			}
		}

		// 清除删除子弹列表
		while (m_deleteBulletList.size())
		{
			auto bullet = m_deleteBulletList.at(0);
			bullet->Blast();
			m_deleteBulletList.eraseObject(bullet);
			tank->getBulletList().eraseObject(bullet);
		}

		// 清除删除砖块列表
		while (m_deleteBrickList.size())
		{
			auto brick = m_deleteBrickList.at(0);
			brick->Blast();
			// 播放音效
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Block_destroyed.wav");
			// 删除的方块对应的mapnode状态要改为可通过
			int x = brick->getPositionX(), y = brick->getPositionY();
			m_map[x2i(x)][y2j(y)].status = ACCESS;	
			m_deleteBrickList.eraseObject(brick);
			m_bgList.eraseObject(brick);
		}

		// 清除删除坦克列表
		while (m_deleteTankList.size())
		{
			auto tank = m_deleteTankList.at(0);
			tank->Blast();
			if (tank->getID() != PLAYER_TAG)
			{
				// 播放音效
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Battle City SFX (13).wav");
				AI_ingame_num--;		// 如果是AI，则减少游戏内AI数量
				AI_remain_num--;		// 如果是AI，减少剩余AI数量	
				add_score(m_tank->getLevel() * 10);				// 加分
				// AI全部杀完，游戏胜利
				if (AI_remain_num == 0) 
					is_success = true;	
			}
			else if (tank->getID() == PLAYER_TAG)
			{
				CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("SoundEffect/Died.wav");	// 播放音效
				if (--player_life > 0)
				{
					m_tank = Tank::create(PLAYER_TAG, player_spawnpointX, player_spawnpointY, TANK_UP, 2);
					m_tankList.pushBack(m_tank);
					enemy_kill = 0;
					this->addChild(m_tank);
				}
				else is_gameover = true;	// 玩家死亡，游戏结束
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
		// 游戏胜利
		Director::getInstance()->replaceScene(CCTransitionCrossFade::create(0.5f, VictoryScene::createScene()));
	else
	{
		// 下一关
		cur_map_level++;
		Director::getInstance()->popScene();
	}
}

void GameClient::gameover()
{
	// 游戏结束
	this->unscheduleUpdate();
	this->unscheduleAllCallbacks();
	this->unscheduleAllSelectors();

	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto tank = m_tankList.at(i);
		tank->unscheduleUpdate();
		tank->stopAllActions();
	}

	// 打印“You Lose!”
	cocos2d::Label* tip;
	tip = Label::createWithTTF("You Lose!", "fonts/Marker Felt.ttf", 48);
	tip->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2 + 120));
	this->addChild(tip, 10);

	// 生成ReplayButton和ReplayLabel
	auto ReplayButton = Button::create("btn_normal.png", "btn_pressed.png");
	auto ReplayLabel = Label::createWithTTF("Replay", "fonts/Marker Felt.ttf", 48);
	ReplayButton->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2));
	ReplayLabel->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2));
	this->addChild(ReplayButton, 10);
	this->addChild(ReplayLabel, 11);

	// Replay按钮监听
	ReplayButton->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type)
		{
			switch (type)
			{
			case ui::Widget::TouchEventType::BEGAN:
				break;
			case ui::Widget::TouchEventType::ENDED:
				log("ReplayButton is clicked!");
				// 渐入渐出转场
				cur_map_level = 1;
				m_score = 0;
				Director::getInstance()->popScene();
				break;
			default:
				break;
			}
		}
	);

	// 添加Quit按钮
	auto QuitButton = Button::create("btn_normal.png", "btn_pressed.png");
	auto QuitLabel = Label::createWithTTF("Quit", "fonts/Marker Felt.ttf", 48);
	QuitButton->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2 - 80));
	QuitLabel->setPosition(Vec2(m_visibleSize.width / 2, m_visibleSize.height / 2 - 80));
	this->addChild(QuitButton, 10);
	this->addChild(QuitLabel, 11);

	// Quit按钮监听
	QuitButton->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type)
		{
			switch (type)
			{
			case ui::Widget::TouchEventType::BEGAN:
				break;
			case ui::Widget::TouchEventType::ENDED:
				log("QuitButton is clicked!");
				exit(0);	// 退出游戏
				break;
			default:
				break;
			}
		}
	);
}
/****						****/


/****		A* 算法相关			****/
int GameClient::aStar(mapNode** map, mapNode* origin, mapNode* destination, int tag)
{
	openList* open = new openList;
	open->next = nullptr;
	open->openNode = origin;
	closedList* close = new closedList;
	close->next = nullptr;
	close->closedNode = nullptr;

	// aStar_offset = (tag - AI_TAG) % 3;
	//循环检验4个方向的相邻节点
	while (checkNeighboringNodes(map, open, open->openNode, destination))
	{
		//从OPEN表中选取节点插入CLOSED表
		insertNodeToClosedList(close, open);
		// 寻路失败
		if (open == nullptr)
		{
			log("No Way!\n");
			break;
		}
		//若终点在OPEN表中，表明寻路成功
		if (open->openNode->status == DESTINATION)
		{
			mapNode* tempNode = open->openNode;
			//调用moveOnPath（）函数控制精灵在路径上移动
			moveOnPath(tempNode, tag);
			break;
		}
	}
	return 0;
}
void GameClient::moveOnPath(mapNode* tempNode, int tag)
{
	//声明存储路径坐标的结构体
	struct pathCoordinate { int x; int y; };
	//分配路径坐标结构体数组
	pathCoordinate* path = new pathCoordinate[MAP_WIDTH * MAP_HEIGHT];
	//利用父节点信息逆序存储路径坐标
	int loopNum = 0;
	while (tempNode != nullptr)
	{
		path[loopNum].x = tempNode->xCoordinate;
		path[loopNum].y = tempNode->yCoordinate;
		loopNum++;
		tempNode = tempNode->parent;
	}
	//将笑脸精灵的坐标存为绘制线段起点
	auto tank = (Tank*)this->getChildByTag(tag);
	int fromX = tank->getPositionX();
	int fromY = tank->getPositionY();
	//声明动作向量存储动作序列
	Vector<FiniteTimeAction*> actionVector;
	//从结构体数组尾部开始扫描
	int dir = tank->getDirection();
	for (int j = loopNum - 2; j >= 0; j--)
	{
		//将地图数组坐标转化为屏幕实际坐标
		int realX = (path[j].x + 0.5) * UNIT;
		int realY = m_visibleSize.height - (path[j].y + 0.5) * UNIT;

		// 转身
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
		//创建移动动作并存入动作向量
		auto moveAction = MoveTo::create(0.2, Vec2(realX, realY));
		actionVector.pushBack(moveAction);

		//绘制从起点到下一个地图单元的线段
		m_draw[tag - AI_TAG]->drawLine(Vec2(fromX, fromY), Vec2(realX, realY), Color4F(1.0, 1.0, 1.0, 1.0));
		//将当前坐标保存为下一次绘制的起点
		fromX = realX;
		fromY = realY;
	}

	if((tag - AI_TAG) % MAX_INGAME_AI_NUM == 0)
		actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnRight, this, tank)));
	else if ((tag - AI_TAG) % MAX_INGAME_AI_NUM == 1)
		actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnDown, this, tank)));
	else if ((tag - AI_TAG) % MAX_INGAME_AI_NUM == 2)
		actionVector.pushBack(CallFunc::create(CC_CALLBACK_0(GameClient::TurnLeft, this, tank)));

	//创建动作序列
	auto actionSequence = Sequence::create(actionVector);
	//笑脸精灵执行移动动作序列
	tank->runAction(actionSequence);
}
// 重置m_map_t的值，防止不同AI的A*算法相互影响
void GameClient::mapcopy()
{
	for (int i = 0; i < MAP_WIDTH; i++)
		for (int j = 0; j < MAP_HEIGHT; j++)
			m_map_t[i][j] = m_map[i][j];
}
/****							****/


/****		AI相关			****/
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
	// if (AI_remain_num < MAX_INGAME_AI_NUM) return;	// 剩余AI数量不足

	// 场上AI数量不够，补充直到满或者没有AI
	while (AI_ingame_num < MAX_INGAME_AI_NUM && AI_ingame_num < AI_remain_num)
	{
		auto AI_tank = Tank::create(AI_TAG + AI_next_offset, AI_spawnpointX[AI_next_offset % MAX_INGAME_AI_NUM], AI_spawnpointY, 1, 1);
		AI_tank->setTag(AI_TAG + AI_next_offset);
		m_tankList.pushBack(AI_tank);
		AI_next_offset++;
		AI_ingame_num++;
		this->addChild(AI_tank);

		// A*算法寻路
		int x = AI_tank->getPositionX(), y = AI_tank->getPositionY();
		// log("%d %d %d %d\n", x, y, x2i(x), y2j(y));
		mapcopy();	// 重置m_map_t，防止不同AI的A*算法相互影响f、g等值

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
		// 补充AI
		AI_fill();
		// 自动开火
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


// 初始化地图
void GameClient::createBackGround()
{
	auto map = TMXTiledMap::create("Chapter12/tank/map.tmx");
	// 隐藏其他层
	for (int i = 1; i <= MAX_MAP_LEVEL; i++)
		map->getLayer("brick" + std::to_string(i))->setVisible(false);
	
	m_mapLayer = map->getLayer("brick" + std::to_string(cur_map_level));
	// m_mapLayer->setVisible(false);
	this->addChild(map, 10);

	//根据地图宽、高分配数组空间
	m_map = new mapNode * [MAP_WIDTH];
	m_map_t = new mapNode * [MAP_WIDTH];
	for (int i = 0; i < MAP_WIDTH; i++)
	{
		m_map[i] = new mapNode[MAP_HEIGHT];
		m_map_t[i] = new mapNode[MAP_HEIGHT];
	}
	//依次扫描地图数组每一个单元
	for (int i = 0; i < MAP_WIDTH; i++)
	{
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			//若当前位置为墙体瓦片设置为不可通过
			int gid = m_mapLayer->getTileGIDAt(Vec2(i, j));
			if (gid == 1 || gid == 3 || gid == 7)
			{
				// 以该方块为中心的9宫格都不可到达
				int dx[9] = { 0, 0, 1, 0, -1, 1, 1, -1 ,-1 };
				int dy[9] = { 0, 1, 0, -1, 0, 1, -1, 1, -1 };
				for (int k = 0; k < 9; k++)
				{
					int ti = i + dx[k], tj = j + dy[k];
					mapNode temp = { NOT_ACCESS, ti, tj, 0, 0, 0, nullptr };
					if(ti < MAP_WIDTH && tj < MAP_HEIGHT)
						m_map[ti][tj] = temp;
				}
				// 添加砖块
				auto brick = Brick::create(Vec2(i2x(i), j2y(j), gid);
				m_bgList.pushBack(brick);
				this->addChild(brick, 2);
			}
			//否则设置为可以通过
			else if(m_map[i][j].status != NOT_ACCESS)
			{
				mapNode temp = { ACCESS, i, j, 0, 0, 0, nullptr };
				m_map[i][j] = temp;
			}
		}
	}
}


/****		键盘相关		****/
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
		m_tank->Flash();		// 新型移动方式：闪现
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_L:
	{
		m_tank->Fire_high();	// 新型进攻方式
	}
	break;
	}
}
/****						****/


/****		按键相关		****/
void GameClient::pressPauseButton(Ref* pSender, Widget::TouchEventType type)
{
	Size visibleSize = Director::getInstance()->getVisibleSize();
	RenderTexture* renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height);

	//遍历当前类的所有子节点信息，画入renderTexture中。
	//这里类似截图。
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