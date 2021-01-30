#pragma once

#include "DXUT.h"
#include <windows.h>
#include "com_pc.h"

#include <fstream>
using namespace std;

ofstream out("out.txt", ios_base::out);

// MyThread.h

#ifndef MY_THREAD_H
#define MY_THREAD_H

class CMyThread
{
public:
    CMyThread();
    virtual ~CMyThread();

    BOOL Start(LPVOID lpParam);
    BOOL End();
    virtual void Run();

protected:
    static DWORD WINAPI Thread(LPVOID lpParam);
    void RunOnceEnd();

    DWORD m_dwWaitTimeOut;
    BOOL m_bExitThread;
    HANDLE m_hTrd;
    LPVOID m_lpParam;
};

#endif

// MyThread.Cpp
// CMyThread
CMyThread::CMyThread()
{
    m_bExitThread = FALSE;
    m_hTrd = NULL;
    m_dwWaitTimeOut = 5000;
}

CMyThread::~CMyThread()
{

}

BOOL CMyThread::Start(LPVOID lpParam)
{
    m_lpParam = lpParam;
    m_bExitThread = FALSE;
    m_hTrd = CreateThread(NULL, 0, Thread, this, 0, NULL);

    return TRUE;
}

BOOL CMyThread::End()
{
    m_bExitThread = TRUE;

    if(m_hTrd != NULL)
    {
        DWORD dwRet = WaitForSingleObject(m_hTrd, m_dwWaitTimeOut);
        if(dwRet == WAIT_OBJECT_0)
        {
        }
        else
        {
            DWORD dwRet = 0;
            GetExitCodeThread(m_hTrd, &dwRet);
            TerminateThread(m_hTrd, dwRet);
        }

        CloseHandle(m_hTrd);
        m_hTrd = NULL;
	}
	
	return TRUE;
}

DWORD WINAPI CMyThread::Thread(LPVOID lpParam)
{
    CMyThread *pTrd = (CMyThread *)lpParam;
 
    while(!pTrd->m_bExitThread)
    {
       pTrd->Run();
    }

    return 0;
}

void CMyThread::RunOnceEnd()
{
    m_bExitThread = TRUE;
    CloseHandle(m_hTrd);
    m_hTrd = NULL;
}

void CMyThread::Run()
{
	/*D3DXVECTOR3 tempX, tempY, tempZ;

	readcom2();

	tempX = AxisX;
	tempY = AxisY;
	tempZ = AxisZ;

    AxisX = tempX - (angle[1] * tempZ - angle[2] * tempY) * D3DX_PI / 180;
	AxisY = tempY - (angle[2] * tempX - angle[0] * tempZ) * D3DX_PI / 180;
	AxisZ = tempZ - (angle[0] * tempY - angle[1] * tempX) * D3DX_PI / 180;

	out<<angle[0]<<" "<<angle[1]<<" "<<angle[2]<<endl;
	out<<AxisX.x<<" "<<AxisX.y<<" "<<AxisX.z<<endl;
	out<<AxisY.x<<" "<<AxisY.y<<" "<<AxisY.z<<endl;
	out<<AxisZ.x<<" "<<AxisZ.y<<" "<<AxisZ.z<<endl;
	out<<"1"<<endl;

	D3DXVec3Normalize(&AxisX, &AxisX);
	D3DXVec3Cross(&AxisY, &AxisZ, &AxisX);
	D3DXVec3Normalize(&AxisY, &AxisY);
	D3DXVec3Cross(&AxisZ, &AxisX, &AxisY);
	D3DXVec3Normalize(&AxisZ, &AxisZ);*/

	for(int i=0;i<3;i++)
	{
		sum[i] += 0.1;
		if ( sum[i] > 50 ) sum[i] = sum[i] - 50;
	}
	Sleep(20);
}