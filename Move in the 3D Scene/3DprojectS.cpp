//--------------------------------------------------------------------------------------
// File: 3DprojectS.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma warning ( disable: 4244 )


#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"
#include "Head\HeadDef.h"

//#define DEBUG_VS   // Uncomment this line to debug D3D9 vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug D3D9 pixel shaders 

#ifndef FVF1
#define  FVF1
#define D3DFVF_CUSTOMVERTEX   (D3DFVF_XYZ|D3DFVF_TEX1)
#endif



//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

CFirstPersonCamera         g_Camera;
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg            g_SettingsDlg;           // Device settings dialog
CDXUTTextHelper*		   g_pTxtHelper = NULL;
CDXUTDialog                g_HUD;                   // dialog for standard controls
CDXUTDialog                g_SampleUI;              // dialog for sample specific controls

// Direct3D 9 resources
ID3DXFont*                 g_pFont9 = NULL;  
ID3DXSprite*               g_pSpriteForTxt = NULL;
ID3DXSprite*               g_pSprite9 = NULL;      
ID3DXEffect*               g_pEffect9 = NULL; 

CSkinMesh*                 g_pMesh = NULL;          //蒙皮网格模型
D3DXMATRIXA16              g_matWorld;              //世界矩阵

CMyThread                  MyThread;                //线程
LPVOID                     lpParam;

//Control
bool personControl=true;
bool rainControl=false;
bool ifRainStop=true;
//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool    CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnD3D9LostDevice( void* pUserContext );
void    CALLBACK OnD3D9DestroyDevice( void* pUserContext );

void    InitApp();
void    RenderText();


