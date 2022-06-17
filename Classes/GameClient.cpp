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
	// ����
	m_visibleSize = Director::getInstance()->getVisibleSize();
	createBackGround();

	// ���
	m_tank = Tank::create(PLAYER_TAG, WINDOWWIDTH / 2 + 8, 96 + 8, TANK_UP, 2);
	m_tankList.pushBack(m_tank);
	int m_x = m_tank->getPositionX(), m_y = m_tank->getPositionY();
	m_map[x2i(m_x)][y2j(m_y)].status = DESTINATION;
	m_destination = &m_map[x2i(m_x)][y2j(m_y)];
	this->addChild(m_tank);

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

void GameClient::AI_init()
{
	AI_update_delta = 0;
	AI_next_offset = 0;
	AI_remain_num = MAX_AI_NUM;
	AI_ingame_num = 0;
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

void GameClient::update(float delta)
{
	AI_update(delta);
	// ά��̹���б�
	for (int i = 0;i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		if (nowTank->getLife() <= 0)
		{
			m_deleteTankList.pushBack(nowTank);
			// m_tankList.eraseObject(nowTank);
		}
	}

	// ̹���ƶ�
	char mvkey = maxValKey();
	if (mvkey == 'W') m_tank->MoveUP();
	else if (mvkey == 'A') m_tank->MoveLeft();
	else if (mvkey == 'S') m_tank->MoveDown();
	else if (mvkey == 'D') m_tank->MoveRight();

	// ̹���� ̹��/��Ʒ ����ײ���
	for (int i = 0;i < m_tankList.size(); i++)
	{
		// ̹����ש��
		for (int j = 0; j < m_bgList.size(); j++)
		{
			auto nowTank = m_tankList.at(i);
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
		for (int j = 0; j < m_tankList.size(); j ++)
		{
			auto nowTank = m_tankList.at(i);
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
		for (int j = 0; j < tank->getBulletList().size(); j ++)
		{
			auto bullet = tank->getBulletList().at(j);
			// �ӵ���ש��
			for (int k = 0; k < m_bgList.size(); k++)
			{
				auto brick = m_bgList.at(k);
				if (brick->getGID() == 7) continue;		// ˮ���Դ���
				if (bullet->getRect().intersectsRect(brick->getRect()))
				{
					m_deleteBulletList.pushBack(bullet);	// �ӵ�����
					if (brick->getGID() == 3) continue;		// �׿鲻������
					m_deleteBrickList.pushBack(brick);		// ש������
				}
			}
			
			// �ӵ���̹�� / �ӵ�
			for (int k = 0;k < m_tankList.size(); k ++)
			{
				// �ӵ���̹��
				auto tank_another = m_tankList.at(k);
				if (tank->getID() == tank_another->getID()) continue;
				// AI��AI���໥Ӱ��
				if (tank->getID() != PLAYER_TAG && tank_another->getID() != PLAYER_TAG) continue;

				if (bullet->getRect().intersectsRect(tank_another->getRect()))
				{
					m_deleteBulletList.pushBack(bullet);		// �ӵ�����
					m_deleteTankList.pushBack(tank_another);	// ̹������
					break;	// ���ӵ������У�����ͬʱ��������Ŀ��
				}

				// �ӵ����ӵ�
				for (int t = 0; t < tank_another->getBulletList().size(); t++)
				{
					auto bullet_another = tank_another->getBulletList().at(t);
					if (bullet->getRect().intersectsRect(bullet_another->getRect()))
					{
						m_deleteBulletList.pushBack(bullet);		// �ӵ�����
						m_deleteBulletList.pushBack(bullet_another);		// �ӵ�����
					}
				}
			}
		}

		// ���ɾ���ӵ��б�
		for (int j = 0; j < m_deleteBulletList.size(); j ++)
		{
			auto bullet = m_deleteBulletList.at(j);
			m_deleteBulletList.eraseObject(bullet);
			tank->getBulletList().eraseObject(bullet);
			bullet->Blast();
		}

		// ���ɾ��ש���б�
		for (int j = 0; j < m_deleteBrickList.size(); j ++)
		{
			auto brick = m_deleteBrickList.at(j);
			int x = brick->getPositionX(), y = brick->getPositionY();
			m_map[x2i(x)][y2j(y)].status = ACCESS;	// ɾ���ķ����Ӧ��mapnode״̬Ҫ��Ϊ��ͨ��
			m_deleteBrickList.eraseObject(brick);
			m_bgList.eraseObject(brick);
			brick->Blast();
		}

		// ���ɾ��̹���б�
		for (int j = 0; j < m_deleteTankList.size(); j ++)
		{
			auto tank = m_deleteTankList.at(j);
			if (tank->getID() != 110)
			{
				AI_ingame_num--;		// �����AI���������Ϸ��AI����
				AI_remain_num--;		// �����AI������ʣ��AI����	
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

// ��ʼ����ͼ
void GameClient::createBackGround()
{
	auto map = TMXTiledMap::create("Chapter12/tank/map.tmx");
	m_mapLayer = map->getLayer("brick");
	m_mapLayer->setVisible(false);
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

// ���Ƶ�������ש��
//void GameClient::drawBigBG(Vec2 position)
//{
//	for (int i = -2; i < 4;i ++)
//	{
//		for (int j = -2;j < 4;j ++)
//		{
//			if ((i == 1)&&(j == 0) || (i == 0)&&(j == 0) || (i == 1)&&(j == 1) || (i == 0)&&(j == 1))
//			{
//				// �м������γɻ���
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
