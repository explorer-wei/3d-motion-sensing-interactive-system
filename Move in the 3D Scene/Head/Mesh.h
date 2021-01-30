
#pragma once

//Mesh.h
//读取、渲染静态场景模型
//针对每个不同地图模型进行分别渲染
#pragma warning( disable:4995 )
#pragma warning( disable:4244 )

#include "DXUT\Core\DXUT.h"
#include <stdlib.h>
#include <memory.h>
#include <time.h>



struct Mesh
{
	LPD3DXMESH              g_pMesh[MESHNUM];  //网格模型对象
	D3DMATERIAL9*           g_pMeshMaterials[MESHNUM];  //网格模型材质
	LPDIRECT3DTEXTURE9*     g_pMeshTextures[MESHNUM];  //网格模型纹理
	DWORD                   g_dwNumMaterials[MESHNUM];    //网格模型材质数量
}SMesh;


struct SizeData
{
	float lx,ly,lz;//沿各轴尺寸
	float deltax,deltay,deltaz;//原点相对世界坐标的偏移量(不妨认为模型z轴下届与其原点z坐标相同)
};


//此结构体用来生成模型的变换信息
struct Transform 
{
	float xScale,yScale,zScale; //沿各轴缩放系数 
	float xMove,yMove,zMove;//沿各轴位移量
	float xRotation,yRotation,zRotation;//绕各轴旋转量
};

Transform meshTransform[MAP][MAP];
int meshType[MAP][MAP];


//传递位置坐标
struct Position 
{
	int x,z;
};


LPCTSTR MESHNAME[MESHNUM]=
{   //保存模型信息		3D类型序号   3D数组序号
	L"Mesh\\小酒楼.X",		//
	L"Mesh\\Q版小院.X",   //
	L"Mesh\\江南房子.X",
	L"Mesh\\树1.X",
	L"Mesh\\树2.X",
	L"Mesh\\树3.X"
};


LPCTSTR TEXTUREPOSITION=L"Mesh\\MeshTexture\\";//保存模型纹理文件的文件夹目录

void RemovePathFromFileName(LPSTR fullPath, LPWSTR fileName);
VOID SetMeshWorldMatrix(IDirect3DDevice9* pd3dDevice,Transform &transformData);
void MakeTransform(Position position,int typeNum,Transform &transformData);
void MakeMapTransform();


HRESULT Meshld(IDirect3DDevice9* pd3dDevice)
{
	//从磁盘文件加载网格模型
	for(int i=0;i<MESHNUM;i++)
	{
		LPD3DXBUFFER pD3DXMtrlBuffer;  //存储网格模型材质的缓冲区对象
		if( FAILED( D3DXLoadMeshFromX( MESHNAME[i], D3DXMESH_MANAGED, 
			pd3dDevice, NULL, 
			&pD3DXMtrlBuffer, NULL, &SMesh.g_dwNumMaterials[i], 
			&SMesh.g_pMesh[i] ) ) )
		{
			MessageBox(NULL, L"Could not find file", MESHNAME[i], MB_OK);
			return E_FAIL;
		}

		//从网格模型中提取材质属性和纹理文件名 
		D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
		SMesh.g_pMeshMaterials[i] = new D3DMATERIAL9[SMesh.g_dwNumMaterials[i]];

		if( SMesh.g_pMeshMaterials[i] == NULL )
			return E_OUTOFMEMORY;

		SMesh.g_pMeshTextures[i]  = new LPDIRECT3DTEXTURE9[SMesh.g_dwNumMaterials[i]];
		if( SMesh.g_pMeshTextures == NULL )
			return E_OUTOFMEMORY;

		//逐块提取网格模型材质属性和纹理文件名
		for( DWORD j=0; j<SMesh.g_dwNumMaterials[i]; j++ )
		{
			//材料属性
			SMesh.g_pMeshMaterials[i][j] = d3dxMaterials[j].MatD3D;
			SMesh.g_pMeshMaterials[i][j].Ambient = SMesh.g_pMeshMaterials[i][j].Diffuse;


			SMesh.g_pMeshTextures[i][j] = NULL;
			if( d3dxMaterials[j].pTextureFilename != NULL && 
				strlen(d3dxMaterials[j].pTextureFilename) > 0 )
			{
				//获取纹理文件名
				WCHAR filename[256];
				RemovePathFromFileName(d3dxMaterials[j].pTextureFilename, filename);

				//创建纹理
				if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, filename, 
					&(SMesh.g_pMeshTextures[i][j]) ) ) )
				{
					MessageBox(NULL, L"Could not find texture file", L"Mesh", MB_OK);
				}
			}
		}
		//释放在加载模型文件时创建的保存模型材质和纹理数据的缓冲区对象
		pD3DXMtrlBuffer->Release();
	}
	memset(meshType,-1,sizeof(int)*MAP*MAP);
	MakeMapTransform();
	return S_OK;
}



