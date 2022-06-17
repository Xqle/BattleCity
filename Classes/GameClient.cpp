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

	// ����
	m_visibleSize = Director::getInstance()->getVisibleSize();
	createBackGround();

	// ���
	m_tank = Tank::create(PLAYER_TAG, WINDOWWIDTH / 2, 100, TANK_UP, 2);
	m_tankList.pushBack(m_tank);
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
	if (AI_remain_num < MAX_INGAME_AI_NUM) return;	// ʣ��AI��������

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
		// ����AI
		AI_fill();
		// �Զ�����
		for (int i = 0; i < m_tankList.size(); i++)
			if(m_tankList.at(i)->getID() != PLAYER_TAG)
				m_tankList.at(i)->Fire();
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
	for (int n = 0; n < MAP_WIDTH; n++)
		m_map[n] = new mapNode[MAP_HEIGHT];
	//����ɨ���ͼ����ÿһ����Ԫ
	for (int i = 0; i < MAP_WIDTH; i++)
	{
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			//����ǰλ��Ϊǽ����Ƭ����Ϊ����ͨ��
			int gid = m_mapLayer->getTileGIDAt(Vec2(i, j));
			if (gid == 1 || gid == 3 || gid == 7)
			{
				mapNode temp = { NOT_ACCESS, i, j, 0, 0, 0, nullptr };
				m_map[i][j] = temp;
				// ���ש��
				auto brick = Brick::create(Vec2((i - 0.5) * UNIT, m_visibleSize.height - (j + 0.5) * UNIT), gid);
				m_bgList.pushBack(brick);
				this->addChild(brick, 2);
			}

			//��������Ϊ����ͨ��
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
