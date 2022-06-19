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
	if (gid == RED_BRICK_GID)
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(0, 16, 16, 16));	// 红砖
	else if (gid == WHITE_BRICK_GID)
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(32, 16, 16, 16));	// 白转
	else if (gid == WATER_TILE_GID)
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(96, 16, 16, 16));	// 水
	else if (gid == BIRD_TILE_TAG)
		m_sprite = Sprite::create("Chapter12/tank/tile.png", Rect(160, 0, 32, 32));	// 守护目标
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
	this->setVisible(false);   // 砖块消失
	this->removeFromParent();  
}

