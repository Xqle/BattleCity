#ifndef __BRICK_H__
#define __BRICK_H__

#include "cocos2d.h"
USING_NS_CC;
using namespace cocos2d;

class Brick : public Sprite
{
public:
	Brick();
	~Brick();
	static Brick* create(Vec2 position, int gid);
	virtual bool init(Vec2 position, int gid);
	void Blast();

	// get
	Rect getRect() { return m_rect; };
	int getGID() { return GID; };
private:
	Sprite* m_sprite;  // 砖块精灵
	Rect m_rect;       // 包围框
	int GID;		   // 对应的GID
};

#endif