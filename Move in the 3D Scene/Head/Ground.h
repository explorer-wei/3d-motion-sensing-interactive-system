#pragma once

#include "DXUT\Core\DXUT.h"

#define D3DFVF_GROUNDVERTEX   (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

struct GROUNDVERTEX
{
	D3DXVECTOR3 position;   //����λ��
	D3DXVECTOR3 normal;     //���㷨��
	FLOAT u,v ;             //������������
};

LPDIRECT3DVERTEXBUFFER9   g_pGroundVB = NULL;     //���㻺��������
LPDIRECT3DTEXTURE9        g_pTextureForGround;    //��������

HRESULT Groundld( IDirect3DDevice9 *pd3dDevice )
{
	HRESULT hr;

	//������������
	V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, L"Texture\\Ground.bmp", &g_pTextureForGround ) );

	//�������㻺����
	V_RETURN( pd3dDevice->CreateVertexBuffer( 4 * sizeof( GROUNDVERTEX ), 
		                                      0, D3DFVF_GROUNDVERTEX, D3DPOOL_DEFAULT, 
											  &g_pGroundVB,NULL ) );

	//��䶥�㻺���� ���� HW
    GROUNDVERTEX *pVertices;
	V_RETURN( g_pGroundVB->Lock(0, 0, (void **)&pVertices, 0) );

	pVertices[0].position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    pVertices[0].normal = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );  
    pVertices[0].u = 0.0f;
    pVertices[0].v = 0.0f;
	pVertices[1].position = D3DXVECTOR3( 20.0f, 0.0f, 0.0f );
    pVertices[1].normal = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );  
    pVertices[1].u = 1.0f;
    pVertices[1].v = 0.0f;
	pVertices[2].position = D3DXVECTOR3( 20.0f, 0.0f, 20.0f );
    pVertices[2].normal = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );  
    pVertices[2].u = 1.0f;
    pVertices[2].v = 1.0f;
	pVertices[3].position = D3DXVECTOR3( 0.0f, 0.0f, 20.0f );
    pVertices[3].normal = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );  
    pVertices[3].u = 0.0f;
    pVertices[3].v = 1.0f;

	g_pGroundVB->Unlock();

	return S_OK;
}

VOID GroundPaint( IDirect3DDevice9 *pd3dDevice )
{
	//�����������������
	D3DXMATRIXA16 matWorldForGround;
	D3DXMatrixIdentity( &matWorldForGround );
	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorldForGround );

	//���ò���
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	mtrl.Diffuse.r=100.0f;
	mtrl.Diffuse.g=100.0f;
	mtrl.Diffuse.b=100.f;
	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	pd3dDevice->SetMaterial( &mtrl );

	//����������Ⱦ״̬
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );  //Ĭ������
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	//����������˷�ʽ
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	//��ʼ������Ⱦ
	pd3dDevice->SetTexture( 0, g_pTextureForGround ); //��������
	pd3dDevice->SetStreamSource( 0, g_pGroundVB, 0, sizeof(GROUNDVERTEX) );
	pd3dDevice->SetFVF( D3DFVF_GROUNDVERTEX );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2);
}

VOID GroundClearUp()
{
	SAFE_RELEASE( g_pGroundVB );
	SAFE_RELEASE( g_pTextureForGround );
}