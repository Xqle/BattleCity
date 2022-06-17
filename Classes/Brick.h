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
	Sprite* m_sprite;  // ש�龫��
	Rect m_rect;       // ��Χ��
	int GID;		   // ��Ӧ��GID
};

#endif