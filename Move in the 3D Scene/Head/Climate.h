
#include <math.h>
#include "dxut/Core/DXUT.h"



//Climate.h
//此文件主要进行一些天气效果的实现

#define  PARTICLENUM  1000 //雪花粒子数量

float timeSize;
float timeLast;

#define lastLong (float)20.0  //控制从黑夜到白天过渡时间以及黑夜和白天的时间比例

//计算天空颜色
float calculateColorForSky()
{
	timeSize=(((int)(timeGetTime()/100))%255-127)*2.0f;
	timeSize=timeSize>0?timeSize:-timeSize;
	timeSize-=127.0f;
	timeLast=atan((double)(timeSize/lastLong))*2.0f/D3DX_PI;
	return (timeLast+atan((double)(127.0f/lastLong))*2.0/D3DX_PI)*115.0f;
}


/*void SetMtl(IDirect3DDevice9* pd3dDevice)
{
	//设置材质
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	mtrl.Diffuse.r=100.0f;
	mtrl.Diffuse.g=100.0f;
	mtrl.Diffuse.b=100.0f;
	mtrl.Diffuse.a = 100.0f;
	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.a = 100.0f;
	pd3dDevice->SetMaterial( &mtrl );
}*/


//--------------------------------------------------------------------------------------
//Desc:设置光线
//--------------------------------------------------------------------------------------
void SetLight(IDirect3DDevice9* pd3dDevice)
{
	//设置灯光
	D3DXVECTOR3 vecDir;

	//白昼灯光
	D3DLIGHT9 lightDay;
	ZeroMemory( &lightDay, sizeof(D3DLIGHT9) );
	lightDay.Type       = D3DLIGHT_DIRECTIONAL;

	lightDay.Diffuse.r  = 0.6025f+timeLast*0.0025f;;
	lightDay.Diffuse.g  = 0.6025f+timeLast*0.0025f;
	lightDay.Diffuse.b  = 0.6025f+timeLast*0.0025f;
	//float yl=-cosf((time/127.0f-1.0f)*D3DX_PI);
	//float zl=sinf((time/127.0f+1.0f)*D3DX_PI);
	vecDir = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
	//D3DXVec3Normalize( (D3DXVECTOR3*)&lightDay.Direction, &vecDir );
	lightDay.Direction = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
	lightDay.Range       = 2000.0f;
	pd3dDevice->SetLight( 0, &lightDay );
	//pd3dDevice->LightEnable( 0, true );
	pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE ); //默认设置
	
	int k=(int)(timeLast*10);   

	//设置环境光
	pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00aaaaaaaa+(0x00060606*k) );
}



//////////////////////////////////////////////////////////////////////////
//Raining 下雨场景部分
//////////////////////////////////////////////////////////////////////////

struct RAINVERTEX
{
	D3DXVECTOR3 vPos;    //顶点位置
//	D3DXVECTOR3 normal;
	DWORD dwColor;			//顶点颜色
};


#define D3DFVF_RAINVERTEX	(D3DFVF_XYZ|D3DFVF_DIFFUSE)

LPDIRECT3DVERTEXBUFFER9    g_pRainVB = NULL;        //雨点粒子顶点缓冲区
D3DXMATRIX                 g_matRain;              //雨点世界矩阵
D3DXVECTOR3                g_RainDir;              //雨点的下降方向

bool ifRaining=false;
int endNum=0;

//雨点粒子结构
struct  RainParticle
{
	float x, y, z;      //位置
	float fDspeed;       //下降速度
};

RainParticle rain[PARTICLENUM];  //雨点粒子数组

#pragma warning( disable:4305 )

HRESULT initRain(IDirect3DDevice9* pd3dDevice)
{
	//创建雨点粒子顶点缓冲区
	pd3dDevice->CreateVertexBuffer( 2*sizeof(RAINVERTEX),
		0, D3DFVF_RAINVERTEX,
		D3DPOOL_MANAGED, &g_pRainVB, NULL );

	//填充雨点粒子顶点缓冲区
																			
	RAINVERTEX*  pRainVertices;
	g_pRainVB->Lock(0, 0, (void**)&pRainVertices, 0);
	pRainVertices[0].vPos = D3DXVECTOR3(0.075, 0.375, 0);
	pRainVertices[0].dwColor = 0xffcccccc;
	pRainVertices[1].vPos = D3DXVECTOR3(0.225, 0.075, 0);
	pRainVertices[1].dwColor = 0xffcccccc;
	g_pRainVB->Unlock();

	return S_OK;
}

