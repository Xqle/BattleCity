#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "cocos2d.h"
#include "ODSocket/ODSocket.h"
#include "Tank.h"
#include "Brick.h"
USING_NS_CC;
using namespace cocos2d;

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

	// �����紫������Ϣ������Ӧ
	void addTank(int id, float x, float y, int dir, int kind);
	void addFire(Tank* tank);

	// ʵ�ּ��̻ص�
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event); 

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };

private:
	Vector<Brick*>  m_bgList;     // �������б�
	Vector<Tank*>   m_tankList;   // ̹���б�
	Tank*           m_tank;       // ��̹��
	Vector<Tank*>	m_drawList;   // �ѻ��Ƶ�̹��
	
	Tank*           m_maxTank[50];        // �������ӿͻ���
	Vector<Tank*>   m_shouldFireList;     // ��¼��Ҫ�����̹�� - ������յ�������Ϣ��̹��

	Vector<Bullet*> m_deleteBulletList;   // ɾ���ӵ��б�
	Vector<Brick*>  m_deleteBrickList;    // ɾ��ש���б�
	Vector<Tank*>   m_deleteTankList;     // ɾ��̹���б�
};

#endif