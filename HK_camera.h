#ifndef _HK_CAMERA_H_ 
#define _HK_CAMERA_H_

#include<HCSDK\HCNetSDK.h>
#include<HCSDK\plaympeg4.h>
#include<HCSDK\PlayM4.h>    //
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <queue>

using namespace cv;
using namespace std;

#define MAX_QUEUE 500 //最大队列空间
class HK_camera
{
public:
	HK_camera(void);
	~HK_camera(void);

public:
	bool Init();                  //初始化
	bool Login(char* sDeviceAddress, char* sUserName, char* sPassword, WORD wPort);            //登陆
																							   //bool Login(const char* sDeviceAddress,const char* sUserName,const char* sPassword, WORD wPort);            //登陆（VS2017版本）
	void show();                  //显示图像
private:
	LONG lUserID;

};
#endif;

