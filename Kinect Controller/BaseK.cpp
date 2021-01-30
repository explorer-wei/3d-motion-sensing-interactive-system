#include <iostream>
#include <Windows.h>
#include <mmsystem.h>
#include "NuiApi.h"
using namespace std;

const int sample = 4;//采样点数（延时约为sample/30秒）
const int N = 2;//行数
const int M = 3;//列数
int key1[N][M] = {{49,50,51},{52,53,54}};//123456是播放
int key2[N][M] = {{55,56,57},{48,90,88}};//7890zx是停止

int main()
{
	INuiSensor*	m_pNuiSensor = NULL;

	//初始化NUI
	HRESULT hr;

	if ( !m_pNuiSensor )
    {
        hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);
		if ( FAILED(hr) )
		{
			return hr;
			cout<<"CreateSensor failed"<<endl;
		}
		else
			cout<<"CreateSensor succeed"<<endl;
	}

	HANDLE m_hNextSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	//初始化NUI
	hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
	if( hr != S_OK )
	{
		cout<<"NuiInitialize failed"<<endl;
		return hr;
	}
	else
		cout<<"NuiInitialize succeed"<<endl;
	
	//打开骨骼追踪
	hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
	if( hr != S_OK )
	{
		cout<<"SkeletonTracking failed"<<endl;
		return hr;
	}
	else
		cout<<"SkeletonTracking succeed"<<endl;

	//获得骨骼帧
	int num = 0;//记录当前帧保存的位置
	POINT rh[sample],lh[sample],head;//Long型
	USHORT rhz,lhz,headz;//高13位是深度，毫米为单位
	float gridW, gridH;//按键宽高
	int nR,mR,nL,mL;//打击点所处方格
