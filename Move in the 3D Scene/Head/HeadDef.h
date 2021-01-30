
//HeadDef.h
//列表出所有自己编写的头文件

#pragma once

#define MAP 10        //地图抽象坐标范围大小
#define DISPLAY (FLOAT)20.0 //定义世界坐标范围
#define WRAPNUM (FLOAT)12.0 //定义路面纹理可重叠寻址范围
#define MAPMESHMINNUM 5
#define MAPMESHMAXNUM 8
#define MESHNUM 6  //静态地图模型数量

int MapType[MAP][MAP]= //下东上西
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
