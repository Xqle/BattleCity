#include "GameClient.h"
#include <algorithm>
#include <cstdlib>

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

	for (int i = 0; i < MAX_INGAME_AI_NUM; i++)
	{
		m_draw[i] = DrawNode::create();
		this->addChild(m_draw[i], 2);
	}
	// 背景
	m_visibleSize = Director::getInstance()->getVisibleSize();
	createBackGround();

	// 玩家
	m_tank = Tank::create(PLAYER_TAG, WINDOWWIDTH / 2 + 8, 96 + 8, TANK_UP, 2);
	m_tankList.pushBack(m_tank);
	int m_x = m_tank->getPositionX(), m_y = m_tank->getPositionY();
	m_map[x2i(m_x)][y2j(m_y)].status = DESTINATION;
	m_destination = &m_map[x2i(m_x)][y2j(m_y)];
	this->addChild(m_tank);

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

	return true;
}

Scene* GameClient::createScene()
{
	auto scene = Scene::create();
	auto layer = GameClient::create();
	scene->addChild(layer);
	return scene;
}

int GameClient::aStar(mapNode** map, mapNode* origin, mapNode* destination, int tag)
{
	openList* open = new openList;
	open->next = nullptr;
	open->openNode = origin;
	closedList* close = new closedList;
	close->next = nullptr;
	close->closedNode = nullptr;
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

void GameClient::AI_init()
{
	AI_update_delta = 0;
	AI_next_offset = 0;
	AI_remain_num = MAX_AI_NUM;
	AI_ingame_num = 0;
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
		auto m_origin = &m_map_t[x2i(x)][y2j(y)];
		aStar(m_map_t, m_origin, m_destination, AI_tank->getID());
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

void GameClient::update(float delta)
{
	AI_update(delta);
	// 维护坦克列表
	for (int i = 0;i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		if (nowTank->getLife() <= 0)
		{
			m_deleteTankList.pushBack(nowTank);
			// m_tankList.eraseObject(nowTank);
		}
	}

	// 坦克移动
	char mvkey = maxValKey();
	if (mvkey == 'W') m_tank->MoveUP();
	else if (mvkey == 'A') m_tank->MoveLeft();
	else if (mvkey == 'S') m_tank->MoveDown();
	else if (mvkey == 'D') m_tank->MoveRight();

	// 坦克与 坦克/物品 的碰撞检测
	for (int i = 0;i < m_tankList.size(); i++)
	{
		// 坦克与砖块
		for (int j = 0; j < m_bgList.size(); j++)
		{
			auto nowTank = m_tankList.at(i);
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
		for (int j = 0; j < m_tankList.size(); j ++)
		{
			auto nowTank = m_tankList.at(i);
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
		for (int j = 0; j < tank->getBulletList().size(); j ++)
		{
			auto bullet = tank->getBulletList().at(j);
			// 子弹与砖块
			for (int k = 0; k < m_bgList.size(); k++)
			{
				auto brick = m_bgList.at(k);
				if (brick->getGID() == 7) continue;		// 水可以穿过
				if (bullet->getRect().intersectsRect(brick->getRect()))
				{
					m_deleteBulletList.pushBack(bullet);	// 子弹消除
					if (brick->getGID() == 3) continue;		// 白块不可消除
					m_deleteBrickList.pushBack(brick);		// 砖块消除
				}
			}
			
			// 子弹与坦克 / 子弹
			for (int k = 0;k < m_tankList.size(); k ++)
			{
				// 子弹与坦克
				auto tank_another = m_tankList.at(k);
				if (tank->getID() == tank_another->getID()) continue;
				// AI和AI不相互影响
				if (tank->getID() != PLAYER_TAG && tank_another->getID() != PLAYER_TAG) continue;

				if (bullet->getRect().intersectsRect(tank_another->getRect()))
				{
					m_deleteBulletList.pushBack(bullet);		// 子弹消除
					m_deleteTankList.pushBack(tank_another);	// 坦克消除
					break;	// 该子弹已命中，不能同时命中两个目标
				}

				// 子弹与子弹
				for (int t = 0; t < tank_another->getBulletList().size(); t++)
				{
					auto bullet_another = tank_another->getBulletList().at(t);
					if (bullet->getRect().intersectsRect(bullet_another->getRect()))
					{
						m_deleteBulletList.pushBack(bullet);		// 子弹消除
						m_deleteBulletList.pushBack(bullet_another);		// 子弹消除
					}
				}
			}
		}

		// 清除删除子弹列表
		for (int j = 0; j < m_deleteBulletList.size(); j ++)
		{
			auto bullet = m_deleteBulletList.at(j);
			m_deleteBulletList.eraseObject(bullet);
			tank->getBulletList().eraseObject(bullet);
			bullet->Blast();
		}

		// 清除删除砖块列表
		for (int j = 0; j < m_deleteBrickList.size(); j ++)
		{
			auto brick = m_deleteBrickList.at(j);
			int x = brick->getPositionX(), y = brick->getPositionY();
			m_map[x2i(x)][y2j(y)].status = ACCESS;	// 删除的方块对应的mapnode状态要改为可通过
			m_deleteBrickList.eraseObject(brick);
			m_bgList.eraseObject(brick);
			brick->Blast();
		}

		// 清除删除坦克列表
		for (int j = 0; j < m_deleteTankList.size(); j ++)
		{
			auto tank = m_deleteTankList.at(j);
			if (tank->getID() != 110)
			{
				AI_ingame_num--;		// 如果是AI，则减少游戏内AI数量
				AI_remain_num--;		// 如果是AI，减少剩余AI数量	
			}
			m_deleteTankList.eraseObject(tank);
			m_tankList.eraseObject(tank);
			tank->Blast();
		}
		m_deleteBulletList.clear();
		m_deleteBrickList.clear();
		m_deleteTankList.clear();
	}
}

// 初始化地图
void GameClient::createBackGround()
{
	auto map = TMXTiledMap::create("Chapter12/tank/map.tmx");
	m_mapLayer = map->getLayer("brick");
	m_mapLayer->setVisible(false);
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

// 绘制单个回字砖块
//void GameClient::drawBigBG(Vec2 position)
//{
//	for (int i = -2; i < 4;i ++)
//	{
//		for (int j = -2;j < 4;j ++)
//		{
//			if ((i == 1)&&(j == 0) || (i == 0)&&(j == 0) || (i == 1)&&(j == 1) || (i == 0)&&(j == 1))
//			{
//				// 中间留空形成回字
//				continue;
//			}
//			auto brick = Brick::create(Vec2(position.x + (0.5 - i) * 16, position.y + (0.5 - j) * 16));
//			m_bgList.pushBack(brick);
//			this->addChild(brick, 2);
//		}
//	}
//}

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
	}
}
