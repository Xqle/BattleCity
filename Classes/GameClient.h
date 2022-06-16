#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "cocos2d.h"
#include "Tank.h"
#include "Brick.h"
#include "aStar.h"

USING_NS_CC;
using namespace cocos2d;

#define PLAYER_TAG 110
#define MAX_AI_NUM 10
#define MAX_INGAME_AI_NUM 3
#define AI_TAG 111
static int tankcount = 0;     // 记录当前坦克数
static int NET_TAG = 11111;   

class GameClient : public Scene
{
public:
	GameClient();
	~GameClient();

	CREATE_FUNC(GameClient);
	virtual bool init();
	static Scene* createScene();
	void createBackGround();
	void update(float delta);
	void drawBigBG(Vec2 position);

	// 实现键盘回调
	char maxValKey();
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event); 

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };

	// AI
	void AI_init();
	void AI_update(float delta);
	void AI_fill();	// 补足场上AI

private:
	Vector<Brick*>  m_bgList;     // 背景块列表
	Vector<Tank*>   m_tankList;   // 坦克列表
	Tank*           m_tank;       // 主坦克
	Vector<Tank*>	m_drawList;   // 已绘制的坦克
	
	Vector<Bullet*> m_deleteBulletList;   // 删除子弹列表
	Vector<Brick*>  m_deleteBrickList;    // 删除砖块列表
	Vector<Tank*>   m_deleteTankList;     // 删除坦克列表

	// AI
	double AI_spawnpointX[MAX_INGAME_AI_NUM] = { WINDOWWIDTH / 4, WINDOWWIDTH / 2, WINDOWWIDTH * 3 / 4 };
	double AI_spawnpointY = 600;	// AI 出生点
	double AI_update_delta;		// AI_action的计时器
	int AI_remain_num;			// 剩余多少个AI			(<= MAX_AI_NUM)
	int AI_ingame_num;			// 当前在屏幕里的AI		(<= MAX_INGAME_AI_NUM)
	int AI_next_offset;			// 下一个创建的AI是第几个AI

	// 键盘按键
	int keys[128];
};

#endif