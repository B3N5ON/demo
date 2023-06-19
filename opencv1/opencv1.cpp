
// opencv1.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#pragma comment(lib, "opencv_world470d.lib")

using namespace cv;
using namespace std;
int g_nTrackerbarValue = 0 ;
int g_nKernelValue ;
const int g_ntrakerMaxValue = 9;
Mat frame, gray, bg, dst,result;
char title[30];
bool averBlur = 0;

Mat draw(Mat bg) {
	int bins = 256;
	int hist_size[] = { bins };
	float range[] = { 0,256 };
	MatND hist;
	const float* ranges[] = { range };
	int channels[] = { 0 };
	calcHist(&bg, 1, channels, Mat(), hist, 1, hist_size, ranges, true, false);
	double maxVal;
	minMaxLoc(hist, 0, &maxVal, 0, 0);
	int scale = 2;
	int hist_height = 256;
	Mat hist_img = Mat::zeros(hist_height, bins * scale, CV_8UC3);
	for (int i = 0; i < bins; i++) {
		float binVal = hist.at<float>(i);
		int intensity = cvRound(binVal / maxVal * hist_height);
		rectangle(hist_img, Point(i * scale, hist_height - 1),
			Point((i + 1) * scale - 1, hist_height - intensity), CV_RGB(255, 255, 255));
	}
	return hist_img;
}

void on_nkernelTracker(int, void*)
{
	g_nKernelValue = g_nTrackerbarValue * 2 + 1;
	blur(dst, result, Size(g_nKernelValue, g_nKernelValue));
	imshow(title, result);
	//imshow("blur", dst);
	//averBlur = 0;
}

void play_video(const char* filename)
{
	VideoCapture capture(filename);
	if (!capture.isOpened())
	{
		printf("open %s failed\n", filename, CAP_ANY);
		return;
	}
	long totalFrameNum = capture.get(CAP_PROP_FRAME_COUNT);
	printf("total frames = %d\n", totalFrameNum);
	sprintf_s(title, "video:%s", filename);
	bool isgray = 0;			//判斷是否為灰階
	bool BgSb = 0;		//判斷是否要去背
	bool isneg = 0;				//判斷是否為負片
	bool bThre = 0;			
	bool Hist = 0;
	char key = 0;
	char kernelName[20];

	while (capture.isOpened()  && key != 'q')
	{
		key = waitKey(20);
		capture.read(frame);
		if (frame.empty())
		{
			cerr << "End of video file" << endl;
			break;
		}
		if (key == 'g')
		{
			isgray = 1;
			isneg = 0;
			BgSb = 0;
			bThre = 0;
			Hist = 0;
		}
		else if (key == 'i')
		{
			isneg = 1;
			isgray = 0;
			BgSb = 0;
			bThre = 0;
			Hist = 0;
		}
		else if (key == 'b')
		{
			isgray = 0;
			isneg = 0;
			BgSb = 1;
			bThre = 0;
			Hist = 0;
		}
		else if (key == 't')
		{
			isgray = 0;
			isneg = 0;
			BgSb = 0;
			bThre = 1;
			Hist = 0;
		}
		else if (key == 'h')
			Hist = 1;
		else if (key == 'a') {
			averBlur = 1;
			sprintf_s(kernelName, "Max=%d", g_ntrakerMaxValue);
			createTrackbar(kernelName, title, &g_nTrackerbarValue, g_ntrakerMaxValue, on_nkernelTracker);
		}
			
		if (isgray) {
			cvtColor(frame, dst, COLOR_RGB2GRAY);
			imshow(title, dst);
			if (Hist)
				imshow("直方圖", draw(dst));
			else if (averBlur) 
				on_nkernelTracker(g_nKernelValue, 0);			
		}
		else if (isneg) {
			dst = Vec3b(255, 255, 255) ^ frame;
			imshow(title, dst);
			if (Hist)
				imshow("直方圖", draw(dst));
			else if (averBlur) 
				on_nkernelTracker(g_nTrackerbarValue, 0);
		}
		else if (BgSb) {
			bg = imread("flymanBG.jpg");
			cvtColor(bg, bg, COLOR_RGB2GRAY);
			cvtColor(frame, frame, COLOR_RGB2GRAY);
			Mat diff;
			absdiff(frame, bg, dst);
			imshow(title, dst);
			if (Hist)
				imshow("直方圖", draw(dst));
			else if (averBlur) 
				on_nkernelTracker(g_nTrackerbarValue, 0);		
		}
		else if (bThre) {
			cvtColor(frame, frame, COLOR_BGR2GRAY);
			threshold(frame, dst, 95, 255, THRESH_BINARY);
			imshow(title, dst);
			if (Hist)
				imshow("直方圖", draw(dst));
			else if (averBlur) 
				on_nkernelTracker(g_nTrackerbarValue, 0);
		}
		else {
			dst = frame;
			imshow(title, dst);
			if (Hist)
				imshow("直方圖", draw(dst));
			else if (averBlur)
				on_nkernelTracker(g_nTrackerbarValue, 0);			
		}
	}
}

void camera() {
	VideoCapture capture(0);
	Mat frame, bg,diff;
	char key = 0;
	bool sub = 0;
	
	while (capture.isOpened() && key != 27)
	{
		key = waitKey(1);
		capture >> frame;
		if (key == 'c') {
			bg = frame;
			cvtColor(bg, bg, COLOR_RGB2GRAY);
			//imshow("bg", bg);
		}
		else if (key == 'a') {
			averBlur = 1;
			char kernelName[20];
			sprintf_s(kernelName, "Max=%d", g_ntrakerMaxValue);
			createTrackbar(kernelName, title, &g_nTrackerbarValue, g_ntrakerMaxValue, on_nkernelTracker);
		}
		else if (key == 'g' && !bg.empty() || sub == 1)
		{
			Mat  frame2, thre;
			sub = 1;
			cvtColor(frame, frame2, COLOR_RGB2GRAY);
			absdiff(frame2, bg, diff);
			threshold(diff, thre, 100, 250, THRESH_BINARY);
			cvtColor(thre, thre, COLOR_GRAY2RGB);
			bitwise_and(thre, frame, dst);
			imshow(title, dst);
			if (averBlur) 
				on_nkernelTracker(g_nTrackerbarValue, 0);			
		}

		else if(sub == 0){
			dst = frame;
			imshow(title, dst);
			if (averBlur) 
				on_nkernelTracker(g_nTrackerbarValue, 0);
		}		
		if (key == 27)
			break;
	}
}

int main(int argc, char **argv)
{
	//camera();
	play_video("flyman512x512.avi");
	//play_video("1.mp4");
}

