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

	// 背景
	m_visibleSize = Director::getInstance()->getVisibleSize();
	createBackGround();

	// 玩家
	m_tank = Tank::create(PLAYER_TAG, WINDOWWIDTH / 2, 100, TANK_UP, 2);
	m_tankList.pushBack(m_tank);
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

void GameClient::AI_init()
{
	AI_update_delta = 0;
	AI_next_offset = 0;
	AI_remain_num = MAX_AI_NUM;
	AI_ingame_num = 0;
	for (int i = 0; i < MAX_INGAME_AI_NUM; i++)
	{
		auto AI_tank = Tank::create(AI_TAG + AI_next_offset, AI_spawnpointX[i], AI_spawnpointY, TANK_DOWN, 1);
		m_tankList.pushBack(AI_tank);
		AI_next_offset++;
		AI_ingame_num++;
		this->addChild(AI_tank);
	}
}

void GameClient::AI_fill()
{
	if (AI_remain_num < MAX_INGAME_AI_NUM) return;	// 剩余AI数量不足

	while (AI_ingame_num < MAX_INGAME_AI_NUM)
	{
		auto AI_tank = Tank::create(AI_TAG + AI_next_offset, 
			AI_spawnpointX[AI_next_offset % MAX_INGAME_AI_NUM], AI_spawnpointY, 1, 1);
		m_tankList.pushBack(AI_tank);
		AI_next_offset++;
		AI_ingame_num++;
		this->addChild(AI_tank);
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
			if(m_tankList.at(i)->getID() != PLAYER_TAG)
				m_tankList.at(i)->Fire();
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
	for (int n = 0; n < MAP_WIDTH; n++)
		m_map[n] = new mapNode[MAP_HEIGHT];
	//依次扫描地图数组每一个单元
	for (int i = 0; i < MAP_WIDTH; i++)
	{
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			//若当前位置为墙体瓦片设置为不可通过
			int gid = m_mapLayer->getTileGIDAt(Vec2(i, j));
			if (gid == 1 || gid == 3 || gid == 7)
			{
				mapNode temp = { NOT_ACCESS, i, j, 0, 0, 0, nullptr };
				m_map[i][j] = temp;
				// 添加砖块
				auto brick = Brick::create(Vec2((i - 0.5) * UNIT, m_visibleSize.height - (j + 0.5) * UNIT), gid);
				m_bgList.pushBack(brick);
				this->addChild(brick, 2);
			}

			//否则设置为可以通过
			else
			{
				mapNode temp = { ACCESS, i, j, 0, 0, 0, nullptr };
				m_map[i][j] = temp;
			}
		}
	}

	// drawBigBG(Vec2(16 * 16, 25 * 16));
	// drawBigBG(Vec2(44 * 16, 25 * 16));
	// drawBigBG(Vec2(16 * 16, 14 * 16));
	// drawBigBG(Vec2(44 * 16, 14 * 16));
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
