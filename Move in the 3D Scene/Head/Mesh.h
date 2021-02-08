
#pragma once

//Mesh.h
//��ȡ����Ⱦ��̬����ģ��
//���ÿ����ͬ��ͼģ�ͽ��зֱ���Ⱦ
#pragma warning( disable:4995 )
#pragma warning( disable:4244 )

#include "DXUT\Core\DXUT.h"
#include <stdlib.h>
#include <memory.h>
#include <time.h>



struct Mesh
{
	LPD3DXMESH              g_pMesh[MESHNUM];  //����ģ�Ͷ���
	D3DMATERIAL9*           g_pMeshMaterials[MESHNUM];  //����ģ�Ͳ���
	LPDIRECT3DTEXTURE9*     g_pMeshTextures[MESHNUM];  //����ģ������
	DWORD                   g_dwNumMaterials[MESHNUM];    //����ģ�Ͳ�������
}SMesh;


struct SizeData
{
	float lx,ly,lz;//�ظ���ߴ�
	float deltax,deltay,deltaz;//ԭ��������������ƫ����(������Ϊģ��z���½�����ԭ��z������ͬ)
};


//�˽ṹ����������ģ�͵ı任��Ϣ
struct Transform 
{
	float xScale,yScale,zScale; //�ظ�������ϵ�� 
	float xMove,yMove,zMove;//�ظ���λ����
	float xRotation,yRotation,zRotation;//�Ƹ�����ת��
};

Transform meshTransform[MAP][MAP];
int meshType[MAP][MAP];


//����λ������
struct Position 
{
	int x,z;
};


LPCTSTR MESHNAME[MESHNUM]=
{   //����ģ����Ϣ		3D�������   3D�������
	L"Mesh\\С��¥.X",		//
	L"Mesh\\Q��СԺ.X",   //
	L"Mesh\\���Ϸ���.X",
	L"Mesh\\��1.X",
	L"Mesh\\��2.X",
	L"Mesh\\��3.X"
};


LPCTSTR TEXTUREPOSITION=L"Mesh\\MeshTexture\\";//����ģ�������ļ����ļ���Ŀ¼

void RemovePathFromFileName(LPSTR fullPath, LPWSTR fileName);
VOID SetMeshWorldMatrix(IDirect3DDevice9* pd3dDevice,Transform &transformData);
void MakeTransform(Position position,int typeNum,Transform &transformData);
void MakeMapTransform();


HRESULT Meshld(IDirect3DDevice9* pd3dDevice)
{
	//�Ӵ����ļ���������ģ��
	for(int i=0;i<MESHNUM;i++)
	{
		LPD3DXBUFFER pD3DXMtrlBuffer;  //�洢����ģ�Ͳ��ʵĻ���������
		if( FAILED( D3DXLoadMeshFromX( MESHNAME[i], D3DXMESH_MANAGED, 
			pd3dDevice, NULL, 
			&pD3DXMtrlBuffer, NULL, &SMesh.g_dwNumMaterials[i], 
			&SMesh.g_pMesh[i] ) ) )
		{
			MessageBox(NULL, L"Could not find file", MESHNAME[i], MB_OK);
			return E_FAIL;
		}

		//������ģ������ȡ�������Ժ������ļ��� 
		D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
		SMesh.g_pMeshMaterials[i] = new D3DMATERIAL9[SMesh.g_dwNumMaterials[i]];

		if( SMesh.g_pMeshMaterials[i] == NULL )
			return E_OUTOFMEMORY;

		SMesh.g_pMeshTextures[i]  = new LPDIRECT3DTEXTURE9[SMesh.g_dwNumMaterials[i]];
		if( SMesh.g_pMeshTextures == NULL )
			return E_OUTOFMEMORY;

		//�����ȡ����ģ�Ͳ������Ժ������ļ���
		for( DWORD j=0; j<SMesh.g_dwNumMaterials[i]; j++ )
		{
			//��������
			SMesh.g_pMeshMaterials[i][j] = d3dxMaterials[j].MatD3D;
			SMesh.g_pMeshMaterials[i][j].Ambient = SMesh.g_pMeshMaterials[i][j].Diffuse;


			SMesh.g_pMeshTextures[i][j] = NULL;
			if( d3dxMaterials[j].pTextureFilename != NULL && 
				strlen(d3dxMaterials[j].pTextureFilename) > 0 )
			{
				//��ȡ�����ļ���
				WCHAR filename[256];
				RemovePathFromFileName(d3dxMaterials[j].pTextureFilename, filename);

				//��������
				if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, filename, 
					&(SMesh.g_pMeshTextures[i][j]) ) ) )
				{
					MessageBox(NULL, L"Could not find texture file", L"Mesh", MB_OK);
				}
			}
		}
		//�ͷ��ڼ���ģ���ļ�ʱ�����ı���ģ�Ͳ��ʺ��������ݵĻ���������
		pD3DXMtrlBuffer->Release();
	}
	memset(meshType,-1,sizeof(int)*MAP*MAP);
	MakeMapTransform();
	return S_OK;
}



