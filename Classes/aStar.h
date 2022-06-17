#ifndef  _A_STAR_H
#define _A_STAR_H

#include <iostream>
#include <math.h>

using namespace std;

#define MAP_WIDTH 60  
#define MAP_HEIGHT 40
#define UNIT 16 

//����ڵ�ĸ���״̬��
#define ACCESS 1		   //����ɵ�����ɵ���
#define NOT_ACCESS 2    
#define ORIGIN 3         //·����㡢�յ�
#define DESTINATION 4	   
#define IN_OPENLIST 5    //�ڵ���OPEN��CLOSED����
#define IN_CLOSEDLIST 6  

//����ڵ�ṹ��
struct mapNode
{
	int status;           //�ڵ��״̬��־
	int xCoordinate;	   //�ڵ�ĺᡢ������
	int yCoordinate;
	int fValue;		   //�ڵ��fֵ��gֵ��hֵ
	int gValue;
	int hValue;
	mapNode* parent;    //�ڵ�ĸ��ڵ�ָ��
};

//����OPEN��ṹ��
struct openList
{
	mapNode *openNode;
	openList* next;
};

//����CLOSED��ṹ��
struct closedList
{
	mapNode *closedNode;
	closedList* next;
};

void insertNodeToClosedList(closedList* close, openList* &open);

void calculateValues(mapNode** map, int x, int y, int i, mapNode* node, mapNode* destination);

void insertToOpenList(openList* open, mapNode* node);

bool ifChangeParent(mapNode** map, int x, int y, int i, mapNode * node);

bool checkNeighboringNodes(mapNode** map, openList* open, mapNode* node, mapNode* destination);


#endif

