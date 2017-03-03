//效能提升版
#include <highgui.h>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <vector>

using namespace cv;
using namespace std;

//CvCapture *capture;
int start, stop;
int D[100][100];	//記錄各點之距離

struct betterXY		//點之特性
{
	int x;
	int y;
	int flag;
	int mid3;
	int p1, p2;		//三連線:兩端點之陣列索引
	int reverse;	//端點之中間點索引
	int a90;		//直角
	int square;		//矩形
	int iso;
};

betterXY point[100]; //圓心
IplImage* pImg, *bw, *gray;
int bye = 0;

void foo(int event, int x, int y, int flags, void* param)
{
	if (flags == 1)
	{
		bye = 1;
	}
}

double angle(int x1, int y1, int x2, int y2, int x3, int y3)	//餘弦值
{
	double dx1 = x1 - x3;
	double dy1 = y1 - y3;
	double dx2 = x2 - x3;
	double dy2 = y2 - y3;
	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

int dist(int x1, int y1, int x2, int y2)//兩點距離
{
	double tmp;
	tmp = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	return (int)tmp;
}


int parse356(int num)
{
	int count = 0, flag = 1;

	for (int i = 0; i < num; i++)
	{
		//先找三連線中間點
		for (int j = 0;j < num; j++)
		{
			for (int k = 0;k < num; k++)
			{
				if (i != j && i != k && angle(point[j].x, point[j].y, point[k].x, point[k].y, point[i].x, point[i].y) + 1 < 0.05)
				{
					if (D[j][k] < 35//兩端距離太遠 非三連線
						&& fabs(D[i][k] - D[i][j]) <= 2) //兩端與中點距離不相同非三連線
					{
						point[i].mid3 = 1;
						point[j].flag = point[k].flag = point[i].flag = flag;
						point[i].p1 = j;
						point[i].p2 = k;
						point[j].reverse = i;
						point[k].reverse = i;

					}
				}
			}
		}
	}

	//以下處理6
	//相連6不分割
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num; j++)
		{
			if (point[i].mid3 == 1 && point[j].mid3 == 1)
			{
				if (i != j && D[i][j] < 25)
				{
					//兩中間點距離很近
					point[j].flag = point[i].flag;
					point[point[j].p1].flag = point[point[j].p2].flag = point[i].flag;
				}
			}
		}
	}

	return 1;
} //parse356


int parse1(int num)
{
	int min = 1000;
	int t;
	//最小距離34

	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num; j++)
		{
			if (i != j)
			{
				if (point[i].a90 == 0 && point[i].mid3 == 0 && point[point[i].reverse].mid3 == 0)
				{//i非?	x形點 中間點 與中間點相連之點
					if (D[i][j] < min)
					{
						min = D[i][j];
					}

					if (min >= 34 && min < 100)
					{
						point[i].iso = 1;
					}
				}//if
			}
			min = 1000;
		}
	}//for i

	return 1;
}//parse1


