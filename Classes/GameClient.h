#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "cocos2d.h"
#include "Tank.h"
#include "Brick.h"
#include "aStar.h"

USING_NS_CC;
using namespace cocos2d;

#define VISIBLE_WIDTH	(int)Director::getInstance()->getVisibleSize().width
#define VISIBLE_HEIGHT	(int)Director::getInstance()->getVisibleSize().height
#define x2i(x) x / UNIT
#define y2j(y) (VISIBLE_HEIGHT - y) / UNIT
#define i2x(i) (i + 0.5) * UNIT
#define j2y(j) VISIBLE_HEIGHT - (j + 0.5) * UNIT)

// ���
#define PLAYER_TAG 110
#define PLAYER_LIFE 5

// AI
#define AI_TAG 111
#define MAX_AI_NUM 10
#define MAX_INGAME_AI_NUM 4

static int tankcount = 0;     // ��¼��ǰ̹����


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

	// ʵ�ּ��̻ص�
	char maxValKey();
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event); 

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };

	// AI
	void AI_init();
	void AI_update(float delta);
	void AI_fill();	// ���㳡��AI
	// A*�㷨
	int aStar(mapNode** map, mapNode* origin, mapNode* destination, int tag);
	void moveOnPath(mapNode* tempNode, int tag);
	void mapcopy();
	// tank�ƶ�
	void TurnLeft(Tank* tank)	{ tank->setDirection(TANK_LEFT);	tank->MyDraw();	}
	void TurnRight(Tank* tank)	{ tank->setDirection(TANK_RIGHT);	tank->MyDraw();	}
	void TurnDown(Tank* tank)   { tank->setDirection(TANK_DOWN);	tank->MyDraw();	}
	void TurnUp(Tank* tank)		{ tank->setDirection(TANK_UP);		tank->MyDraw();	}

private:
	Vector<Brick*>  m_bgList;     // �������б�
	Vector<Tank*>   m_tankList;   // ̹���б�
	Vector<Tank*>	m_drawList;   // �ѻ��Ƶ�̹��
	
	Vector<Bullet*> m_deleteBulletList;   // ɾ���ӵ��б�
	Vector<Brick*>  m_deleteBrickList;    // ɾ��ש���б�
	Vector<Tank*>   m_deleteTankList;     // ɾ��̹���б�

	// �ػ�Ŀ��
	Sprite* m_bird;
	Rect	m_bird_rect;
	double	m_bird_spawnpointX = WINDOWWIDTH / 2;
	double	m_bird_spawnpointY = WINDOWHEIGHT / 5;
	// ���
	Tank* m_tank;       // ��̹��
	int	   player_life = PLAYER_LIFE;
	double player_spawnpointX = WINDOWWIDTH / 2 - 8;
	double player_spawnpointY = 104;
	double invulnerable_timer = 0;	// �޵�ʱ��
	int enemy_kill = 0;		// ��ɱ��������

	// AI
	double AI_spawnpointX[MAX_INGAME_AI_NUM] = { WINDOWWIDTH / 5 + 3, WINDOWWIDTH * 2 / 5, WINDOWWIDTH * 3 / 5, WINDOWWIDTH * 4 / 5 };
	double AI_spawnpointY = 600;	// AI ������
	double AI_update_delta;		// AI_action�ļ�ʱ��
	int AI_remain_num;			// ʣ����ٸ�AI			(<= MAX_AI_NUM)
	int AI_ingame_num;			// ��ǰ����Ļ���AI		(<= MAX_INGAME_AI_NUM)
	int AI_next_offset;			// ��һ��������AI�ǵڼ���AI

	// A*�㷨
	TMXLayer* m_mapLayer;		//��Ƭ��ͼ��ͼ�����
	Size m_visibleSize;			//��Ļ�Ŀɼ��ߴ�
	mapNode** m_map;			//��ͼ����ָ��
	mapNode** m_map_t;			//��ʱ�õĵ�ͼ����ָ��
	// mapNode* m_origin;			//Ѱ·���ָ��
	mapNode* m_destination;		//Ѱ·�յ�ָ��
	DrawNode* m_draw[MAX_INGAME_AI_NUM];

	// ���̰���
	int keys[128];
	

};

#endif