VOID MeshPaint(IDirect3DDevice9* pd3dDevice)
{
	
	//��������׶λ��
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );  //Ĭ������
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	pd3dDevice->SetRenderState( D3DRS_ZENABLE, true );       //Ĭ�ϼ���
	pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );  //���ñȽϺ�����Ĭ��ΪD3DCMP_LESSEQUAL
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);    //Ĭ��ֵΪTRUE

		
	//��ʼ��Ⱦ����
	for(int x=0;x<MAP;x++)
	{
		for (int z=0;z<MAP;z++)
		{
			if (meshType[x][z]==-1) continue;
			if ((meshType[x][z]>1) && (meshType[x][z]<6))continue;
			SetMeshWorldMatrix(pd3dDevice,meshTransform[x][z]);  //�����������

			
			//�ƻ�Alpha���, ����ALPHA���ϵ��
			pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   true );
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	
			//����������Ⱦ״̬
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

			//����Alpha��������
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

			//�����Ⱦ����ģ��
			for( DWORD j=0; j<SMesh.g_dwNumMaterials[meshType[x][z]]; j++ )
			//for( int j=SMesh.g_dwNumMaterials[meshType[x][z]]-1; j>=0;j-- )
			{
				
				//���ò��Ϻ�����
				pd3dDevice->SetMaterial( &(SMesh.g_pMeshMaterials[meshType[x][z]][j]) );
				pd3dDevice->SetTexture( 0, SMesh.g_pMeshTextures[meshType[x][z]][j] );

				//��Ⱦģ��
				SMesh.g_pMesh[meshType[x][z]]->DrawSubset( j );
			}
		}
	}
	
	for(int x=0;x<MAP;x++)
	{
		for (int z=0;z<MAP;z++)
		{
			if ((meshType[x][z] < 2) || (meshType[x][z] > 5))continue;
			SetMeshWorldMatrix(pd3dDevice,meshTransform[x][z]);  //�����������

			//�ƻ�Alpha���, ����ALPHA���ϵ��
			pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   true );
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	
			//����������Ⱦ״̬
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

			//����Alpha��������
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

			//�����Ⱦ����ģ��
	        for( int j=SMesh.g_dwNumMaterials[meshType[x][z]]-1; j>=0;j-- )
			//for( DWORD j=0; j<SMesh.g_dwNumMaterials[meshType[x][z]]; j++ )
			{
				
				//���ò��Ϻ�����
				pd3dDevice->SetMaterial( &(SMesh.g_pMeshMaterials[meshType[x][z]][j]) );
				pd3dDevice->SetTexture( 0, SMesh.g_pMeshTextures[meshType[x][z]][j] );

				//��Ⱦģ��
				SMesh.g_pMesh[meshType[x][z]]->DrawSubset( j );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Desc: �Ӿ���·������ȡ�����ļ���
//-----------------------------------------------------------------------------
void RemovePathFromFileName(LPSTR fullPath, LPWSTR fileName)
{
	//�Ƚ�fullPath�����ͱ任ΪLPWSTR
	WCHAR wszBuf[MAX_PATH];
	MultiByteToWideChar( CP_ACP, 0, fullPath, -1, wszBuf, MAX_PATH );
	wszBuf[MAX_PATH-1] = L'\0';

	WCHAR* wszFullPath = wszBuf;

	lstrcpy(fileName,TEXTUREPOSITION);
	//�Ӿ���·������ȡ�ļ���
	LPWSTR pch=wcsrchr(wszFullPath,'\\');
	if (pch)
		lstrcpy(fileName+17, ++pch);
	else
		lstrcpy(fileName+17, wszFullPath);
}


//-----------------------------------------------------------------------------
// Desc: �����������
//-----------------------------------------------------------------------------
VOID SetMeshWorldMatrix(IDirect3DDevice9* pd3dDevice,Transform &Transformdata)
{
	//�����������������

	//����ƽ�ƾ���
	D3DXMATRIXA16 matWorldMove;
	D3DXMatrixIdentity( &matWorldMove );
	D3DXMatrixTranslation(&matWorldMove,Transformdata.xMove,Transformdata.yMove,Transformdata.zMove);

	//�������ž���
	D3DXMATRIXA16 matWorldScale;
	D3DXMatrixIdentity( &matWorldScale );
	D3DXMatrixScaling(&matWorldScale,Transformdata.xScale,Transformdata.yScale,Transformdata.zScale);

	//������ת����
	D3DXMATRIXA16 matWorldRotationX,matWorldRotationY,matWorldRotationZ;
	D3DXMatrixIdentity( &matWorldRotationX );
	D3DXMatrixIdentity( &matWorldRotationY );
	D3DXMatrixIdentity( &matWorldRotationZ );
	D3DXMatrixRotationX(&matWorldRotationX,Transformdata.xRotation/360.0f*2.0f*D3DX_PI);
	D3DXMatrixRotationY(&matWorldRotationY,Transformdata.yRotation/360.0f*2.0f*D3DX_PI);
	D3DXMatrixRotationZ(&matWorldRotationZ,Transformdata.zRotation/360.0f*2.0f*D3DX_PI);

	//�����������
	D3DXMATRIXA16 matWorld;
	//D3DXMatrixMultiply(&matWorld,&matWorldScale,&matWorldMove);
	
	matWorld=matWorldScale*matWorldRotationX*matWorldRotationY*matWorldRotationZ*matWorldMove;//ע��˳��
	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

}


//���ݸõ�ͼԪ������ ��ͼ���� ��Ԫ������ ��������������еĸ��任ϵ��
void MakeTransform(Position position,int meshType,Transform &transformData)
{
	float changeSizeXYZ = 1.0f;
	
	transformData.xMove = 0;
	transformData.yMove = 0;
	transformData.zMove = 0;
	transformData.xRotation = 0;
	transformData.yRotation = 0;
	transformData.zRotation = 0;

	switch (meshType)
	{
	case 0: //С��¥
		changeSizeXYZ = 0.2;
		transformData.yRotation = 90;
		break;
	case 1: //Q��СԺ
		changeSizeXYZ = 0.25;
		transformData.yRotation = -90;
		transformData.yMove = -0.5f;
		transformData.zMove = -18.0f;
		break;
	case 2: //���Ϸ���
		transformData.yRotation = 90;
		transformData.yMove = -0.2f;
		break;
	case 3:
	case 4:
	case 5:
		changeSizeXYZ += (rand()%10)/10.0f;//��������һ��仯~
		break;
	default:;
	}

	transformData.xMove += (DISPLAY/MAP)*(position.x+0.5);
	transformData.zMove += (DISPLAY/MAP)*(position.z+0.5);
	transformData.yMove -= -0.05f;

	transformData.xScale = (DISPLAY/MAP)/250.0f*changeSizeXYZ;
	transformData.yScale = (DISPLAY/MAP)/250.0f*changeSizeXYZ;
	transformData.zScale = (DISPLAY/MAP)/250.0f*changeSizeXYZ;
}


void MeshClearUp()
{
	for(int i=0;i<MESHNUM;i++)
	{
		//�ͷ�����ģ�Ͳ���
		if( SMesh.g_pMeshMaterials[i] != NULL ) 
			delete[] SMesh.g_pMeshMaterials[i];

		//�ͷ�����ģ������
		if( SMesh.g_pMeshTextures[i] )
		{
			//SMesh.g_pMesh[i];
			for( DWORD j = 0; j < SMesh.g_dwNumMaterials[i]; j++ )
			{
				if( SMesh.g_pMeshTextures[i][j] )
					SMesh.g_pMeshTextures[i][j]->Release();
			}
			delete[] SMesh.g_pMeshTextures[i];
		}

		//�ͷ�����ģ�Ͷ���
		if( SMesh.g_pMesh[i] != NULL )
			SMesh.g_pMesh[i]->Release();
	}
}


//����ȫ��ͼ�任ϵ��
void MakeMapTransform()
{
	Position position;
	srand(time(0));//��ʱ�����ӳ�ʼ���������
	for(int x=0;x<MAP;x++)
	{
		for (int z=0;z<MAP;z++)
		{
			position.x=x;
			position.z=z;
			if(MapType[x][z] == 1/*TEXTURENUM*/) continue;//���������޾�̬����Ԫ��������
			meshType[x][z]=MapType[x][z]-5;//TEXTURENUM;
			MakeTransform(position,meshType[x][z],meshTransform[x][z]);//���ɱ任ϵ��
		}
	}
}