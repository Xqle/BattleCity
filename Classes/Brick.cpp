#include "Brick.h"

Brick::Brick()
{

}

Brick::~Brick()
{

}

bool Brick::init(Vec2 position, bool _is_highLevel)
{
	if (!Sprite::init())
	{
		return false;
	}

	if (!_is_highLevel)
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(0, 0, 16, 16));
	else
	{
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(32, 0, 16, 16));

	}

	is_highLevel = _is_highLevel;

	this->addChild(m_sprite);
	m_sprite->setPosition(Vec2::ZERO);

	this->setPosition(position);
	m_rect = Rect(this->getPositionX() - 8, this->getPositionY() - 8, 16, 16);

	return true;
}

Brick* Brick::create(Vec2 position, bool _is_highLevel)
{
	Brick* pRet = new(std::nothrow) Brick();
	if (pRet && pRet->init(position, _is_highLevel))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

void Brick::Blast()
{
	this->setVisible(false);   // ×©¿éÏûÊ§
	this->removeFromParent();
}

