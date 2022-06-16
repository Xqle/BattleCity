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
static int tankcount = 0;     // ��¼��ǰ̹����
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

private:
	Vector<Brick*>  m_bgList;     // �������б�
	Vector<Tank*>   m_tankList;   // ̹���б�
	Tank*           m_tank;       // ��̹��
	Vector<Tank*>	m_drawList;   // �ѻ��Ƶ�̹��
	
	Vector<Bullet*> m_deleteBulletList;   // ɾ���ӵ��б�
	Vector<Brick*>  m_deleteBrickList;    // ɾ��ש���б�
	Vector<Tank*>   m_deleteTankList;     // ɾ��̹���б�

	// AI
	double AI_spawnpointX[MAX_INGAME_AI_NUM] = { WINDOWWIDTH / 4, WINDOWWIDTH / 2, WINDOWWIDTH * 3 / 4 };
	double AI_spawnpointY = 600;	// AI ������
	double AI_update_delta;		// AI_action�ļ�ʱ��
	int AI_remain_num;			// ʣ����ٸ�AI			(<= MAX_AI_NUM)
	int AI_ingame_num;			// ��ǰ����Ļ���AI		(<= MAX_INGAME_AI_NUM)
	int AI_next_offset;			// ��һ��������AI�ǵڼ���AI

	// ���̰���
	int keys[128];
};

#endif