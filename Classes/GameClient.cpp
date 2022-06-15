#include "GameClient.h"

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
	createBackGround();

	// ���
	m_tank = Tank::create(110, WINDOWWIDTH / 2, 100, 1, 2);
	m_tankList.pushBack(m_tank);

	// ��ײ���
	this->scheduleUpdate();

	// �����¼�
	auto key_listener = EventListenerKeyboard::create();
	key_listener->onKeyPressed = CC_CALLBACK_2(GameClient::onKeyPressed, this);
	key_listener->onKeyReleased = CC_CALLBACK_2(GameClient::onKeyReleased, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	this->addChild(m_tank);
	m_drawList.pushBack(m_tank); // �������ټ��룬��ΪID�ɷ���������

	m_shouldFireList.clear();

	return true;
}

Scene* GameClient::createScene()
{
	auto scene = Scene::create();
	auto layer = GameClient::create();
	scene->addChild(layer);
	return scene;
}

void GameClient::update(float delta)
{
	// �յ������Ŀ�����Ϣ��̹��ִ��Fire
	if (m_shouldFireList.size() > 0)
	{
		auto tank = m_shouldFireList.at(0);
		tank->Fire();
		m_shouldFireList.clear();
	}

	// ά��̹���б�
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		if (nowTank->getLife() <= 0)
		{
			m_tankList.eraseObject(nowTank);
		}
		bool notDraw = true;
		for (int j = 0; j < m_drawList.size(); j++)
		{
			auto drawTank = m_drawList.at(j);
			if (drawTank->getID() == nowTank->getID())
			{
				notDraw = false;
			}
		}

		// ������δ���Ƶ�̹��-��Ժ��������Ŀͻ���
		if (notDraw)
		{
			this->addChild(nowTank);
			m_drawList.pushBack(nowTank);
		}
	}

	// TODO ��ײ���
	// ̹���� ̹�ˣ���Ʒ����ײ���
	for (int i = 0; i < m_tankList.size(); i++)
	{
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
		for (int j = 0; j < m_tankList.size(); j++)
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

		// ̹�����ӵ�
		auto tank = m_tankList.at(i);
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);
			for (int k = 0; k < m_tankList.size(); k++)
			{
				auto tank_another = m_tankList.at(k);
				if (tank->getID() != tank_another->getID())
				{
					if (bullet->getRect().intersectsRect(tank_another->getRect()))
					{
						// �ӵ�����
						m_deleteBulletList.pushBack(bullet);

						// ̹������
						m_deleteTankList.pushBack(tank_another);
					}
				}
			}
		}

		// �ӵ���ǽ
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);
			for (int k = 0; k < m_bgList.size(); k++)
			{
				auto brick = m_bgList.at(k);
				if (bullet->getRect().intersectsRect(brick->getRect()))
				{
					// �ӵ�����
					m_deleteBulletList.pushBack(bullet);

					// ש������
					m_deleteBrickList.pushBack(brick);
				}
			}
		}

		// ���ɾ���ӵ��б�
		for (int j = 0; j < m_deleteBulletList.size(); j++)
		{
			auto bullet = m_deleteBulletList.at(j);
			m_deleteBulletList.eraseObject(bullet);
			tank->getBulletList().eraseObject(bullet);
			bullet->Blast();
		}

		// ���ɾ��ש���б�
		for (int j = 0; j < m_deleteBrickList.size(); j++)
		{
			auto brick = m_deleteBrickList.at(j);
			m_deleteBrickList.eraseObject(brick);
			m_bgList.eraseObject(brick);
			brick->Blast();
		}

		// ���ɾ��̹���б�
		for (int j = 0; j < m_deleteTankList.size(); j++)
		{
			auto tank = m_deleteTankList.at(j);
			m_deleteTankList.eraseObject(tank);
			m_tankList.eraseObject(tank);
			tank->Blast();
		}
		m_deleteBulletList.clear();
		m_deleteBrickList.clear();
		m_deleteTankList.clear();
	}
}

// ����4������ש��
void GameClient::createBackGround()
{
	auto map = TMXTiledMap::create("Chapter12/tank/map.tmx");
	this->addChild(map, 10);
	int x, y;


	drawBigBG(Vec2(16 * 16, 5 * 16));
	x = 16;
	y = 43;
	drawBigBG(Vec2(20 * 16, 20 * 16));

	x = 45;
	//drawBigBG(Vec2(x * 16, y * 16));

	drawBigBG_HighLevel(Vec2(45 * 16, 8 * 16), 1, 8);
	drawBigBG_HighLevel(Vec2(55 * 16, 20 * 16), 1, 8);
	drawBigBG_HighLevel(Vec2(34 * 16, 10 * 16), 0, 14);
	drawBigBG_HighLevel(Vec2(23 * 16, 6 * 16), 1, 6);



	// ɨ���ͼ

}

// ���Ƶ�������ש��
void GameClient::drawBigBG(Vec2 position)
{
	for (int i = -2; i < 4; i++)
	{
		for (int j = -2; j < 4; j++)
		{
			if ((i == 1) && (j == 0) || (i == 0) && (j == 0) || (i == 1) && (j == 1) || (i == 0) && (j == 1))
			{
				// �м������γɻ���
				continue;
			}
			auto brick = Brick::create(Vec2(position.x + (0.5 - i) * 16, position.y + (0.5 - j) * 16), false);
			
			m_bgList.pushBack(brick);
			this->addChild(brick, 2);
		}
	}
}

void GameClient::drawBigBG_HighLevel(Vec2 position, int dir, int length)
{
	if (dir == 1) // ����
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < length; j++)
			{
				auto brick = Brick::create(Vec2(position.x + (0.5 - i) * 16, position.y + (0.5 - j) * 16), true);

				m_bgList.pushBack(brick);
				this->addChild(brick, 2);
			}
		}
	}
	else
	{
		for (int i = 0; i < length; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				auto brick = Brick::create(Vec2(position.x + (0.5 - i) * 16, position.y + (0.5 - j) * 16), true);
				m_bgList.pushBack(brick);
				this->addChild(brick, 2);
			}
		}
	}
}

void GameClient::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
	{
		m_tank->MoveLeft();
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		// m_tank->MoveUP();
	{
		m_tank->MoveUP();
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		// m_tank->MoveDown();
	{
		m_tank->MoveDown();
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		// m_tank->MoveRight();
	{
		m_tank->MoveRight();
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
		m_tank->Stay(TANK_LEFT);
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
	{
		m_tank->Stay(TANK_UP);
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
	{
		m_tank->Stay(TANK_DOWN);
	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
	{
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

//////////////////////////////////////////////////////////////////////////
// �������紫������ʱ��������Ӧ
void GameClient::addTank(int id, float x, float y, int dir, int kind)
{
	m_maxTank[tankcount] = Tank::create(id, x, y, dir, kind);
	m_tankList.pushBack(m_maxTank[tankcount++]);
}

void GameClient::addFire(Tank* tank)
{
	m_shouldFireList.pushBack(tank);
}
