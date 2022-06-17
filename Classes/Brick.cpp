#include "Brick.h"

Brick::Brick()
{

}

Brick::~Brick()
{

}

bool Brick::init(Vec2 position, int gid)
{
	if (!Sprite::init())
	{
		return false;
	}
	GID = gid;
	if(gid == 1) 
		m_sprite = Sprite::create("Chapter12/tank/tile.png",Rect(0, 16, 16, 16));	// ºì×©
	else if(gid == 3) 
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(32, 16, 16, 16));	// °××ª
	else if(gid == 7)
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(96, 16, 16, 16));	// Ë®
	this->addChild(m_sprite);
	m_sprite->setPosition(Vec2::ZERO);

	this->setPosition(position);
	m_rect = Rect(this->getPositionX() - 8, this->getPositionY() - 8, 16, 16);
	
	return true;
}

Brick* Brick::create(Vec2 position, int gid)
{
	Brick* pRet = new(std::nothrow) Brick();
	if (pRet && pRet->init(position, gid))
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

void Brick::Blast()
{
	this->setVisible(false);   // ×©¿éÏûÊ§
	this->removeFromParent();  
}

