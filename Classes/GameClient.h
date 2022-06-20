#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "cocos2d.h"
#include "Tank.h"
#include "Brick.h"
#include "aStar.h"
#include "GamePause.h"
#include "SimpleAudioEngine.h"
#include "Global.h"
#include "VictoryScene.h"
USING_NS_CC;
using namespace cocos2d;

#define VISIBLE_WIDTH	(int)Director::getInstance()->getVisibleSize().width
#define VISIBLE_HEIGHT	(int)Director::getInstance()->getVisibleSize().height
#define x2i(x) x / UNIT
#define y2j(y) (VISIBLE_HEIGHT - y) / UNIT
#define i2x(i) (i + 0.5) * UNIT
#define j2y(j) VISIBLE_HEIGHT - (j + 0.5) * UNIT)

// 玩家
#define PLAYER_TAG 110
#define PLAYER_LIFE 5

// AI
#define AI_TAG 111
#define MAX_AI_NUM 5
#define MAX_INGAME_AI_NUM 3

static int tankcount = 0;     // 记录当前坦克数


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

	// 实现键盘回调
	char maxValKey();
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event); 

	void onTouchEnded(EventKeyboard::KeyCode keyCode, Event* event);

	void pressMenuButton(Ref* pSender, Widget::TouchEventType type);
	void pressReplayButton(Ref* pSender, Widget::TouchEventType type);
	void pressPauseButton(Ref* pSender, Widget::TouchEventType type);

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };

	// 计分板操作函数

	// 让目标target 加score 分
	void add_score(int score) { m_score += score;  };

	// 获取目标target当前分数
	int get_score() { return m_score; };


	// UI related
	Button* Menubtn;
	Button* Replaybtn;
	Button* Pausebtn;

	Text* P1score;
	TextBMFont* P2score;

	bool stopped;

	// AI
	void AI_init();
	void AI_update(float delta);
	void AI_fill();	// 补足场上AI
	// A*算法
	int aStar(mapNode** map, mapNode* origin, mapNode* destination, int tag);
	void moveOnPath(mapNode* tempNode, int tag);
	void mapcopy();
	// tank移动
	void TurnLeft(Tank* tank)	{ tank->setDirection(TANK_LEFT);	tank->MyDraw();	}
	void TurnRight(Tank* tank)	{ tank->setDirection(TANK_RIGHT);	tank->MyDraw();	}
	void TurnDown(Tank* tank)   { tank->setDirection(TANK_DOWN);	tank->MyDraw();	}
	void TurnUp(Tank* tank)		{ tank->setDirection(TANK_UP);		tank->MyDraw();	}

	// 胜利/失败
	void gameover();
	void success();

private:
	Vector<Brick*>  m_bgList;     // 背景块列表
	Vector<Tank*>   m_tankList;   // 坦克列表
	Vector<Tank*>	m_drawList;   // 已绘制的坦克
	
	Vector<Bullet*> m_deleteBulletList;   // 删除子弹列表
	Vector<Brick*>  m_deleteBrickList;    // 删除砖块列表
	Vector<Tank*>   m_deleteTankList;     // 删除坦克列表

	// 守护目标
	Sprite* m_bird;			// 精灵
	Rect	m_bird_rect;	// 判定框
	double	m_bird_spawnpointX = 32 * UNIT;
	double	m_bird_spawnpointY = 4 * UNIT;

	// 玩家
	Tank* m_tank;       // 主坦克
	int	   player_life = PLAYER_LIFE;
	double player_spawnpointX = 32 * UNIT;
	double player_spawnpointY = 23 * UNIT;
	double invulnerable_timer = 0;	// 无敌时间
	int enemy_kill = 0;		// 击杀敌人数量

	// AI
	double AI_spawnpointX[MAX_INGAME_AI_NUM] = { 7 * UNIT + 8, 32 * UNIT + 8, 57 * UNIT + 8};
	double AI_spawnpointY = 44 * UNIT + 8;	// AI 出生点
	int AI_destinationX[MAX_INGAME_AI_NUM] = { 17, 31, 46 };
	int AI_destinationY[MAX_INGAME_AI_NUM] = { 44, 37, 44 };
	double AI_update_delta;		// AI_action的计时器
	int AI_remain_num;			// 剩余多少个AI			(<= MAX_AI_NUM)
	int AI_ingame_num;			// 当前在屏幕里的AI		(<= MAX_INGAME_AI_NUM)
	int AI_next_offset;			// 下一个创建的AI是第几个AI

	// A*算法
	TMXLayer* m_mapLayer;		//瓦片地图的图层对象
	Size m_visibleSize;			//屏幕的可见尺寸
	mapNode** m_map;			//地图数组指针
	mapNode** m_map_t;			//临时用的地图数组指针
	// mapNode* m_origin;			//寻路起点指针
	mapNode* m_destination[MAX_INGAME_AI_NUM];		//寻路终点指针
	DrawNode* m_draw[MAX_AI_NUM];

	// 键盘按键
	int keys[128];

	// 失败判断
	bool is_success;
	bool is_gameover;
};

#endif