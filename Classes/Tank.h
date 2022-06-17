#ifndef __TANK_H__
#define __TANK_H__

#include "BaseObject.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "Global.h"
#include "Bullet.h"
USING_NS_CC;
using namespace cocos2d;
using namespace cocostudio;
using namespace cocos2d::ui;

#define TANKBASELIFE 1    // TANK��������
#define TANKSPEED    1    // TANKĬ���ٶ�
#define TANKSIZE    32    // TANK�ߴ��С
#define TANK_UP      1	  // TANK����״̬
#define TANK_DOWN    2    // TANK����״̬
#define TANK_LEFT    3    // TANK����״̬
#define TANK_RIGHT   4    // TANK����״̬
#define TANK_STAY    5    // TANKֹͣ״̬

class Tank : public BaseObject
{
public:
	static Tank* create(int ID, float x, float y, int dir, int kind);
	void MoveUP();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void Fire();
	void Stay(int dir);
	void Blast();     // ��ըʱ���Զ�����lifeΪ0

	bool isMoving() { return m_isMoving; };
	Rect getRect() { return m_rect; };
	Vector<Bullet*> getBulletList() { return m_bulletList; };
	
	void MyDraw();	// ת��ʱ�滭����
private:
	virtual bool init(int ID, float x, float y, int dir, int kind);
	void Draw();
	void update(float t);
	void deleteObj(Sprite* obj);

private:
	Sprite*     m_sprite;          // ͼƬ����
	Texture2D*  m_texture;         // ����texture
	Vector<Bullet*> m_bulletList;  // �ӵ��б�

	bool  m_moveUp;             // ̹�������ƶ�
	bool  m_moveDown;           // ̹�������ƶ�
	bool  m_moveLeft;           // ̹�������ƶ�
	bool  m_moveRight;			// ̹�������ƶ�

	float m_frametime;			// �л�ͼƬʱ�䲽��
	float m_temptime;			// ÿ����temptime�л�ͼƬ
	int   m_texchange;			// ̹���Ĵ������л�����

	Rect  m_rect;               // ̹�˰�Χ��
	bool  m_isMoving;           // ̹�������ƶ�

	int   m_textureX;           
	int   m_textureY;
};

#endif