int parse4(int num)
{
	//找所有矩形點
	//相連4不分割
	for (int i = 0;i < num;i++)
	{
		for (int j = 0;j < num;j++)
		{
			for (int k = 0;k < num;k++)
			{
				if (point[i].flag != 0 && point[i].mid3 == 0 && point[point[i].reverse].mid3 == 0)//i非中間點及與中間點相連之點
				{
					if (i != j&&point[j].flag != 0 && point[j].mid3 == 0 && point[point[j].reverse].mid3 == 0)//j非中間點及與中間點相連之點
					{
						if (k != i&&k != j&&point[k].flag != 0 && point[k].mid3 == 0 && point[point[k].reverse].mid3 == 0)//k非中間點及與中間點相連之點
						{
							if (fabs(angle(point[j].x, point[j].y, point[k].x, point[k].y, point[i].x, point[i].y)) < 0.1)//直角
							{
								if (fabs(D[i][k] - D[i][j]) <= 4)//兩端與直角點距離不相同非直角
								{
									point[i].a90 = 1;
								}
							}
						}
					}
				}
			}
		}
	}

	//(jmi)=0&&(ikj)=0&&(jmk)=0&&(ikm)=0
	for (int i = 0;i < num;i++)
	{
		for (int j = 0;j < num;j++)
		{
			for (int k = 0;k < num;k++)
			{
				for (int m = 0;m < num;m++)
				{
					if (i != j&&i != k&&i != m)
					{
						if (point[i].a90 == 1 && point[j].a90 == 1 && point[k].a90 == 1 && point[m].a90 == 1)//直角點
						{
							/*i m j k*/
							if (fabs(angle(point[j].x, point[j].y, point[m].x, point[m].y, point[i].x, point[i].y)) < 0.1&&
								fabs(angle(point[i].x, point[i].y, point[k].x, point[k].y, point[j].x, point[j].y)) < 0.1&&
								fabs(angle(point[j].x, point[j].y, point[m].x, point[m].y, point[k].x, point[k].y)) < 0.1&&
								fabs(angle(point[i].x, point[i].y, point[k].x, point[k].y, point[m].x, point[m].y)) < 0.1)
							{
								point[j].flag = point[k].flag = point[m].flag = point[i].flag;
								point[i].square = point[j].square = point[k].square = point[m].square = 1;//矩形點

							}
						}
					}
				}
			}
		}
	}

	//消除非矩形點的直角點
	for (int i = 0;i < num;i++)
	{
		if (point[i].a90 == 1 && point[i].square == 0)point[i].a90 = 0;
	}

	return 1;
} //parse4

int p[700][700];
void houghcycle(IplImage * src, int r, int N, int xs, int ys)
{
	for (int j = 0;j < src->height;j++)
	{
		for (int i = 0;i < src->width;i++)
		{
			p[i][j] = 0;
		}
	}

	for (int i = 0; i < 18; i++)
	{
		point[i].x = point[i].y = point[i].flag = point[i].mid3 = point[i].reverse = point[i].a90 = point[i].square = point[i].iso = 0;//初始0
	}

	for (int j = 0;j < src->height;j++)
	{
		for (int i = 0;i < src->width;i += 2)
		{
			CvScalar s;
			s = cvGet2D(src, j, i);
			if (s.val[0] == 255)
			{//白
				double t;
				int y;
				//找出該點所有半徑r的可能圓方程
				for (int x = i - 6; x <= i + 6;x++)
				{
					if (x >= 0 && x < src->width)
					{
						t = r*r - (i - x)*(i - x);//(x,y)為圓心座標
						if (t >= 0)
						{
							t = sqrt(t);
							y = j - t;//上半
							if (y >= 0 && y < src->height)
							{
								p[(x / xs) * xs][(y / ys) * ys]++;//統計圓心
							}

							y = j + t;//下半
							if (y >= 0 && y < src->height)
							{
								p[(x / xs)*xs][(y / ys)*ys]++;//統計圓心
							}
						}//if t>=0
					}//if

					if (x + 1 >= 0 && x + 1 < src->width)
					{
						t = r*r - ((i + 1) - (x + 1))*((i + 1) - (x + 1));//(x,y)為圓心座標
						if (t >= 0)
						{
							t = sqrt(t);
							y = j - t;//上半
							if (y >= 0 && y < src->height)
							{
								p[((x + 1) / xs)*xs][(y / ys)*ys]++;//統計圓心
							}
							y = j + t;//下半
							if (y >= 0 && y < src->height)
							{
								p[((x + 1) / xs)*xs][(y / ys)*ys]++;//統計圓心
							}
						}//if t>=0
					}//if
				}//for x
			}//if 255
		}
	}

	int count = 0;

	for (int j = 0;j < src->height;j++)
	{
		for (int i = 0;i < src->width;i++)
		{
			if (p[i][j] >= N)
			{//N為圓上點的最小數量
				//(i,j)為圓心
				for (int k = 0;k < 18;k++)
				{
					point[count].x = i;
				}
				point[count].y = j;
				point[count].flag = count + 1;
				count++;
			}

			if (p[i][j] >= N)
			{
				//printf("p[%d][%d]=%d\n",i,j,p[i][j]);
			}
		}
	}


	for (int i = 0;i < 100;i++)
	{
		if (point[i].x != 0)
		{
			for (int j = 0;j < 100;j++)
			{
				if (i != j&&point[j].x != 0 && dist(point[i].x, point[i].y, point[j].x, point[j].y) <= 11)
				{//去掉重疊及過近的圓
					if (p[point[i].x][point[i].y] > p[point[j].x][point[j].y])
					{
						point[j].x = point[j].y = point[j].flag = 0;
					}
					else
					{
						point[i].x = point[i].y = point[i].flag = 0;
					}
				}

				int max = 0;
				for (int i = 0;i < 100;i++)
				{
					if (p[point[i].x][point[i].y] > max)max = p[point[i].x][point[i].y];
					{
						for (int i = 0;i < 100;i++)
						{
							if (p[point[i].x][point[i].y] < max * 2 / 3)
							{
								point[i].x = point[i].y = point[i].flag = 0;//多出來的點通常統計值特別小
							}
						}
					}
				}
			}
		}
	}
}