HRESULT InitGriphics( IDirect3DDevice9* pd3dDevice);
VOID SetViewAndProjMatrix( IDirect3DDevice9* pd3dDevice);
void SetLight(IDirect3DDevice9* pd3dDevice);


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"3D交互应用" );
    DXUTCreateDevice( true, 1600, 900 );
    DXUTMainLoop(); // Enter into the DXUT render loop

	MyThread.End();

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 100; 
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"全屏/窗口切换", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"渲染方式切换", 35, iY += 24, 125, 22, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"调整设备（F2）", 35, iY += 24, 125, 22, VK_F2 );
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();

	int wndWidth;
	int wndHeight;
	wndWidth = (int) DXUTGetWindowWidth(); //窗口宽度
	wndHeight= (int) DXUTGetWindowHeight(); //窗口宽度
    WCHAR str[256];

    g_pTxtHelper->Begin();

	g_pTxtHelper->Init(g_pFont9, g_pSpriteForTxt, NULL, NULL, 15);

	//输出三个轴的向量
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f) );
	g_pTxtHelper->SetInsertionPos(100, 468);
	wsprintf(str, L"( %d,  %d,  %d )", (int)(matWorldForCylinder._11 * 10), (int)(matWorldForCylinder._13 * 10), (int)(matWorldForCylinder._12 * 10) );
	g_pTxtHelper->DrawTextLine( str );

	g_pTxtHelper->SetForegroundColor( D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f) );
	g_pTxtHelper->SetInsertionPos(100, 500);
	wsprintf(str, L"( %d,  %d,  %d )", (int)(matWorldForCylinder._31 * 10), (int)(matWorldForCylinder._33 * 10), (int)(matWorldForCylinder._32 * 10) );
	g_pTxtHelper->DrawTextLine( str );

	g_pTxtHelper->SetForegroundColor( D3DXCOLOR(0.3f, 0.3f, 0.96f, 1.0f) );
	g_pTxtHelper->SetInsertionPos(100, 532);
	wsprintf(str, L"( %d,  %d,  %d )", (int)(matWorldForCylinder._23 * 10), (int)(matWorldForCylinder._21 * 10), (int)(matWorldForCylinder._22 * 10) );
    g_pTxtHelper->DrawTextLine( str );

	//输出绕三个轴转的角度
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f) );
	g_pTxtHelper->SetInsertionPos(150, 601);
	wsprintf(str, L"%d", (int)(angle[0]) % 360 );
	g_pTxtHelper->DrawTextLine( str );

	g_pTxtHelper->SetForegroundColor( D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f) );
	g_pTxtHelper->SetInsertionPos(150, 635);
	wsprintf(str, L"%d", (int)(-angle[2]) % 360 );
	g_pTxtHelper->DrawTextLine( str );

	g_pTxtHelper->SetForegroundColor( D3DXCOLOR(0.3f, 0.3f, 0.96f, 1.0f) );
	g_pTxtHelper->SetInsertionPos(150, 669);
	wsprintf(str, L"%d", (int)(angle[1]) % 360 );
	g_pTxtHelper->DrawTextLine( str );

	g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                      D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION(2,0) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    if( pDeviceSettings->ver == DXUT_D3D9_DEVICE )
    {
        IDirect3D9 *pD3D = DXUTGetD3D9Object();
        D3DCAPS9 Caps;
        pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps );

        // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
        // then switch to SWVP.
        if( (Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
            Caps.VertexShaderVersion < D3DVS_VERSION(1,1) )
        {
            pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // Debugging vertex shaders requires either REF or software vertex processing 
        // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
        if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
        {
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
            pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
#endif
#ifdef DEBUG_PS
        pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    }

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( (DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF) ||
            (DXUT_D3D10_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d10.DriverType == D3D10_DRIVER_TYPE_REFERENCE) )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
	InitGriphics(pd3dDevice);

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
    
    V_RETURN( D3DXCreateFont( pd3dDevice, 16, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                              L"Arial", &g_pFont9 ) );

	//创建网格模型
	g_pMesh = new CSkinMesh();
	V_RETURN(g_pMesh->OnCreate(pd3dDevice, L"Media\\boy.X"));    //载入模型！

    // Read the D3DX effect file
    WCHAR str[MAX_PATH];
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"3DprojectS.fx" ) );
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect9, NULL ) );

    //设置观察矩阵 —— HW
    D3DXVECTOR3 vecEye(10.0f, 0.6f, 1.0f);//观察点
    D3DXVECTOR3 vecAt (10.0f, 0.6f, 2.0f);//视线落点
    g_Camera.SetViewParams( &vecEye, &vecAt );

	pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);//不剔除任何表面
	pd3dDevice->SetRenderState(D3DRS_ZENABLE,true);//采用深度测试
	initSky(pd3dDevice);

	//线程
	MyThread.Start(lpParam);
	MyThread.Run();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, 
                                    const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont9 ) V_RETURN( g_pFont9->OnResetDevice() );
    if( g_pEffect9 ) V_RETURN( g_pEffect9->OnResetDevice() );

    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pSprite9 ) );
    g_pTxtHelper = new CDXUTTextHelper( g_pFont9, g_pSprite9, NULL, NULL, 15 );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-170, pBackBufferSurfaceDesc->Height-350 );
    g_SampleUI.SetSize( 170, 300 );

	//构造模型的世界矩阵 —— HW
    D3DXMATRIXA16  g_matTrans, g_matScal, g_matRota1, g_matRota2;
	
	D3DXMatrixIdentity( &g_matWorld );
	D3DXMatrixIdentity( &g_matScal);
	D3DXMatrixIdentity( &g_matRota1);
	D3DXMatrixIdentity( &g_matRota2);
	D3DXMatrixIdentity( &g_matTrans);
	
	//初始化模型大小位置 —— HW
	D3DXMatrixScaling( &g_matScal, 0.5f, 0.5f, 0.5f);//放缩
	D3DXMatrixTranslation( &g_matTrans, 10.0f, 0.0f, 5.0f);//平移
    
	g_matWorld *= g_matScal * g_matRota1 * g_matRota2 * g_matTrans;

	pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);//不剔除任何表面
	pd3dDevice->SetRenderState(D3DRS_ZENABLE,true);//采用深度测试

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input

    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mWorldViewProjection;
    
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }
	
	float timeSize;
	timeSize=calculateColorForSky();
	
	//设置灯光
	SetLight(pd3dDevice);

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, (int)(0), (int)(0), (int)(Skylu)), 1.0f, 0) );
	
	
    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		// Render the scene
		{

			//设置投影及观察矩阵
			pd3dDevice->SetTransform(D3DTS_VIEW,g_Camera.GetViewMatrix());
			pd3dDevice->SetTransform(D3DTS_PROJECTION,g_Camera.GetProjMatrix());

			
//			RenderText();//渲染文字
			
			SkyRender( pd3dDevice, fElapsedTime);//渲染天空
			
			GroundPaint(pd3dDevice);//渲染地面

			MeshPaint( pd3dDevice);//开始渲染静态地图模型
			
			CylinderPaint(pd3dDevice);//渲染小坐标
			
			g_pMesh->Render(&g_matWorld, fElapsedTime);//渲染模型
			
//			InfoBackPaint(pd3dDevice, g_pSprite9);//渲染信息背景

			//结束渲染
		}

        // Get the projection & view matrix from the camera class
        mWorld = *g_Camera.GetWorldMatrix();
        mProj = *g_Camera.GetProjMatrix();
        mView = *g_Camera.GetViewMatrix();

        mWorldViewProjection = mWorld * mView * mProj;

        // Update the effect's variables.  Instead of using strings, it would 
        // be more efficient to cache a handle to the parameter by calling 
        // ID3DXEffect::GetParameterByName
        V( g_pEffect9->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        V( g_pEffect9->SetMatrix( "g_mWorld", &mWorld ) );
        V( g_pEffect9->SetFloat( "g_fTime", (float)fTime ) );

        DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" ); // These events are to help PIX identify what the code is doing
//        RenderText();
        V( g_HUD.OnRender( fElapsedTime ) );
        V( g_SampleUI.OnRender( fElapsedTime ) );
        DXUT_EndPerfEvent();

        V( pd3dDevice->EndScene() );
    }

}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;


	//初始的摄像机位置和观察点,与OnD3D9CreateDevice函数中保持一致 —— HW
	D3DXVECTOR3 vecEye(10.0f, 0.6f, 1.0f);
    D3DXVECTOR3 vecAt(10.0f, 0.6f, 2.0f);
	
	// Pass all remaining windows messages to camera so it can respond to user input
	if(personControl) g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	switch( uMsg )
	{
	case WM_KEYDOWN:
         if (wParam == 13) g_Camera.SetViewParams( &vecEye, &vecAt );//回车键，观察点复原
		 m_bKey[wParam] = 1;
		 return 0;

	case WM_KEYUP:
		m_bKey[wParam] = 0;
		return 0;
	}
    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
    }
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont9 ) g_pFont9->OnLostDevice();
    if( g_pEffect9 ) g_pEffect9->OnLostDevice();
    SAFE_RELEASE( g_pSprite9 );
    SAFE_DELETE( g_pTxtHelper );
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{	
	GroundClearUp();//释放地面
     
//	CylinderClearUp();//释放小坐标
	
//	InfoBackClearUp();//释放信息背景	
	
	skyClearUp();//释放天空
	
	MeshClearUp();//释放静态模型空间

	g_pMesh->OnDestory();


    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE( g_pEffect9 );
    SAFE_RELEASE( g_pFont9 );

	
}