void resetRain()
{
	//初始化雨点粒子数组
//	srand(GetTickCount());
	RainNum=20;
	for(int i=0; i<PARTICLENUM; i++)
	{	
		rain[i].x        = float(rand()%50-10);
		rain[i].z        = float(rand()%50-10);
		rain[i].y        = float(rand()%10+10);
		rain[i].fDspeed   = 10.0f + (rand()%10);//20.0f;
	}

	//计算雨点下降方向
	g_RainDir = D3DXVECTOR3(3, 1, 0) - D3DXVECTOR3(1, 5, 0);
	D3DXVec3Normalize(&g_RainDir, &g_RainDir);
}

void rainRender(IDirect3DDevice9* pd3dDevice, float fElapsedTime)
{

	//计算每个雨点的当前位置
	for(int i=0; i<RainNum/*PARTICLENUM*/; i++)
	{
		if(rain[i].y<0)
			rain[i].y  = float(rand()%10+10);

		if(rain[i].x>30)
			rain[i].x = float(rand()%50-10);

		if(rain[i].z>100)
			rain[i].z = float(rand()%50-10);

		rain[i].x += rain[i].fDspeed * g_RainDir.x * fElapsedTime;
		rain[i].y += rain[i].fDspeed * g_RainDir.y * fElapsedTime;
		rain[i].z += rain[i].fDspeed * g_RainDir.z * fElapsedTime;
	}



	//渲染雨点
	
	pd3dDevice->SetTexture( 0, NULL );
	for(int i=0; i<RainNum; i++)
		{
			D3DXMatrixTranslation(&g_matRain, rain[i].x, rain[i].y, rain[i].z);
			pd3dDevice->SetTransform( D3DTS_WORLD,  &g_matRain );
			pd3dDevice->SetStreamSource(0, g_pRainVB, 0, sizeof(RAINVERTEX));
			pd3dDevice->SetFVF(D3DFVF_RAINVERTEX);
			pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);
		}
	
}


bool ifRainingEnd()
{
	for (int i=0;i<RainNum;i++)
	{
		if(rain[i].y>0) return false;
	}
	return true;
}


void rainClearUp()
{
	SAFE_RELEASE(g_pRainVB);
}


//雾化效果设定
void Fog(IDirect3DDevice9* pd3dDevice)
{
	static float fogStart = 1.0f;
	static float fogEnd = 20.0f;
	static float fogDensity=0.02f;
	pd3dDevice->SetRenderState(D3DRS_FOGENABLE,true);
	pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP2);
	pd3dDevice->SetRenderState(D3DRS_FOGSTART,*(DWORD*)&fogStart);
	pd3dDevice->SetRenderState(D3DRS_FOGEND,*(DWORD*)&fogEnd);
	pd3dDevice->SetRenderState(D3DRS_FOGDENSITY,*(DWORD*)&fogDensity);
	pd3dDevice->SetRenderState(D3DRS_FOGCOLOR,0xff999999);
}



//////////////////////////////////////////////////////////////////////////
//Desc:天空云雾效果实现
//////////////////////////////////////////////////////////////////////////
LPDIRECT3DVERTEXBUFFER9    g_pSkyVB;                //天空顶点缓冲区
LPDIRECT3DTEXTURE9         g_pCloudTex;              //云彩纹理

struct  SKYVERTEX
{
	float  x, y, z;    //顶点位置
	DWORD  dwColor;    //顶点颜色
	float  u,v ;	   //顶点纹理坐标     
};
#define D3DFVF_SKYVERTEX   (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)


#define angleTurn  10 //表示每次转过的角度（不是弧度）
#define RADIUS   60 //球形天空半径



struct SKYVERTEX4 
{
	SKYVERTEX point[4];
};

SKYVERTEX4 skyBall[180/angleTurn][360/angleTurn];

