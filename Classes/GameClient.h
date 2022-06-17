#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "cocos2d.h"
#include "Tank.h"
#include "Brick.h"
#include "GamePause.h"
USING_NS_CC;
using namespace cocos2d;

enum tileBlock
{
	highLevelBrick, // �߼�ש��
	forest,
	lake,
	empty,
	brick			// ��ͨש��
};// ��ͼ������


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
	void drawBigBG_HighLevel(Vec2 position, int dir, int length);

	// �����紫������Ϣ������Ӧ
	void addTank(int id, float x, float y, int dir, int kind);
	void addFire(Tank* tank);

	// ʵ�ּ��̻ص�
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event); 

	void onTouchEnded(EventKeyboard::KeyCode keyCode, Event* event);

	void pressMenuButton(Ref* pSender, Widget::TouchEventType type);
	void pressReplayButton(Ref* pSender, Widget::TouchEventType type);
	void pressPauseButton(Ref* pSender, Widget::TouchEventType type);

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };
	tileBlock** block_status;	// ��ͼ������Ծ���

	// �Ʒְ��������

	// ��Ŀ��target ��score ��
	void add_score(int target, int score); 

	// ��ȡĿ��target��ǰ����
	int get_score(int target, int score);


	// UI related
	Button* Menubtn;
	Button* Replaybtn;
	Button* Pausebtn;

	TextBMFont* P1score;
	TextBMFont* P2score;

	bool stopped;

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

	int score_list[10]; // �Ʒְ�
};

#endif