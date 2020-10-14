//#include "stdafx.h" //VS2017中需要添加此预编译头文件
#include"HK_camera.h"
#include "singleton.h"
#include <iostream>
#include <map>
#include <vector>
//全局变量
LONG g_nPort;
Mat g_BGRImage;

//数据解码回调函数，
//功能：将YV_12格式的视频数据流转码为可供opencv处理的BGR类型的图片数据，并实时显示。
void CALLBACK DecCBFun(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, long nUser, long nReserved2)
{
	if (pFrameInfo->nType == T_YV12)
	{
		if (g_BGRImage.empty())
		{
			g_BGRImage.create(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
		}
		Mat YUVImage(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, (unsigned char*)pBuf);

		cvtColor(YUVImage, g_BGRImage, COLOR_YUV2BGR_YV12);
		if (g_BGRImage.empty())
		{
			return;
		}
		//imshow("可见光", g_BGRImage);
		//waitKey(15);

		Queue<cv::Mat>* img_queue = Singleton<Queue<cv::Mat>>::instance(MAX_QUEUE);
		cv::Mat img = g_BGRImage;
		if (img_queue->length() < MAX_QUEUE)
		{
			img_queue->push(img);
		}

		YUVImage.~Mat();
	}
}

//实时视频码流数据获取 回调函数
void CALLBACK g_RealDataCallBack_V30(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser)
{
	if (dwDataType == NET_DVR_STREAMDATA)//码流数据
	{
		if (dwBufSize > 0 && g_nPort != -1)
		{
			if (!PlayM4_InputData(g_nPort, pBuffer, dwBufSize))
			{
				std::cout << "fail input data" << std::endl;
			}
			else
			{
				std::cout << "success input data" << std::endl;
			}

		}
	}
}
//构造函数
HK_camera::HK_camera(void)
{

}
//析构函数
HK_camera::~HK_camera(void)
{
}
//初始化函数，用作初始化状态检测
bool HK_camera::Init()
{
	if (NET_DVR_Init())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//登录函数，用作摄像头id以及密码输入登录
bool HK_camera::Login(char* sDeviceAddress, char* sUserName, char* sPassword, WORD wPort)
//bool HK_camera::Login(const char* sDeviceAddress,const char* sUserName,const char* sPassword, WORD wPort);        //登陆（VS2017版本）
{
	NET_DVR_USER_LOGIN_INFO pLoginInfo = { 0 };
	NET_DVR_DEVICEINFO_V40 lpDeviceInfo = { 0 };

	pLoginInfo.bUseAsynLogin = 0;     //同步登录方式
	strcpy_s(pLoginInfo.sDeviceAddress, sDeviceAddress);
	strcpy_s(pLoginInfo.sUserName, sUserName);
	strcpy_s(pLoginInfo.sPassword, sPassword);
	pLoginInfo.wPort = wPort;

	lUserID = NET_DVR_Login_V40(&pLoginInfo, &lpDeviceInfo);

	if (lUserID < 0)
	{
		printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
		cout << "login lUserID:  " << lUserID << " 光电设备(可见光镜头)通讯异常，请检查！" << endl;
		NET_DVR_Cleanup();
		return false;
	}
	else
	{
		return true;
	}
}

//视频流显示函数
void HK_camera::show()
{
	if (PlayM4_GetPort(&g_nPort))            //获取播放库通道号
	{
		if (PlayM4_SetStreamOpenMode(g_nPort, STREAME_REALTIME))      //设置流模式
		{
			if (PlayM4_OpenStream(g_nPort, NULL, 0, 1024 * 1024))         //打开流
			{
				if (PlayM4_SetDecCallBackExMend(g_nPort, DecCBFun, NULL, 0, NULL))
				{
					if (PlayM4_Play(g_nPort, NULL))
					{
						std::cout << "success to set play mode" << std::endl;
					}
					else
					{
						std::cout << "fail to set play mode" << std::endl;
					}
				}
				else
				{
					std::cout << "fail to set dec callback " << std::endl;
				}
			}
			else
			{
				std::cout << "fail to open stream" << std::endl;
			}
		}
		else
		{
			std::cout << "fail to set stream open mode" << std::endl;
		}
	}
	else
	{
		std::cout << "fail to get port" << std::endl;
	}
	//启动预览并设置回调数据流
	NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
	struPlayInfo.hPlayWnd = NULL; //窗口为空，设备SDK不解码只取流
	struPlayInfo.lChannel = 1; //Channel number 设备通道
	struPlayInfo.dwStreamType = 0;// 码流类型，0-主码流，1-子码流，2-码流3，3-码流4, 4-码流5,5-码流6,7-码流7,8-码流8,9-码流9,10-码流10
	struPlayInfo.dwLinkMode = 0;// 0：TCP方式,1：UDP方式,2：多播方式,3 - RTP方式，4-RTP/RTSP,5-RSTP/HTTP 
	struPlayInfo.bBlocked = 1; //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

	if (NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30, NULL))
	{
		//namedWindow("RGBImage");
	}
}