HRESULT initSky(IDirect3DDevice9* pd3dDevice)
{
	//创建天空顶点缓冲区
	pd3dDevice->CreateVertexBuffer( sizeof(skyBall),
		0, D3DFVF_SKYVERTEX,
		D3DPOOL_MANAGED, &g_pSkyVB, NULL );
	VOID* pVertices;

	for (int i=0;i<180/angleTurn;i++)
	{
		for (int j=0;j<360/angleTurn;j++)
		{
			if( FAILED( g_pSkyVB->Lock( sizeof(SKYVERTEX4)*(i*360/angleTurn+j), sizeof(SKYVERTEX), (void**)&pVertices, 0 ) ) )
				return E_FAIL;
			float theta=i*angleTurn/180.0f*D3DX_PI-D3DX_PI/2;
			float fi=j*angleTurn/180.0f*D3DX_PI;
			float delta=angleTurn/180.0f*D3DX_PI;

			skyBall[i][j].point[0].x=DISPLAY/2-RADIUS*cos(theta)*cos(fi);
			skyBall[i][j].point[0].z=DISPLAY/2-RADIUS*cos(theta)*sin(fi);
			skyBall[i][j].point[0].y=-15+RADIUS*sin(theta);
			
			skyBall[i][j].point[0].dwColor=0xff00000f;                                                //原为0x0000000；黑白天空  现在想该为蓝色天空

			skyBall[i][j].point[0].u=( 1-cos(theta)*cos(fi) )/2.0f;
			skyBall[i][j].point[0].v=( 1+cos(theta)*sin(fi) )/2.0f;

			skyBall[i][j].point[1].x=DISPLAY/2-RADIUS*cos(theta)*cos(fi+delta);
			skyBall[i][j].point[1].z=DISPLAY/2-RADIUS*cos(theta)*sin(fi+delta);
			skyBall[i][j].point[1].y=-15+RADIUS*sin(theta);
			skyBall[i][j].point[1].dwColor=0xff00000f;//0x00000000;
			skyBall[i][j].point[1].u=( 1-cos(theta)*cos(fi+delta) )/2.0f;
			skyBall[i][j].point[1].v=( 1+cos(theta)*sin(fi+delta) )/2.0f;

			skyBall[i][j].point[2].x=DISPLAY/2-RADIUS*cos(theta+delta)*cos(fi);
			skyBall[i][j].point[2].z=DISPLAY/2-RADIUS*cos(theta+delta)*sin(fi);
			skyBall[i][j].point[2].y=-15+RADIUS*sin(theta+delta);
			skyBall[i][j].point[2].dwColor=0xff00000f;//0x00000000;
			skyBall[i][j].point[2].u=( 1-cos(theta+delta)*cos(fi) )/2.0f;
			skyBall[i][j].point[2].v=( 1+cos(theta+delta)*sin(fi) )/2.0f;

			skyBall[i][j].point[3].x=DISPLAY/2-RADIUS*cos(theta+delta)*cos(fi+delta);
			skyBall[i][j].point[3].z=DISPLAY/2-RADIUS*cos(theta+delta)*sin(fi+delta);
			skyBall[i][j].point[3].y=-15+RADIUS*sin(theta+delta);
			skyBall[i][j].point[3].dwColor=0xff00000f;//0x00000000;
			skyBall[i][j].point[3].u=( 1-cos(theta+delta)*cos(fi+delta) )/2.0f;
			skyBall[i][j].point[3].v=( 1+cos(theta+delta)*sin(fi+delta) )/2.0f;
			memcpy( pVertices, skyBall[i][j].point, sizeof(SKYVERTEX4));
			g_pSkyVB->Unlock();
		}
	}

	//创建地面纹理和云彩纹理
	if( FAILED(  D3DXCreateTextureFromFile( pd3dDevice, L"Texture\\cloud.bmp", &g_pCloudTex ) ) )
		return E_FAIL;

	return S_OK;
}

void SkyRender(IDirect3DDevice9* pd3dDevice, float fElapsedTime)
{
	//设置纹理状态
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	//设置Alpha混合系数
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//移动天空纹理坐标
	SKYVERTEX*  pVertices;
	g_pSkyVB->Lock(0, 0, (void**)&pVertices, 0);
	for(int u=0; u<4*(180/angleTurn)*(360/angleTurn); u++)
	{
		pVertices[u].v -= (0.02f)*fElapsedTime;
	}
	g_pSkyVB->Unlock();

	D3DXMATRIX  g_matSky;   //天空世界矩阵
	
	//构造天空世界矩阵
	D3DXMatrixIdentity(&g_matSky);

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_ADD);
	pd3dDevice->SetTransform( D3DTS_WORLD, &g_matSky );
	pd3dDevice->SetTexture( 0, g_pCloudTex );


	//渲染天空
	for (int i=0;i<180/angleTurn;i++)
	{
		for (int j=0;j<360/angleTurn;j++)
		{
			pd3dDevice->SetRenderState( D3DRS_LIGHTING, false);
			pd3dDevice->SetStreamSource( 0, g_pSkyVB, sizeof(SKYVERTEX4)*(i*360/angleTurn+j), sizeof(SKYVERTEX) );
			pd3dDevice->SetFVF( D3DFVF_SKYVERTEX );
			pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
		}
	}

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,false);
}



void skyClearUp()
{
	SAFE_RELEASE(g_pSkyVB);
	SAFE_RELEASE(g_pCloudTex);
}