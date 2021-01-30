#include "DXUT.h"
#include <windows.h>
#include <winbase.h>
#include <omp.h>

HANDLE hCom;  //全局变量，串口句柄

class Comm
{
public:
	Comm(LPCTSTR Com_Num=L"COM7",int Com_Rate=230400,int rwBuffer=1024);
	int read(char *str,DWORD len);
	int send(char *lpOutBuffer,DWORD len);
	HANDLE hCom;
	~Comm();
};

Comm::Comm(LPCTSTR Com_Num,int Com_Rate,int rwBuffer)
{
	// TODO: Add extra initialization here
	hCom=CreateFile(Com_Num,//COM1口
		GENERIC_READ|GENERIC_WRITE, //允许读和写
		0, //独占方式
		NULL,
		OPEN_EXISTING, //打开而不是创建
		0, //同步方式
		NULL);
	/*if(hCom==(HANDLE)-1)
	{
	AfxMessageBox("打开COM失败!");
	return FALSE;
	}*/

	SetupComm(hCom,rwBuffer,rwBuffer); //输入缓冲区和输出缓冲区的大小都是 rwBuffer(default=1024)

	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout=MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier=0;
	TimeOuts.ReadTotalTimeoutConstant=0;
	//在读一次输入缓冲区的内容后读操作就立即返回，
	//而不管是否读入了要求的字符。


	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier=100;
	TimeOuts.WriteTotalTimeoutConstant=500;
	SetCommTimeouts(hCom,&TimeOuts); //设置超时

	DCB dcb;
	GetCommState(hCom,&dcb);

	dcb.BaudRate=Com_Rate; //波特率为
	dcb.ByteSize=8; //每个字节有8位
	dcb.Parity=NOPARITY; //无奇偶校验位
	//dcb.StopBits=TWOSTOPBITS; //两个停止位
	dcb.StopBits=ONESTOPBIT; //一个停止位
	dcb.wReserved=0;
	dcb.fBinary = TRUE;

	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	SetCommState(hCom,&dcb);

	//PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);

}


Comm::~Comm()
{
		CloseHandle(hCom);
}


int Comm:: read(char *str,DWORD len)
{
	memset(str,'\0',len+1);
	//DWORD wCount=100;//读取的字节数
	BOOL bReadStat;
	bReadStat=ReadFile(hCom,str,len,&len,NULL);
	if(!bReadStat)
		return 0;
	//PurgeComm(hCom, PURGE_TXABORT|
	//	PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	return len;
}


int Comm::send(char *lpOutBuffer,DWORD len)
{
	DWORD dwBytesWrite=7;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;

	ClearCommError(hCom,&dwErrorFlags,&ComStat);
	bWriteStat=WriteFile(hCom,lpOutBuffer,len,&len,NULL);
	if(!bWriteStat)
	{
		return 0;
	}

	return len;
}


Comm Com(L"COM3");
static double angle[3],sum[3] = {1.0, 1.0, 1.0};
static D3DXVECTOR3 AxisX(1.0f, 0.0f, 0.0f), AxisY(0.0f, 1.0f, 0.0f), AxisZ(0.0f, 0.0f, 1.0f);
const double angle_static[3]={-5.48561554/1812-0.000630,-7.948734359/1812-0.000675,-2.462734808/1812-0.00052};
int k=0;

void find55()
{
	char former_c,c;
	int L;
	while(1)
	{
		L=Com.read(&c,1);
		if(L)
		{
			if(former_c=0x55&&c==0x55) break;
			former_c=c;
		}
	}
}
void readcom1()
{
	static char c=0,former_c=0,str[40],buf[100];
	static int num=0,i,j;
	static short int *	shortstr=(short int *)(str);
	int L=Com.read(buf,20);
	memcpy(str+num,buf,L);
	num=num+L;
	if(num>=20)
	{
		if(str[18]==0x55&&str[19]==0x55)
		{
			for(i=0;i<3;i++)
			{
				angle[i]=double(shortstr[i+3])/1812-angle_static[i];
				sum[i]+=angle[i];
			}
			num=num-20;
			memcpy(str,str+20,num);
			k++;
		}
		else
		{
			find55();
			num=0;
			for(i=0;i<3;i++)
			{
				sum[i]+=angle[i];
			}
		}
	}
}

void readcom2()
{
	static char c=0,former_c=0,str[40],buf[100];
	static int num=0,i,j;
	static short int *	shortstr=(short int *)(str);
	int L=Com.read(buf,20);
	for(j=0;j<L;j++)
	{
		c=buf[j];
		str[num]=c;
		num++;
		if(former_c==0x55&&c==0x55&&num>=19)
		{
			num=0;
			//#pragma omp parallel for
			for(i=0;i<3;i++)
			{
				angle[i]=double(shortstr[i+3])/1812-angle_static[i];
				if (angle[i]<0.01 && angle[i]>-0.01) angle[i]=0;
				sum[i]+=angle[i];
			}
			k++;
		}
		former_c=c;
	}
}

void readcom3()
{
	static char c=0,former_c=0,str[20];
	static int len=0,num=0,cnt=0,i=0;
	static short int *shortstr=(short int *)str;
	while(1)
	{
		if(Com.read(&c,1))
		{
			str[num]=c;
			num++;
			if(former_c==0x55&&c==0x55&&num>=19)
			{
				num=0;
				cnt++;
				for(i=0;i<3;i++) angle[i]=double(shortstr[i+3])/1812-angle_static[i];
				for(i=0;i<3;i++) sum[i]+=angle[i];
				k++;
				break;
			}
			former_c=c;
		}
	}
}