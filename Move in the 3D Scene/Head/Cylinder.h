//渲染参考小坐标

#pragma once

#include "DXUT\Core\DXUT.h"

#include <math.h>

#define D3DFVF_CYLINDERVERTEX   (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

struct CYLINDERVERTEX
{
	D3DXVECTOR3 position;  //顶点的位置
	D3DXVECTOR3 normal;    //顶点的方向量
	FLOAT u, v;            //顶点纹理坐标
	
};

LPDIRECT3DVERTEXBUFFER9 g_pCylinderVB     = NULL;    //圆柱顶点缓冲区对象
LPDIRECT3DVERTEXBUFFER9 g_pXAxisVB        = NULL;    //X轴顶点缓冲区对象
LPDIRECT3DVERTEXBUFFER9 g_pYAxisVB        = NULL;    //Y轴顶点缓冲区对象
LPDIRECT3DVERTEXBUFFER9 g_pZAxisVB        = NULL;    //Z轴顶点缓冲区对象
LPDIRECT3DTEXTURE9      g_pTextureForXAxis;          //X轴纹理对象
LPDIRECT3DTEXTURE9      g_pTextureForYAxis;          //Y轴纹理对象
LPDIRECT3DTEXTURE9      g_pTextureForZAxis;          //Z轴纹理对象
static D3DXMATRIXA16    matWorldForCylinder;         //圆柱的世界矩阵
D3DXMATRIX              matRot;
BYTE                    m_bKey[256];

float fRoll = 0.0f, fPitch = 0.0f, fYaw = 0.0f;
string part_control = "Bip01_Head";//控制部位的选择

extern CMyThread MyThread;//线程

