
//HeadDef.h
//�б�������Լ���д��ͷ�ļ�

#pragma once

#define MAP 10        //��ͼ�������귶Χ��С
#define DISPLAY (FLOAT)20.0 //�����������귶Χ
#define WRAPNUM (FLOAT)12.0 //����·��������ص�Ѱַ��Χ
#define MAPMESHMINNUM 5
#define MAPMESHMAXNUM 8
#define MESHNUM 6  //��̬��ͼģ������

int MapType[MAP][MAP]= //�¶�����
{
	1, 10, 9, 9, 9, 8, 1, 1, 1, 10,
	1, 1, 1, 1, 1, 1, 1, 6, 1, 8,
	1, 1, 10, 1, 8, 1, 1, 1, 1, 1,
	1, 7, 1, 5, 1, 7, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 6, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 5, 1, 7, 1, 5, 1, 9, 8,
	1, 1, 1, 8, 1, 10, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 6, 1,
	1, 10, 9, 8, 9, 8, 1, 1, 1, 1
};


int RainNum = 0;
int Skylu = 200;

#include "Mesh.h"
#include "Ground.h"
#include "info.h"
#include "Thread.h"
#include "Cylinder.h"
#include "Climate.h"
#include "AllocateHierarchy.h"
#include "SkinMesh.h"
//#include "BattleEffect.h"
