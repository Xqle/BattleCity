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

// ���
#define PLAYER_TAG 110
#define PLAYER_LIFE 5

// AI
#define AI_TAG 111
#define MAX_AI_NUM 5
#define MAX_INGAME_AI_NUM 3

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

	// ʵ�ּ��̻ص�
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

	// �Ʒְ��������

	// ��Ŀ��target ��score ��
	void add_score(int score) { m_score += score;  };

	// ��ȡĿ��target��ǰ����
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

	// ʤ��/ʧ��
	void gameover();
	void success();

private:
	Vector<Brick*>  m_bgList;     // �������б�
	Vector<Tank*>   m_tankList;   // ̹���б�
	Vector<Tank*>	m_drawList;   // �ѻ��Ƶ�̹��
	
	Vector<Bullet*> m_deleteBulletList;   // ɾ���ӵ��б�
	Vector<Brick*>  m_deleteBrickList;    // ɾ��ש���б�
	Vector<Tank*>   m_deleteTankList;     // ɾ��̹���б�

	// �ػ�Ŀ��
	Sprite* m_bird;			// ����
	Rect	m_bird_rect;	// �ж���
	double	m_bird_spawnpointX = 32 * UNIT;
	double	m_bird_spawnpointY = 4 * UNIT;

	// ���
	Tank* m_tank;       // ��̹��
	int	   player_life = PLAYER_LIFE;
	double player_spawnpointX = 32 * UNIT;
	double player_spawnpointY = 23 * UNIT;
	double invulnerable_timer = 0;	// �޵�ʱ��
	int enemy_kill = 0;		// ��ɱ��������

	// AI
	double AI_spawnpointX[MAX_INGAME_AI_NUM] = { 7 * UNIT + 8, 32 * UNIT + 8, 57 * UNIT + 8};
	double AI_spawnpointY = 44 * UNIT + 8;	// AI ������
	int AI_destinationX[MAX_INGAME_AI_NUM] = { 17, 31, 46 };
	int AI_destinationY[MAX_INGAME_AI_NUM] = { 44, 37, 44 };
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
	mapNode* m_destination[MAX_INGAME_AI_NUM];		//Ѱ·�յ�ָ��
	DrawNode* m_draw[MAX_AI_NUM];

	// ���̰���
	int keys[128];

	// ʧ���ж�
	bool is_success;
	bool is_gameover;
};

#endif