HRESULT Cylinderld(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

    CYLINDERVERTEX *pVertices;

	//坐标轴，X杆
	V_RETURN( pd3dDevice->CreateVertexBuffer( 16 * sizeof( CYLINDERVERTEX ), 
		                                      0, D3DFVF_CYLINDERVERTEX, D3DPOOL_DEFAULT, 
											  &g_pXAxisVB,NULL ) );
    
	V_RETURN( g_pXAxisVB->Lock(0, 0, (void **)&pVertices, 0) );

	for (int i=0; i<5; i++ )
	{
		float theta = (2 * D3DX_PI * i) / 4;
		pVertices[2 * i].position = D3DXVECTOR3( 0.0f, 0.02f * sinf(theta),  0.02f * cosf(theta) );
        pVertices[2 * i].normal = D3DXVECTOR3( 0.0f, sinf(theta), cosf(theta) );
		pVertices[2 * i].u = ( (float) i) / 4;
		pVertices[2 * i].v = 1.0f;

		pVertices[2 * i + 1].position = D3DXVECTOR3( 1.0f, 0.02f * sinf(theta), 0.02f * cosf(theta) );
        pVertices[2 * i + 1].normal = D3DXVECTOR3( 0.0f, sinf(theta), cosf(theta) );
		pVertices[2 * i + 1].u = ( (float) i) / 4;
		pVertices[2 * i + 1].v = 0.0f;
	}

	//X坐标上的圆锥
	pVertices[10].position = D3DXVECTOR3( 1.5f, 0.0f,  0.0f );
	pVertices[10].normal= D3DXVECTOR3( 1.0f, 0.0f,  0.0f );
	pVertices[10].u = 1.0f;
	pVertices[10].v = 1.0f;
    for (int i=0; i<5; i++)
	{
		float theta = (2 * D3DX_PI * i) / 4;
		pVertices[i+11].position = D3DXVECTOR3( 1.0f, 0.1f * sinf(theta),  0.1f * cosf(theta) );
        pVertices[i+11].normal = D3DXVECTOR3( 0.0f, sinf(theta), cosf(theta) );
		pVertices[i+11].u = ( (float) i) / 4;
		pVertices[i+11].v = 0.0f;
	}

	g_pXAxisVB->Unlock();

	//坐标轴，Y杆
	V_RETURN( pd3dDevice->CreateVertexBuffer( 16 * sizeof( CYLINDERVERTEX ), 
		                                      0, D3DFVF_CYLINDERVERTEX, D3DPOOL_DEFAULT, 
											  &g_pYAxisVB,NULL ) );
    
	V_RETURN( g_pYAxisVB->Lock(0, 0, (void **)&pVertices, 0) );

	for (int i=0; i<5; i++ )
	{
		float theta = (2 * D3DX_PI * i) / 4;
		pVertices[2 * i].position = D3DXVECTOR3( 0.02f * sinf(theta), 0.0f, 0.02f * cosf(theta) );
        pVertices[2 * i].normal = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
		pVertices[2 * i].u = ( (float) i) / 4;
		pVertices[2 * i].v = 1.0f;

		pVertices[2 * i + 1].position = D3DXVECTOR3( 0.02f * sinf(theta), 1.0f, 0.02f * cosf(theta) );
        pVertices[2 * i + 1].normal = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
		pVertices[2 * i + 1].u = ( (float) i) / 4;
		pVertices[2 * i + 1].v = 0.0f;
	}

	//Y坐标上的圆锥
	pVertices[10].position = D3DXVECTOR3( 0.0f, 1.5f,  0.0f );
	pVertices[10].normal= D3DXVECTOR3( 0.0f, 1.0f,  0.0f );
	pVertices[10].u = 1.0f;
	pVertices[10].v = 1.0f;
    for (int i=0; i<5; i++)
	{
		float theta = (2 * D3DX_PI * i) / 4;
		pVertices[i+11].position = D3DXVECTOR3( 0.1f * sinf(theta), 1.0f, 0.1f * cosf(theta) );
        pVertices[i+11].normal = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
		pVertices[i+11].u = ( (float) i) / 4;
		pVertices[i+11].v = 0.0f;
	}

	g_pYAxisVB->Unlock();

	//坐标轴，Z杆
	V_RETURN( pd3dDevice->CreateVertexBuffer( 16 * sizeof( CYLINDERVERTEX ), 
		                                      0, D3DFVF_CYLINDERVERTEX, D3DPOOL_DEFAULT, 
											  &g_pZAxisVB,NULL ) );
    
	V_RETURN( g_pZAxisVB->Lock(0, 0, (void **)&pVertices, 0) );

	for (int i=0; i<5; i++ )
	{
		float theta = (2 * D3DX_PI * i) / 4;
		pVertices[2 * i].position = D3DXVECTOR3( 0.02f * sinf(theta), 0.02f * cosf(theta), 0.0f );
        pVertices[2 * i].normal = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
		pVertices[2 * i].u = ( (float) i) / 4;
		pVertices[2 * i].v = 1.0f;

		pVertices[2 * i + 1].position = D3DXVECTOR3( 0.02f * sinf(theta), 0.02f * cosf(theta), 1.0f );
        pVertices[2 * i + 1].normal = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
		pVertices[2 * i + 1].u = ( (float) i) / 4;
		pVertices[2 * i + 1].v = 0.0f;
	}

	//Z坐标上的圆锥
	pVertices[10].position = D3DXVECTOR3( 0.0f, 0.0f, 1.5f );
	pVertices[10].normal= D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	pVertices[10].u = 1.0f;
	pVertices[10].v = 1.0f;
    for (int i=0; i<5; i++)
	{
		float theta = (2 * D3DX_PI * i) / 4;
		pVertices[i+11].position = D3DXVECTOR3( 0.1f * sinf(theta), 0.1f * cosf(theta), 1.0f );
        pVertices[i+11].normal = D3DXVECTOR3( sinf(theta), cosf(theta), 0.0f );
		pVertices[i+11].u = ( (float) i) / 4;
		pVertices[i+11].v = 0.0f;
	}

	g_pZAxisVB->Unlock();

	D3DXMatrixIdentity( &matWorldForCylinder );
	
	//载入纹理
	V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, L"Texture\\red.bmp", &g_pTextureForXAxis ) );
	V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, L"Texture\\green.bmp", &g_pTextureForYAxis ) );
	V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, L"Texture\\blue.bmp", &g_pTextureForZAxis ) );

	//小坐标在世界坐标中的位置
	matWorldForCylinder._11 = 1.0f;
	matWorldForCylinder._12 = 0.0f;
	matWorldForCylinder._13 = 0.0f;
	matWorldForCylinder._21 = 0.0f;
	matWorldForCylinder._22 = 1.0f;
	matWorldForCylinder._23 = 0.0f;
	matWorldForCylinder._31 = 0.0f;
	matWorldForCylinder._32 = 0.0f;
	matWorldForCylinder._33 = 1.0f;
	matWorldForCylinder._41 = 7.0f;
	matWorldForCylinder._42 = 5.0f;
	matWorldForCylinder._43 = 20.0f;

	return S_OK;
}