void calcCircles_New(const Mat &input)
{
	IplImage *frame, *change, *getHSV;
	frame = &IplImage(input);
	CvSize ImageSize = cvSize(frame->width, frame->height);
	change = cvCreateImage(ImageSize, IPL_DEPTH_8U, 1);
	getHSV = cvCreateImage(ImageSize, IPL_DEPTH_8U, 3);

	cvSmooth(frame, getHSV, CV_BILATERAL, 7, 0, 600, 600);
	cvCvtColor(getHSV, change, CV_RGB2GRAY);

	//cvShowImage("BILATERAL", getHSV);

	cvSmooth(change, change, CV_MEDIAN, 3, 0, 0, 0);
	cvSmooth(change, change, CV_GAUSSIAN, 3, 3, 1, 1);

	cvNot(change, change);

	cvThreshold(change, change, 50, 255, CV_THRESH_BINARY);

	cvCanny(change, change, 15, 255, 3);

	cvDilate(change, change, 0, 1);
	cvErode(change, change, 0, 1);

	cvShowImage("change", change);

	houghcycle(change, 5, 8, 1, 1);
}

int main()
{
	IplConvKernel * pKernel = NULL;
	//capture = cvCreateCameraCapture(0);
	VideoCapture capture(0);
	if (!capture.isOpened()) return 0;
	capture.set(CAP_PROP_EXPOSURE, -6);			//曝光，此camera原始值-5
	capture.set(CAP_PROP_CONTRAST, 20);			//對比，此camera原始值20
	capture.set(CAP_PROP_BRIGHTNESS, 200);		//亮度，此camera原始值10
	capture.set(CAP_PROP_GAMMA, 100);		//Gamma，此camera原始值200
	Mat frame, grayframe;

	capture >> frame;
	pImg = &IplImage(frame);
	//pImg = cvQueryFrame(&capture);
	bw = cvCreateImage(cvSize(pImg->width, pImg->height), IPL_DEPTH_8U, 1);
	gray = cvCreateImage(cvSize(pImg->width, pImg->height), IPL_DEPTH_8U, 1);
	timeBeginPeriod(1);
	timeEndPeriod(1);

	while (1)
	{
	top:
		capture >> frame;
		pImg = &IplImage(frame);
		//pImg = cvQueryFrame(capture);
		cvNamedWindow("原圖", 1);
		cvSetMouseCallback("原圖", foo);
		cvShowImage("原圖", pImg);
		if (cvWaitKey(1) >= 0)
		{//1ms後按任意鍵可退出
			break;

			if (bye == 1) {
				//cvReleaseCapture(&capture);
				return 1;//放無窮迴圈內

			}
		}

		start = timeGetTime();//開始辨識
		int num, num1;
		//IplImage * canny = cvCreateImage(cvSize(pImg->width, pImg->height), IPL_DEPTH_8U, 1);
		for (int i = 0;i < 5;i++)
		{//提高i可重複檢測是否少點
			//cvtColor(frame, grayframe, CV_BGR2GRAY);
			//GaussianBlur(grayframe, grayframe, Size(5, 5), 0, 0, BORDER_CONSTANT);			//高斯平滑
			//gray = &IplImage(grayframe);
			//cvCvtColor(pImg, gray, CV_BGR2GRAY);
			//cvCanny(gray, canny, 50, 150, 3);
			//cvCanny(gray, canny, 100, 100);
			//houghcycle(canny, 5, 14, 1, 1);//找圓心
			calcCircles_New(frame);
			for (int j = 0;j < 100;j++)
			{//去除投影圓R+G+B<400
				CvScalar s = cvGet2D(pImg, point[j].y, point[j].x);
				if (s.val[0] + s.val[1] + s.val[2] < 400)
				{
					point[j].x = point[j].y = point[j].flag = 0;
				}
			}
			num = 0;
			for (int i = 0;i < 100;i++)
			{
				if (point[i].x != 0)
				{
					num++;
				}
			}
		}//提高i可重複檢測是否少點

		for (int i = 0;i < 99;i++)//把圓心搬到前面
		{
			for (int j = 0;j < 99 - i;j++)
			{
				betterXY t;
				if (point[j].flag < point[j + 1].flag)
				{
					t = point[j + 1];
					point[j + 1] = point[j];
					point[j] = t;
				}
			}
		}

		//先記錄各點之距離
		for (int i = 0;i < num;i++)
		{
			for (int j = i + 1;j < num;j++)
			{
				D[i][j] = D[j][i] = dist(point[i].x, point[i].y, point[j].x, point[j].y);
			}
		}

		parse356(num);//找點數356骰子
		parse4(num);//找點數4骰子
		parse1(num);//找點數1骰子

		//Mat cvframe;
		//frame.copyTo(cvframe);
		IplImage* cr = cvCreateImage(cvSize(pImg->width, pImg->height), IPL_DEPTH_8U, 3);
		//cr = &IplImage(cvframe);
		cvZero(cr);
		for (int i = 0;i < 100;i++)
		{
			if (point[i].x != 0 && point[i].mid3 == 0 && point[i].a90 == 0 && point[i].iso == 0)
			{
				//綠色非孤立點
				cvCircle(cr, cvPoint(point[i].x, point[i].y), 5, CV_RGB(0, 200, 0));
			}

			if (point[point[i].reverse].mid3 == 1)
			{
				//紫色兩端點
				cvCircle(cr, cvPoint(point[i].x, point[i].y), 5, CV_RGB(200, 0, 200));
			}
			
			if (point[i].x != 0 && point[i].mid3 == 0 && point[i].a90 != 0 && point[i].iso == 0)
			{
				//藍色矩形點
				cvCircle(cr, cvPoint(point[i].x, point[i].y), 5, CV_RGB(0, 0, 200));
			}
			
			if (point[i].x != 0 && point[i].mid3 != 0 && point[i].a90 == 0 && point[i].iso == 0)
			{
				//紅色中間點
				cvCircle(cr, cvPoint(point[i].x, point[i].y), 5, CV_RGB(200, 0, 0));
			}
			
			if (point[i].iso == 1)
			{
				//黃色孤立點
				cvCircle(cr, cvPoint(point[i].x, point[i].y), 5, CV_RGB(200, 200, 0));
			}
		}

		for (int i = 0;i < num;i++)
		{
			for (int j = 0;j < num;j++)
			{
				if (point[i].flag == point[j].flag&&i != j&&point[i].flag != 0)
				{
					cvLine(cr, cvPoint(point[i].x, point[i].y), cvPoint(point[j].x, point[j].y), CV_RGB(255, 255, 0), 1);
				}
			}
		}

		int tab[1000];//tab設太小 flag值若超過其大小時無法計數
		for (int i = 0;i < 1000;i++)
		{
			tab[i] = 0;//初始0
		}

		//計算同flag的數量
		for (int i = 0;i < num;i++)
		{
			tab[point[i].flag]++;
		}

		//以下調整tab
		int midnum, a90num;
		midnum = a90num = 0;
		for (int i = 0;i < 100;i++)
		{
			if (tab[i] >= 8)
			{
				for (int j = 0;j < num;j++)
				{
					if (point[j].flag == i)
					{
						if (point[j].mid3 == 1) midnum++;
						if (point[j].a90 == 1) a90num++;
					}
				}

				//-----------------------------------------------------------------
				if (midnum == 4)
				{//2個6點
					for (int j = 0;j < 100;j++)
					{
						if (tab[j] == 0)
						{
							tab[i] -= 6;
							tab[j] += 6;
							goto a1;
						}
					}
				}

				if (midnum == 6)
				{//3個6點
					for (int j = 0;j < 100;j++)
					{
						for (int k = 0;k < 100;k++)
						{
							if (j != k&&tab[j] == 0 && tab[k] == 0)
							{
								tab[i] -= 12;
								tab[j] += 6;
								tab[k] += 6;
								goto a1;
							}
						}
					}
				}

				if (a90num == 8)
				{//2個4點
					for (int j = 0;j < 100;j++)
					{
						if (tab[j] == 0)
						{
							tab[i] -= 4;
							tab[j] += 4;
							goto a1;
						}
					}
				}

				if (a90num == 12)
				{//3個4點
					for (int j = 0;j < 100;j++)
					{
						for (int k = 0;k < 100;k++)
						{
							if (j != k&&tab[j] == 0 && tab[k] == 0)
							{
								tab[i] -= 8;
								tab[j] += 4;
								tab[k] += 4;
								goto a1;
							}
						}
					}
				}
			}
		}//for i
	a1:


		//計算剩下的非孤立點數量
		int notiso;
		notiso = 0;
		for (int i = 0;i < num;i++)
		{
			if (point[i].iso == 0 && point[i].mid3 == 0 && point[point[i].reverse].mid3 == 0 && point[i].a90 == 0)
			{
				notiso++;
			}
		}

		while (notiso >= 2)
		{
			for (int i = 0;i < 100;i++)
			{
				for (int j = i + 1;j < 100;j++)
				{
					if (tab[i] == 1 && tab[j] == 1)
					{
						tab[i]++;
						tab[j]--;
						goto a2;
					}
				}
			}
		a2:
			notiso -= 2;
		}//while


		//---------------------------------補點
		//6 少1個中間點(多2個非孤立點)
		//找孤立3連線端點ij 存在2點km dist(ik)=dist(jm) dist(ij)=dist(km) jik與ikm為直角
		//找tab值為3的點+3 另外去掉1個2點
		//j m
		//i k
		for (int i = 0;i < num;i++)
		{
			for (int j = 0;j < num;j++)
			{
				for (int k = 0;k < num;k++)
				{
					for (int m = 0;m < num;m++)
					{
						if (i != j&&point[i].mid3 == 0 && point[j].mid3 == 0 && point[point[i].reverse].mid3 == 1 && point[point[j].reverse].mid3 == 1)
						{
							if (k != m&&point[k].mid3 == 0 && point[m].mid3 == 0 && point[point[k].reverse].mid3 == 0 && point[point[m].reverse].mid3 == 0)
							{
								if (point[k].iso == 0 && point[m].iso == 0 && point[k].a90 == 0 && point[m].a90 == 0)//非矩?恲I孤立點
								{
									if (fabs(angle(point[j].x, point[j].y, point[k].x, point[k].y, point[i].x, point[i].y)) < 0.1)
									{
										if (fabs(angle(point[i].x, point[i].y, point[m].x, point[m].y, point[k].x, point[k].y)) < 0.1)
										{
											if (fabs(D[i][k] - D[j][m]) <= 4)
											{
												if (fabs(D[i][j] - D[k][m]) <= 4)
												{
													point[k].reverse = point[m].reverse = point[i].reverse;//防止km重做

													for (int x = 0;x < 100;x++)
													{
														if (tab[x] == 3)
														{
															tab[x] += 3;
															printf("6點少中點\n");
															goto A;
														}
													}
												A:

													for (int y = 0;y < 100;y++)
													{
														if (tab[y] == 2)
														{
															tab[y] -= 2;
															goto AA;
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	AA:

		//6少1個端點(多2個非孤立點)
		//找孤立3連線端點i與中間點j 存在2點km dist(ik)=dist(jm) dist(ij)=dist(km) jik與ikm為直角
		//找tab值為3的點+3 另外去掉1個2點
		for (int i = 0;i < num;i++)
		{
			for (int j = 0;j < num;j++)
			{
				for (int k = 0;k < num;k++)
				{
					for (int m = 0;m < num;m++)
					{
						if (point[i].mid3 == 0 && point[j].mid3 == 1 && point[point[i].reverse].mid3 == 1)
						{
							if (k != m&&point[k].mid3 == 0 && point[m].mid3 == 0 && point[point[k].reverse].mid3 == 0 && point[point[m].reverse].mid3 == 0)
							{
								if (point[k].iso == 0 && point[m].iso == 0 && point[k].a90 == 0 && point[m].a90 == 0)//非矩?恲I孤立點
								{
									if (fabs(angle(point[j].x, point[j].y, point[k].x, point[k].y, point[i].x, point[i].y)) < 0.1)
									{
										if (fabs(angle(point[i].x, point[i].y, point[m].x, point[m].y, point[k].x, point[k].y)) < 0.1)
										{
											if (fabs(angle(point[j].x, point[j].y, point[k].x, point[k].y, point[m].x, point[m].y)) < 0.1)
											{
												if (fabs(D[i][k] - D[j][m]) <= 4)
												{
													if (fabs(D[i][j] - D[k][m]) <= 4)
													{
														point[k].reverse = point[m].reverse = point[i].reverse;//防止km重做

														for (int x = 0;x < 100;x++)
														{
															if (tab[x] == 3)
															{
																tab[x] += 3;
																printf("6點少端點\n");
																goto A1;
															}
														}
													A1:

														for (int y = 0;y < 100;y++)
														{
															if (tab[y] == 2)
															{
																tab[y] -= 2;
																goto A2;
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	A2:

		//5少1個端點(多1個非孤立點)不處理少中間點 因為與4點無法區分
		//找孤立3連線端點i與中間點j 存在1點kdist(ij)=dist(jk) ijk為直角
		//找tab值為3的點+2 另外去掉1個1點
		//j k
		//i
		for (int i = 0;i < num;i++)
		{
			for (int j = 0;j < num;j++)
			{
				for (int k = 0;k < num;k++)
				{
					if (i != j&&i != k&&point[i].mid3 == 0 && point[j].mid3 == 1 && point[point[i].reverse].mid3 == 1)
					{
						if (point[k].mid3 == 0 && point[point[k].reverse].mid3 == 0)
						{
							if (point[k].iso == 0 && point[k].a90 == 0)//非矩形點孤立點
							{
								if (fabs(angle(point[i].x, point[i].y, point[k].x, point[k].y, point[j].x, point[j].y)) < 0.1)
								{
									if (fabs(D[i][j] - D[j][k]) <= 4)
									{
										point[k].reverse = point[i].reverse;//防止k重做

										for (int x = 0;x < 100;x++)
										{
											if (tab[x] == 3)
											{
												tab[x] += 2;
												printf("5點少1點\n");
												goto b1;
											}
										}
									b1:

										for (int y = 0;y < 100;y++)
										{
											if (tab[y] == 1 || tab[y] == 2)
											{
												tab[y] -= 1;
												goto b2;
											}//1:奇數個非孤立點2:偶數個非孤立點
										}
									}
								}
							}
						}
					}
				}
			}
		}
	b2:

		//4少一個直角點(多3個非孤立點)
		//ijk為直角 dist(ij)=dist(jk) 找tab值為1的點+3 另外去掉1個2點
		//j k
		//i
		for (int i = 0;i < num;i++)
		{
			for (int j = 0;j < num;j++)
			{
				for (int k = 0;k < num;k++)
				{
					if (i != j&&i != k&&point[i].mid3 == 0 && point[j].mid3 == 0 && point[k].mid3 == 0)
					{
						if (point[point[i].reverse].mid3 == 0 && point[point[j].reverse].mid3 == 0 && point[point[k].reverse].mid3 == 0)
						{
							if (point[i].a90 == 0 && point[j].a90 == 0 && point[k].a90 == 0)
							{
								if (point[i].iso == 0 && point[j].iso == 0 && point[k].iso == 0)
								{
									if (fabs(angle(point[i].x, point[i].y, point[k].x, point[k].y, point[j].x, point[j].y)) < 0.1)
									{
										if (fabs(D[i][j] - D[j][k]) <= 4)
										{
											point[i].a90 = point[j].a90 = point[k].a90 = 1;//防止重做

											for (int x = 0;x < 100;x++)
											{
												if (tab[x] == 1)
												{
													tab[x] += 3;
													printf("4點少1點\n");
													goto c1;
												}
											}
										c1:

											for (int y = 0;y < 100;y++)
											{
												if (tab[y] == 2)
												{
													tab[y] -= 2;
													goto c2;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	c2:

		//3 2個非孤立點超近 找tab值為2的點+1 不處理少中間點 因為與2點無法區分
		for (int i = 0;i < num;i++)
		{
			for (int j = 0;j < num;j++)
			{
				if (i != j&&point[i].mid3 == 0 && point[j].mid3 == 0)
				{
					if (point[point[i].reverse].mid3 == 0 && point[point[j].reverse].mid3 == 0)
					{
						if (point[i].a90 == 0 && point[j].a90 == 0)
						{
							if (point[i].iso == 0 && point[j].iso == 0)
							{
								if (D[i][j] <= 20)
								{
									point[i].iso = point[j].iso = 1;//防止重做

									for (int x = 0;x < 100;x++)
									{
										if (tab[x] == 2)
										{
											tab[x] += 1;
											printf("3點少1點\n");
											goto d1;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	d1:

		//---------------------------------補點
		stop = timeGetTime();//辨識完畢 tab建立完成
		int a = 99, b = 99, c = 99, t;
		for (int i = 0;i < 100;i++)
		{
			if (tab[i] == 0)
			{
				t = i;
				break;
			}
		}

		for (int i = 0;i < 100;i++)
		{
			if (tab[i] != 0)
			{
				a = i;
				break;
			}
		}

		for (int i = a + 1;i < 100;i++)
		{
			if (tab[i] != 0)
			{
				b = i;
				break;
			}
		}

		for (int i = b + 1;i < 100;i++)
		{
			if (tab[i] != 0)
			{
				c = i;
				break;
			}
		}

		char s[50] = { 0 };
		if (b == 99 && c == 99)
		{
			sprintf(s, "%d", tab[a]);
		}

		if (b != 99 && c == 99)
		{
			sprintf(s, "%d+%d=%d", tab[a], tab[b], tab[a] + tab[b]);
		}

		if (b != 99 && c != 99)
		{
			sprintf(s, "%d+%d+%d=%d", tab[a], tab[b], tab[c], tab[a] + tab[b] + tab[c]);
		}

		// 圖片,要寫的文字, 文字要寫的位置 ,文字風格 ,放大倍率 ,顏色
		Mat crmat = cvarrToMat(cr);
		putText(crmat, s, Point(19, 210), FONT_HERSHEY_TRIPLEX, 1, CV_RGB(255, 255, 255));
		sprintf(s, "%dms", stop - start);
		putText(crmat, s, Point(19, 240), FONT_HERSHEY_TRIPLEX, 1, CV_RGB(255, 255, 255));
		cvNamedWindow("結果", 1);
		cvShowImage("結果", cr);
		//cvNamedWindow("canny", 1);
		//cvShowImage("canny", canny);
		cvSaveImage("color.bmp", cr);
		//cvSaveImage("canny.bmp", canny);
		cvSaveImage("原圖.bmp", pImg);
	}

	/*
	//測試用code
	printf("i(%d,%d) j(%d,%d) k(%d,%d) m(%d,%d)\n",point[i].x,point[i].y,point[j].x,point[j].y,point[k].x,point[k].y,point[m].x,point[m].y);
	for(int i=0;i<num;i++)
	{
		printf("%d:矩形值=%d flag=%d mid3=%d iso=%d",i+1,point[i].a90,point[i].flag,point[i].mid3,point[i].iso);
		if(point[i].reverse!=0)
		{
			printf(" reverse=%d",point[point[i].reverse].mid3);
		}
		printf("\n");
	}
	for(int i=0;i<num;i++)
	{
		vScalar s=cvGet2D(pImg,point[i].y,point[i].x);
		printf("(%d,%d):R=%d G=%d B=%d\n",point[i].x,point[i].y,(int)s.val[2],(int)s.val[1],(int)s.val[0]);
	}
	*/

	//if (bye == 0)
	if (MessageBox(NULL, "繼續", "", MB_YESNO) == IDYES)
	{
		system("cls");
		cvDestroyAllWindows();
		//cvReleaseCapture(&capture);
		//capture = cvCreateCameraCapture(0); //不這樣做的話攝影機亮度會自動變暗
		goto top;
	}
}

