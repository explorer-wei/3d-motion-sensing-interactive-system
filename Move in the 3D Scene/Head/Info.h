#pragma once

#include "DXUT\Core\DXUT.h"

LPDIRECT3DTEXTURE9        g_pInfoBack1Tex;  //��Ϣ����
LPDIRECT3DTEXTURE9        g_pInfoBack2Tex;  //��Ϣ����



HRESULT InfoBackld(IDirect3DDevice9* pd3dDevice)
{
	//������Ϣ����
	if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, L"Texture\\InfoBack1.jpg", &g_pInfoBack1Tex ) ) )
	{
		MessageBox(NULL, L"����������Ϣʧ��", L"Info.exe", MB_OK);
		return E_FAIL;
	}

	if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, L"Texture\\InfoBack2.jpg", &g_pInfoBack2Tex ) ) )
	{
		MessageBox(NULL, L"����������Ϣʧ��", L"Info.exe", MB_OK);
		return E_FAIL;
	}

    return S_OK;
}

VOID InfoBackPaint(IDirect3DDevice9 *pd3dDevice, ID3DXSprite *g_pSprite9)
{

	float wndWidth = DXUTGetWindowWidth(); //���ڿ��
    float wndHeight= DXUTGetWindowHeight(); //���ڿ��

	g_pSprite9->Begin(D3DXSPRITE_ALPHABLEND);
		
	D3DXMATRIXA16 matScaling;
	D3DXMATRIXA16 matTran;
	D3DXMatrixTranslation(&matTran, 0.0f, 30.0f, 0.0f);		
	D3DXMatrixScaling(&matScaling, 0.5f, 0.5f, 1.0f);
	g_pSprite9->SetTransform(&(matScaling*matTran));
	g_pSprite9->Draw(g_pInfoBack1Tex, NULL, NULL, NULL, 0xffffffff);

	D3DXMatrixTranslation(&matTran, 0.0f, 350.0f, 0.0f);		
	D3DXMatrixScaling(&matScaling, 0.5f, 0.795f, 1.0f);
	g_pSprite9->SetTransform(&(matScaling*matTran));
	g_pSprite9->Draw(g_pInfoBack2Tex, NULL, NULL, NULL, 0xffffffff);

	g_pSprite9->End();
}

VOID InfoBackClearUp()
{
	SAFE_RELEASE( g_pInfoBack1Tex );
	SAFE_RELEASE( g_pInfoBack2Tex );
}
