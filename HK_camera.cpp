//#include "stdafx.h" //VS2017����Ҫ��Ӵ�Ԥ����ͷ�ļ�
#include"HK_camera.h"
#include "singleton.h"
#include <iostream>
#include <map>
#include <vector>
//ȫ�ֱ���
LONG g_nPort;
Mat g_BGRImage;

//���ݽ���ص�������
//���ܣ���YV_12��ʽ����Ƶ������ת��Ϊ�ɹ�opencv�����BGR���͵�ͼƬ���ݣ���ʵʱ��ʾ��
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
		//imshow("�ɼ���", g_BGRImage);
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

//ʵʱ��Ƶ�������ݻ�ȡ �ص�����
void CALLBACK g_RealDataCallBack_V30(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser)
{
	if (dwDataType == NET_DVR_STREAMDATA)//��������
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
//���캯��
HK_camera::HK_camera(void)
{

}
//��������
HK_camera::~HK_camera(void)
{
}
//��ʼ��������������ʼ��״̬���
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

//��¼��������������ͷid�Լ����������¼
bool HK_camera::Login(char* sDeviceAddress, char* sUserName, char* sPassword, WORD wPort)
//bool HK_camera::Login(const char* sDeviceAddress,const char* sUserName,const char* sPassword, WORD wPort);        //��½��VS2017�汾��
{
	NET_DVR_USER_LOGIN_INFO pLoginInfo = { 0 };
	NET_DVR_DEVICEINFO_V40 lpDeviceInfo = { 0 };

	pLoginInfo.bUseAsynLogin = 0;     //ͬ����¼��ʽ
	strcpy_s(pLoginInfo.sDeviceAddress, sDeviceAddress);
	strcpy_s(pLoginInfo.sUserName, sUserName);
	strcpy_s(pLoginInfo.sPassword, sPassword);
	pLoginInfo.wPort = wPort;

	lUserID = NET_DVR_Login_V40(&pLoginInfo, &lpDeviceInfo);

	if (lUserID < 0)
	{
		printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
		cout << "login lUserID:  " << lUserID << " ����豸(�ɼ��⾵ͷ)ͨѶ�쳣�����飡" << endl;
		NET_DVR_Cleanup();
		return false;
	}
	else
	{
		return true;
	}
}

//��Ƶ����ʾ����
void HK_camera::show()
{
	if (PlayM4_GetPort(&g_nPort))            //��ȡ���ſ�ͨ����
	{
		if (PlayM4_SetStreamOpenMode(g_nPort, STREAME_REALTIME))      //������ģʽ
		{
			if (PlayM4_OpenStream(g_nPort, NULL, 0, 1024 * 1024))         //����
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
	//����Ԥ�������ûص�������
	NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
	struPlayInfo.hPlayWnd = NULL; //����Ϊ�գ��豸SDK������ֻȡ��
	struPlayInfo.lChannel = 1; //Channel number �豸ͨ��
	struPlayInfo.dwStreamType = 0;// �������ͣ�0-��������1-��������2-����3��3-����4, 4-����5,5-����6,7-����7,8-����8,9-����9,10-����10
	struPlayInfo.dwLinkMode = 0;// 0��TCP��ʽ,1��UDP��ʽ,2���ಥ��ʽ,3 - RTP��ʽ��4-RTP/RTSP,5-RSTP/HTTP 
	struPlayInfo.bBlocked = 1; //0-������ȡ��, 1-����ȡ��, �������SDK�ڲ�connectʧ�ܽ�����5s�ĳ�ʱ���ܹ�����,���ʺ�����ѯȡ������.

	if (NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, g_RealDataCallBack_V30, NULL))
	{
		//namedWindow("RGBImage");
	}
}