VOID MeshPaint(IDirect3DDevice9* pd3dDevice)
{
	
	//设置纹理阶段混合
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );  //默认设置
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	pd3dDevice->SetRenderState( D3DRS_ZENABLE, true );       //默认激活
	pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );  //设置比较函数，默认为D3DCMP_LESSEQUAL
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);    //默认值为TRUE

		
	//开始渲染场景
	for(int x=0;x<MAP;x++)
	{
		for (int z=0;z<MAP;z++)
		{
			if (meshType[x][z]==-1) continue;
			if ((meshType[x][z]>1) && (meshType[x][z]<6))continue;
			SetMeshWorldMatrix(pd3dDevice,meshTransform[x][z]);  //设置世界矩阵

			
			//计划Alpha混合, 设置ALPHA混合系数
			pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   true );
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	
			//设置纹理渲染状态
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

			//纹理Alpha操作设置
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

			//逐块渲染网格模型
			for( DWORD j=0; j<SMesh.g_dwNumMaterials[meshType[x][z]]; j++ )
			//for( int j=SMesh.g_dwNumMaterials[meshType[x][z]]-1; j>=0;j-- )
			{
				
				//设置材料和纹理
				pd3dDevice->SetMaterial( &(SMesh.g_pMeshMaterials[meshType[x][z]][j]) );
				pd3dDevice->SetTexture( 0, SMesh.g_pMeshTextures[meshType[x][z]][j] );

				//渲染模型
				SMesh.g_pMesh[meshType[x][z]]->DrawSubset( j );
			}
		}
	}
	
	for(int x=0;x<MAP;x++)
	{
		for (int z=0;z<MAP;z++)
		{
			if ((meshType[x][z] < 2) || (meshType[x][z] > 5))continue;
			SetMeshWorldMatrix(pd3dDevice,meshTransform[x][z]);  //设置世界矩阵

			//计划Alpha混合, 设置ALPHA混合系数
			pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   true );
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	
			//设置纹理渲染状态
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

			//纹理Alpha操作设置
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

			//逐块渲染网格模型
	        for( int j=SMesh.g_dwNumMaterials[meshType[x][z]]-1; j>=0;j-- )
			//for( DWORD j=0; j<SMesh.g_dwNumMaterials[meshType[x][z]]; j++ )
			{
				
				//设置材料和纹理
				pd3dDevice->SetMaterial( &(SMesh.g_pMeshMaterials[meshType[x][z]][j]) );
				pd3dDevice->SetTexture( 0, SMesh.g_pMeshTextures[meshType[x][z]][j] );

				//渲染模型
				SMesh.g_pMesh[meshType[x][z]]->DrawSubset( j );
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Desc: 从绝对路径中提取纹理文件名
//-----------------------------------------------------------------------------
void RemovePathFromFileName(LPSTR fullPath, LPWSTR fileName)
{
	//先将fullPath的类型变换为LPWSTR
	WCHAR wszBuf[MAX_PATH];
	MultiByteToWideChar( CP_ACP, 0, fullPath, -1, wszBuf, MAX_PATH );
	wszBuf[MAX_PATH-1] = L'\0';

	WCHAR* wszFullPath = wszBuf;

	lstrcpy(fileName,TEXTUREPOSITION);
	//从绝对路径中提取文件名
	LPWSTR pch=wcsrchr(wszFullPath,'\\');
	if (pch)
		lstrcpy(fileName+17, ++pch);
	else
		lstrcpy(fileName+17, wszFullPath);
}


//-----------------------------------------------------------------------------
// Desc: 设置世界矩阵
//-----------------------------------------------------------------------------
VOID SetMeshWorldMatrix(IDirect3DDevice9* pd3dDevice,Transform &Transformdata)
{
	//创建并设置世界矩阵

	//设置平移矩阵
	D3DXMATRIXA16 matWorldMove;
	D3DXMatrixIdentity( &matWorldMove );
	D3DXMatrixTranslation(&matWorldMove,Transformdata.xMove,Transformdata.yMove,Transformdata.zMove);

	//设置缩放矩阵
	D3DXMATRIXA16 matWorldScale;
	D3DXMatrixIdentity( &matWorldScale );
	D3DXMatrixScaling(&matWorldScale,Transformdata.xScale,Transformdata.yScale,Transformdata.zScale);

	//设置旋转矩阵
	D3DXMATRIXA16 matWorldRotationX,matWorldRotationY,matWorldRotationZ;
	D3DXMatrixIdentity( &matWorldRotationX );
	D3DXMatrixIdentity( &matWorldRotationY );
	D3DXMatrixIdentity( &matWorldRotationZ );
	D3DXMatrixRotationX(&matWorldRotationX,Transformdata.xRotation/360.0f*2.0f*D3DX_PI);
	D3DXMatrixRotationY(&matWorldRotationY,Transformdata.yRotation/360.0f*2.0f*D3DX_PI);
	D3DXMatrixRotationZ(&matWorldRotationZ,Transformdata.zRotation/360.0f*2.0f*D3DX_PI);

	//设置世界矩阵
	D3DXMATRIXA16 matWorld;
	//D3DXMatrixMultiply(&matWorld,&matWorldScale,&matWorldMove);
	
	matWorld=matWorldScale*matWorldRotationX*matWorldRotationY*matWorldRotationZ*matWorldMove;//注意顺序！
	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

}


//根据该地图元素所在 地图坐标 、元素类型 生成其世界矩阵中的各变换系数
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
	case 0: //小酒楼
		changeSizeXYZ = 0.2;
		transformData.yRotation = 90;
		break;
	case 1: //Q版小院
		changeSizeXYZ = 0.25;
		transformData.yRotation = -90;
		transformData.yMove = -0.5f;
		transformData.zMove = -18.0f;
		break;
	case 2: //江南房子
		transformData.yRotation = 90;
		transformData.yMove = -0.2f;
		break;
	case 3:
	case 4:
	case 5:
		changeSizeXYZ += (rand()%10)/10.0f;//让树产生一点变化~
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
		//释放网格模型材质
		if( SMesh.g_pMeshMaterials[i] != NULL ) 
			delete[] SMesh.g_pMeshMaterials[i];

		//释放网格模型纹理
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

		//释放网格模型对象
		if( SMesh.g_pMesh[i] != NULL )
			SMesh.g_pMesh[i]->Release();
	}
}


//生成全地图变换系数
void MakeMapTransform()
{
	Position position;
	srand(time(0));//用时间种子初始化随机函数
	for(int x=0;x<MAP;x++)
	{
		for (int z=0;z<MAP;z++)
		{
			position.x=x;
			position.z=z;
			if(MapType[x][z] == 1/*TEXTURENUM*/) continue;//若该坐标无静态场景元素则跳过
			meshType[x][z]=MapType[x][z]-5;//TEXTURENUM;
			MakeTransform(position,meshType[x][z],meshTransform[x][z]);//生成变换系数
		}
	}
}