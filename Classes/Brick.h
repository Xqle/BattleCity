#ifndef __BRICK_H__
#define __BRICK_H__

#include "cocos2d.h"
USING_NS_CC;
using namespace cocos2d;

	
#define RED_BRICK_GID	1
#define WHITE_BRICK_GID 3
#define WATER_TILE_GID	7
#define BIRD_TILE_TAG	8

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