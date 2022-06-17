#include "Tank.h"

Tank* Tank::create(int ID, float x, float y, int dir, int kind)
{
	Tank* pRet = new(std::nothrow) Tank();
	if (pRet && pRet->init(ID, x, y, dir, kind))
	{
		pRet->autorelease();
		return pRet;
	} else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

bool Tank::init(int ID, float x, float y, int dir, int kind)
{
	if (!BaseObject::init())
	{
		return false;
	}
	Director::getInstance()->setProjection(Director::Projection::_2D); // ��Ϊ����ͶӰ������ͼƬģ��

	setLife(TANKBASELIFE); // tank life
	setSpeed(TANKSPEED);   // tank speed
	setLevel(1);           // tank level
	setHindered(TANK_STAY);    // tank hindered
	m_moveUp = FALSE;
	m_moveDown = FALSE;
	m_moveRight = FALSE;
	m_moveLeft = FALSE;

	m_isMoving = false;

	m_frametime = 2.0;
	m_temptime = 0;

	// tank init ifo
	setID(ID);
	this->setPositionX(x);
	this->setPositionY(y);
	setDirection(dir);
	setKind(kind);

	// tank kind
	if (kind == 1)
	{
		m_texture = Director::getInstance()->getTextureCache()->addImage("Chapter12/tank/player1.png");
	}
	if (kind == 2)
	{
		m_texture = Director::getInstance()->getTextureCache()->addImage("Chapter12/tank/player2.png");
	}

	// tank initY state
	m_textureX = ((this->getLevel() - 1) * 4 + 1) * 14;
	if (dir == TANK_UP)	 m_textureY = 1*14;
	else if (dir == TANK_LEFT)	 m_textureY = 7*14;
	else if (dir == TANK_RIGHT) m_textureY = 3*14;
	else if (dir == TANK_DOWN)  m_textureY = 5*14;
	

	// ̹�˳�ʼ״̬
	m_sprite = Sprite::createWithTexture(m_texture, Rect(m_textureX-14.0, m_textureY-14.0, 28, 28));
	m_rect = Rect(this->getPositionX() - 16, this->getPositionY() - 16, 32, 32);
	m_sprite->setPosition(Vec2::ZERO);
	m_sprite->setScale(TANKSIZE/28);
	this->addChild(m_sprite);

	this->scheduleUpdate();
	return true;
}

void Tank::MoveUP()
{
	m_moveUp = true;
	m_moveDown = false;
	m_moveLeft = false;
	m_moveRight = false;

	setDirection(TANK_UP);
}

void Tank::MoveDown()
{
	m_moveDown = true;
	m_moveUp = false;
	m_moveLeft = false;
	m_moveRight = false;

	setDirection(TANK_DOWN);
}

void Tank::MoveLeft()
{
	m_moveLeft = true;
	m_moveDown = false;
	m_moveRight = false;
	m_moveUp = false;

	setDirection(TANK_LEFT);
}

void Tank::MoveRight()
{
	m_moveRight = true;
	m_moveLeft = false;
	m_moveUp = false;
	m_moveDown = false;

	setDirection(TANK_RIGHT);
}

void Tank::Fire()
{
	Vec2 position;
	switch (this->getDirection())
	{
	case TANK_UP:
		position = Vec2(this->getPositionX(), this->getPositionY() + 14);
		break;
	case TANK_DOWN:
		position = Vec2(this->getPositionX(), this->getPositionY() - 14);
		break;
	case TANK_LEFT:
		position = Vec2(this->getPositionX() - 14, this->getPositionY());
		break;
	case TANK_RIGHT:
		position = Vec2(this->getPositionX() + 14, this->getPositionY());
		break;
	}
	auto bullet = Bullet::create(position, 3, this->getDirection());
	m_bulletList.pushBack(bullet);            // ��ӵ��ӵ��б�
	this->getParent()->addChild(bullet, 8);   // ��ӵ���Ϸ����
}

// ����Draw��дMyDraw()������A*�㷨ת��
void Tank::MyDraw()
{
	if (this->getLife() <= 0) return;
	
	int dir = this->getDirection();
	// ������������
	m_textureX = ((this->getLevel() - 1) * 4 + 1) * 14;		// get tank textureX
	if (dir == TANK_UP)	 m_textureY = 1 * 14;
	else if (dir == TANK_LEFT)	 m_textureY = 7 * 14;
	else if (dir == TANK_RIGHT) m_textureY = 3 * 14;
	else if (dir == TANK_DOWN)  m_textureY = 5 * 14;

	this->removeChild(m_sprite, true); // ��Ҫ����ǰһ�������Ƴ�,�����ڴ��޷��ͷ�
	m_sprite = Sprite::createWithTexture(m_texture, Rect(m_textureX - 14.0, m_textureY - 14.0, 28, 28));
	m_sprite->setScale(TANKSIZE / 28);
	this->addChild(m_sprite);          // ���¾���ͼƬ
}


void Tank::Draw()
{
	if(this->getLife()){
		m_textureX = ((this->getLevel() - 1) * 4 + 1) * 14;   // get tank textureX

		// ����̹���Ĵ�ת��
		if (m_moveUp || m_moveDown || m_moveLeft || m_moveRight)
		{
			m_temptime += m_frametime;
			if(m_temptime > 5)												
			{
				m_temptime -= 5;
				m_texchange = (m_texchange+1) % 2;
			}
			switch (m_texchange)
			{
			case 0:
				m_textureX = ((this->getLevel() - 1) * 4 + 1) * 14;
				break;
			case 1:
				m_textureX = ((this->getLevel() - 1) * 4 + 1) * 14 + 2*14;
				break;
			}
		}

		this->removeChild(m_sprite, true); // ��Ҫ����ǰһ�������Ƴ�,�����ڴ��޷��ͷ�
		m_sprite = Sprite::createWithTexture(m_texture, Rect(m_textureX-14.0, m_textureY-14.0, 28, 28));
		m_sprite->setScale(TANKSIZE/28);
		this->addChild(m_sprite);          // ���¾���ͼƬ
	}
}

void Tank::Blast()
{
	this->setVisible(false);   // ̹����ʧ
	this->setLife(0);
	auto explode = Sprite::create("Chapter12/tank/explode2.png");
	this->getParent()->addChild(explode);
	explode->setPosition(this->getPosition());  // ��ʾ��ը
	explode->runAction(Sequence::create(
		DelayTime::create(0.3f),
		FadeOut::create(0.3f),                  // ��ը��ʧ
		CallFunc::create(CC_CALLBACK_0(Tank::deleteObj, this, explode)),
		NULL
		));
}

void Tank::update(float t)
{
	m_isMoving = m_moveUp | m_moveDown | m_moveLeft | m_moveRight;				 // �����ƶ�״̬
	m_rect = Rect(this->getPositionX() - 16, this->getPositionY() - 16, 32, 32); // ����rect
	// m_rect = Rect(this->getPositionX() - 14, this->getPositionY() - 14, 28, 28); // ����rect
	for (int i = 0;i < m_bulletList.size();i++)
	{
		auto nowBullet = m_bulletList.at(i);
		if (nowBullet->getLife() <= 0)
		{
			m_bulletList.eraseObject(nowBullet);
		}
	}
	if (this->getLife() <= 0)
	{
		this->unscheduleUpdate();
	}
	if (m_moveUp)
	{
		m_textureY = 1*14;
		if ((this->getPositionY() <= WINDOWHEIGHT - 14) && (this->getHindered() != TANK_UP))
		{
			setHindered(TANK_STAY);
			this->setPositionY(this->getPositionY() + this->getSpeed());
		}
		Draw();
	}
	if (m_moveDown)
	{
		m_textureY = 5*14;
		if ((this->getPositionY() >= 14) && (this->getHindered() != TANK_DOWN))
		{
			setHindered(TANK_STAY);
			this->setPositionY(this->getPositionY() - this->getSpeed());
		}
		Draw();
	}
	if (m_moveLeft)
	{
		m_textureY = 7*14;
		if ((this->getPositionX() >= 14) && (this->getHindered() != TANK_LEFT))
		{
			setHindered(TANK_STAY);
			this->setPositionX(this->getPositionX() - this->getSpeed());
		}
		Draw();
	}
	if (m_moveRight)
	{
		m_textureY = 3*14;
		if ((this->getPositionX() <= WINDOWWIDTH - 14) && (this->getHindered() != TANK_RIGHT))
		{
			setHindered(TANK_STAY);
			this->setPositionX(this->getPositionX() + this->getSpeed());
		}
		Draw();
	}
}

void Tank::Stay(int dir)
{
	switch (dir)
	{
	case TANK_UP:
		m_moveUp = false;
		this->setPositionY(this->getPositionY() - 1);
		break;
	case TANK_DOWN:
		m_moveDown = false;
		this->setPositionY(this->getPositionY() + 1);
		break;
	case TANK_LEFT:
		m_moveLeft = false;
		this->setPositionX(this->getPositionX() + 1);
		break;
	case TANK_RIGHT:
		m_moveRight = false;
		this->setPositionX(this->getPositionX() - 1);
		break;
	}
}

void Tank::deleteObj(Sprite* obj)
{
	obj->removeFromParent();
	this->removeFromParent();
}