//-----------------------------------------------------------------------------
// Desc: 创建场景图形
//-----------------------------------------------------------------------------
HRESULT InitGriphics( IDirect3DDevice9* pd3dDevice)
{

	//载入地面
	if ( FAILED(Groundld(pd3dDevice)) )
	{
		MessageBox(NULL, L"载入地面失败", L"Ground", MB_OK);
		return E_FAIL;
	}
/*
	if ( FAILED(Cylinderld(pd3dDevice)) )
	{
		MessageBox(NULL, L"创建小坐标失败", L"CylinderVertex", MB_OK);
		return E_FAIL;
	}

	if ( FAILED(InfoBackld(pd3dDevice)) )
	{
		MessageBox(NULL, L"信息背景失败", L"InfoBackTex", MB_OK);
		return E_FAIL;
	}
*/
	//载入静态场景模型
	if( FAILED(Meshld(pd3dDevice)) )
	{
		MessageBox(NULL, L"创建静态场景模型失败", L"SMesh", MB_OK);
		return E_FAIL;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Desc: 设置变换矩阵
//-----------------------------------------------------------------------------
VOID SetViewAndProjMatrix( IDirect3DDevice9* pd3dDevice)
{
	D3DXVECTOR3 vEyePt( DISPLAY/2, DISPLAY/2,-20.0f  );
	D3DXVECTOR3 vLookatPt( DISPLAY/2, DISPLAY/2,0.0f  );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	//创建并设置投影矩阵
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 10000.0f );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}