void SetMatWorldForCylinder(IDirect3DDevice9* pd3dDevice)
{
	static long curTime=0;
	static float elapsetime=0;
	elapsetime = (timeGetTime()-curTime)/1000.0f;
	curTime = timeGetTime();

	fPitch = 0.0f, fYaw = 0.0f, fRoll = 0.0f;
    
	if (m_bKey['K']) fPitch -= 3*elapsetime;//绕X旋转
    if (m_bKey['I']) fPitch += 3*elapsetime;
	if (m_bKey['U']) fYaw   -= 3*elapsetime;//绕Y旋转
    if (m_bKey['O']) fYaw   += 3*elapsetime;
	if (m_bKey['L']) fRoll  -= 3*elapsetime;//绕Z旋转
    if (m_bKey['J']) fRoll  += 3*elapsetime;

	//更新小坐标方向
	D3DXQUATERNION qR;
	D3DXMATRIX matRot;
	D3DXQuaternionRotationYawPitchRoll( &qR, fYaw, fPitch, fRoll );
	D3DXMatrixRotationQuaternion( &matRot, &qR );
	D3DXMatrixMultiply(&matWorldForCylinder, &matRot, &matWorldForCylinder);

	//选择控制部位
	if (m_bKey['1']) part_control = "Bip01_R_UpperArm";//上臂
	if (m_bKey['2']) part_control = "Bip01_L_UpperArm";
	if (m_bKey['3']) part_control = "Bip01_R_Forearm";//前臂
	if (m_bKey['4']) part_control = "Bip01_L_Forearm";
	if (m_bKey['5']) part_control = "Bip01_R_Thigh";//大腿
	if (m_bKey['6']) part_control = "Bip01_L_Thigh";
	if (m_bKey['7']) part_control = "Bip01_R_Calf";//小腿
	if (m_bKey['8']) part_control = "Bip01_L_Calf";
	if (m_bKey['9']) part_control = "Bip01_Spine1";//腰;
	if (m_bKey['0']) part_control = "Bip01_Head";//头

    pd3dDevice->SetTransform( D3DTS_WORLD, &matWorldForCylinder );

}

void CylinderPaint(IDirect3DDevice9* pd3dDevice)
{
    //设置世界矩阵
	SetMatWorldForCylinder( pd3dDevice );
/*
	pd3dDevice->SetTexture( 0, g_pTextureForXAxis ); //设置纹理
	pd3dDevice->SetStreamSource( 0, g_pXAxisVB, 0, sizeof(CYLINDERVERTEX) );
	pd3dDevice->SetFVF( D3DFVF_CYLINDERVERTEX );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 4 );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 10, 4 );

	pd3dDevice->SetTexture( 0, g_pTextureForYAxis ); //设置纹理
	pd3dDevice->SetStreamSource( 0, g_pYAxisVB, 0, sizeof(CYLINDERVERTEX) );
	pd3dDevice->SetFVF( D3DFVF_CYLINDERVERTEX );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 4 );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 10, 4 );

	pd3dDevice->SetTexture( 0, g_pTextureForZAxis ); //设置纹理
	pd3dDevice->SetStreamSource( 0, g_pZAxisVB, 0, sizeof(CYLINDERVERTEX) );
	pd3dDevice->SetFVF( D3DFVF_CYLINDERVERTEX );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 4 );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 10, 4 );*/
}

void CylinderClearUp()
{
	SAFE_RELEASE( g_pCylinderVB );
	SAFE_RELEASE( g_pXAxisVB );
	SAFE_RELEASE( g_pYAxisVB );
	SAFE_RELEASE( g_pZAxisVB );
	SAFE_RELEASE( g_pTextureForXAxis );
	SAFE_RELEASE( g_pTextureForYAxis );
	SAFE_RELEASE( g_pTextureForZAxis );
}