//	FILE *	file;//save log
//	file = fopen("kinect.log.txt","w");
	int i = 0;
	int tagR = 1;//1为右手缩回，0为右手伸出
	int tagL = 1;//1为左手缩回，0为左手伸出
	int tagB = 1;//0为双手伸出
	int tagControl = -1;//0、4为PPT，1、3为鼠标，2为电子乐队
	int tagC = 0;//1为标定
	int distance = 400;//触发距离
	int high=50,low=250,left=250,right=50;//初始化手势范围
	int ID = 0;//记录当前用户的ID号
	DWORD lastCalTime,CalTime;//标定延时
	//空中鼠标
	int Rx,Ry;
	INPUT MyMouseInput;

	while(1)
	{
		NUI_SKELETON_FRAME SkeletonFrame = {0};

		bool bFoundSkeleton = false;

		if ( SUCCEEDED(m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &SkeletonFrame )) ) //间隔多少ms取下一帧
		{
			for (i = 0; i < NUI_SKELETON_COUNT; i++) 
			{
				if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ||
					(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY))
				{
					bFoundSkeleton = true;
					ID = i;
				}
			}
		}

		// smooth out the skeleton data
		HRESULT hr = m_pNuiSensor->NuiTransformSmooth(&SkeletonFrame,NULL);
		if ( FAILED(hr) )
		{
			cout<<"Smoothing failed"<<endl;
			return hr;
		}

		// Show skeleton only if it is tracked, and the center-shoulder joint is at least inferred.
		if ( SkeletonFrame.SkeletonData[ID].eTrackingState == NUI_SKELETON_TRACKED &&
			SkeletonFrame.SkeletonData[ID].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
		{
			NuiTransformSkeletonToDepthImage(SkeletonFrame.SkeletonData[ID].SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT], &rh[num].x, &rh[num].y, &rhz);
			NuiTransformSkeletonToDepthImage(SkeletonFrame.SkeletonData[ID].SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT], &lh[num].x, &lh[num].y, &lhz);
			NuiTransformSkeletonToDepthImage(SkeletonFrame.SkeletonData[ID].SkeletonPositions[NUI_SKELETON_POSITION_HEAD], &head.x, &head.y, &headz);
			headz = headz>>3;
			rhz = rhz>>3;
			lhz = lhz>>3;
			int d = (lh[num].x-rh[num].x)*(lh[num].x-rh[num].x)+(lh[num].y-rh[num].y)*(lh[num].y-rh[num].y);//左右手距离

			//双手伸出，鼠标受控
			if(d<120)
			{
				if( (headz-rhz)>distance && (headz-lhz)>distance && tagB)
				{
					tagB = 0;
					if(tagControl == -1)//准备PPT
					{
						tagControl = 0;
						keybd_event(VK_F5,0,0,0);
						keybd_event(VK_F5,0,KEYEVENTF_KEYUP,0);					
						cout<<"Run PPT [ "<<tagControl<<" ]"<<endl;
						tagC = 1;
						CalTime = timeGetTime( );
						lastCalTime = CalTime;
					}
					else if(tagControl == 0)//准备空中鼠标
					{
						tagControl = 1;
						keybd_event(VK_ESCAPE,0,0,0);
						keybd_event(VK_ESCAPE,0,KEYEVENTF_KEYUP,0);
						cout<<"Calibrate Mouse[ "<<tagControl<<" ]"<<endl;
						tagC = 1;
						high=50,low=250,left=250,right=50;
						CalTime = timeGetTime( );
						lastCalTime = CalTime;
					}
					else if(tagControl == 1)//准备虚拟按键
					{
						tagControl = 2;	
						cout<<"Calibrate Key[ "<<tagControl<<" ]"<<endl;
						tagC = 1;
						high=50,low=250,left=250,right=50;
						CalTime = timeGetTime( );
						lastCalTime = CalTime;
					}
					else if(tagControl == 2)//准备鼠标
					{
						tagControl = 3;
						keybd_event(VK_ESCAPE,0,0,0);
						keybd_event(VK_ESCAPE,0,KEYEVENTF_KEYUP,0);
						cout<<"Calibrate Mouse Again[ "<<tagControl<<" ]"<<endl;
						tagC = 1;
						high=50,low=250,left=250,right=50;
						CalTime = timeGetTime( );
						lastCalTime = CalTime;
					}
					else if(tagControl == 3)//准备重新接入PPT
					{
						tagControl = 4;
						keybd_event(VK_SHIFT,0,0,0);
						keybd_event(VK_F5,0,0,0);						
						keybd_event(VK_F5,0,KEYEVENTF_KEYUP,0);
						keybd_event(VK_SHIFT,0,KEYEVENTF_KEYUP,0);
						cout<<"Restart PPT [ "<<tagControl<<" ]"<<endl;
						tagC = 1;
						CalTime = timeGetTime( );
						lastCalTime = CalTime;
					}
					else
					{
						tagControl = 4;
						tagC = 0;
						keybd_event(VK_F5,0,0,0);
						keybd_event(VK_F5,0,KEYEVENTF_KEYUP,0);					
						cout<<"F5 [ "<<tagControl<<" ]"<<endl;
					}
				}
			}
			else
			{			
				if(tagC == 1)
				{
					if((CalTime-lastCalTime)<4000)//标定状态,4秒标定时间
					{
						//标定校准,按左上、右上、右下、左下的顺序
						//手势与光标的映射
						high = high > rh[num].y ? high : rh[num].y;
						low = low < rh[num].y ? low : rh[num].y;
						left = left < lh[num].x ? left : lh[num].x;
						right = right > rh[num].x ? right : rh[num].x;
						CalTime = timeGetTime( );
					}
					else
					{
						tagC = 0;
						gridW = (float)(right-left)/M;
						gridH = (float)(high-low)/N;
	//					fprintf(file,"Calibrate HIGH:%d,LOW:%d,LEFT:%d,RIGHT:%d\n",high,low,left,right);
						cout<<endl<<"Calibrate HIGH:"<<high<<" LOW:"<<low<<" LEFT:"<<left<<" RIGHT:"<<right<<endl;
						cout<<"gridW = "<<gridW<<", gridH = "<<gridH<<endl<<endl;
					}
				}

				//PPT
				if((tagControl==0 || tagControl==4) && abs(rhz-lhz)>150 && tagC!=1)
				{
					//右手伸出,→键按下
					if((headz-rhz)>distance && tagR)
					{
						tagR = 0;
						keybd_event(VK_RIGHT,0,0,0);
						keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);					
	//					fprintf(file,"Right [%d]\n",headz-rhz);
						cout<<"Right [ "<<headz-rhz<<" ]"<<endl;					
					}
					else if((headz-rhz)<distance-50)//5cm误差隔离区
					{
						tagR = 1;
					}
					//左手伸出,←键按下
					if((headz-lhz)>distance && tagL)
					{
						tagL = 0;
						keybd_event(VK_LEFT,0,0,0);
						keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);					
	//					fprintf(file,"Left [%d]\n",headz-lhz);
						cout<<"Left [ "<<headz-lhz<<" ]"<<endl;					
					}
					else if((headz-lhz)<distance-50)//5cm误差隔离区
					{
						tagL = 1;
					}
				
					if(tagL == 1 && tagR == 1)
						tagB = 1;
				}

				//鼠标
				if((tagControl==1 || tagControl==3) && tagC!=1)
				{
					tagB = 1;
					//滤波 - 求sample个点均值
					LONG tempx = 0, tempy = 0;
					for(i=0; i<sample; i++)
					{
						tempx = tempx + rh[i].x;
						tempy = tempy + rh[i].y;
					}
					tempx = tempx/sample;
					tempy = tempy/sample;

					Rx = (65535.0f/(float)(right-left))*(tempx-left);
					Ry = (65535.0f/(float)(high-low))*(tempy-low);
					Rx = Rx < 65535 ? Rx : 65000;
					Rx = Rx > 0 ? Rx : 500;
					Ry = Ry < 65535 ? Ry : 65000;
					Ry = Ry > 0 ? Ry : 500;

					if(abs(headz-rhz)<(distance-50))//5cm误差隔离区
					{
						tagR = 1;
						MyMouseInput.type = INPUT_MOUSE;
						MyMouseInput.mi.dx = Rx;
						MyMouseInput.mi.dy = Ry;
						MyMouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP|MOUSEEVENTF_MOVE;
						MyMouseInput.mi.mouseData = NULL;
						MyMouseInput.mi.dwExtraInfo = NULL;
						MyMouseInput.mi.time = NULL;
						SendInput(1,&MyMouseInput,sizeof(INPUT));
	//					fprintf(file,"%d %d\n",tempx,tempy);
						cout<<"Move [ "<<Rx<<", "<<Ry<<" ]"<<endl;							
					}
					else if(abs(headz-rhz)>distance && tagR == 1)
					{
						tagR = 0;
						// left down
						MyMouseInput.type = INPUT_MOUSE;
						MyMouseInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
						SendInput(1,&MyMouseInput,sizeof(INPUT));
						MyMouseInput.mi.mouseData = NULL;
						MyMouseInput.mi.dwExtraInfo = NULL;
						MyMouseInput.mi.time = NULL;
						// left up
						ZeroMemory(&MyMouseInput,sizeof(INPUT));
						MyMouseInput.type = INPUT_MOUSE;
						MyMouseInput.mi.dwFlags  = MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP;
						SendInput(1,&MyMouseInput,sizeof(INPUT));

	//					fprintf(file,"%d %d\n",tempx,tempy);
						cout<<"Click [ "<<Rx<<", "<<Ry<<" ]"<<endl;	
					}
				}

				//按键
				if(tagControl==2 && tagC!=1)
				{
					rh[num].y = rh[num].y > high ? high : rh[num].y;
					rh[num].y = rh[num].y < low ? low : rh[num].y;
					rh[num].x = rh[num].x > right ? right : rh[num].x;
					lh[num].y = lh[num].y > high ? high : lh[num].y;
					lh[num].y = lh[num].y < low ? low : lh[num].y;
					lh[num].x = lh[num].x < left ? left : lh[num].x;
					nR = (rh[num].y-low)/gridH;
					mR = (rh[num].x-left)/gridW;
					nL = (lh[num].y-low)/gridH;
					mL = (lh[num].x-left)/gridW;

					//右手
					if((headz-rhz)>distance && tagR)
					{
						tagR = 0;
						keybd_event(key1[nR][mR],0,0,0);//1
						keybd_event(key1[nR][mR],0,KEYEVENTF_KEYUP,0);					
	//					fprintf(file,"Right [%d]\n",headz-rhz);
						cout<<"Right On, [ "<<mR<<", "<<nR<<" ]"<<endl;					
					}
					else if((headz-rhz)<distance-50 && !tagR)//5cm误差隔离区
					{
						tagB = 1;
						tagR = 1;
						keybd_event(key2[nR][mR],0,0,0);//7
						keybd_event(key2[nR][mR],0,KEYEVENTF_KEYUP,0);
						cout<<"Right Off, [ "<<mR<<", "<<nR<<" ]"<<endl;
					}
					//左手
					if((headz-lhz)>distance && tagL)
					{
						tagL = 0;
						keybd_event(key1[nL][mL],0,0,0);//2
						keybd_event(key1[nL][mL],0,KEYEVENTF_KEYUP,0);					
	//					fprintf(file,"Right [%d]\n",headz-rhz);
						cout<<"Left On, [ "<<mL<<", "<<nL<<" ]"<<endl;					
					}
					else if((headz-lhz)<distance-50 && !tagL)//5cm误差隔离区
					{
						tagB = 1;
						tagL = 1;
						keybd_event(key2[nL][mL],0,0,0);//8
						keybd_event(key2[nL][mL],0,KEYEVENTF_KEYUP,0);
						cout<<"Left Off, [ "<<mL<<", "<<nL<<" ]"<<endl;
					}
				}
			}			
			num = (num+1)%sample;
		}
	}

	//关闭NUI链接
	NuiShutdown();
//	fclose(file);
	return 0;
}
