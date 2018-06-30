#include <opencv2\core\core.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <math.h>
#include <iostream>
#include <string>
#include <highgui.h>
#include <cv.h>

using namespace cv;
using namespace std;

void Mask(Mat &frametmp, Mat &output)
{
	for (int i = 0; i<frametmp.rows; i++)
	{
		for (int j = 0; j<frametmp.cols; j++)
		{
			if (output.at<uchar>(i, j) == 0)
			{
				frametmp.at<Vec3b>(i, j)[0] = 0;
				frametmp.at<Vec3b>(i, j)[1] = 0;
				frametmp.at<Vec3b>(i, j)[2] = 0;
			}
		}
	}
}
string int2str(int &count)
{
	string s;
	stringstream ss(s);
	ss << count;
	return ss.str();
}

int main() {
	vector< vector < Point > > contours;
	vector<Mat> imgbox, newimgbox, difference;

	Mat frame, output, dilatetmp, mask2;
	// Returns the structuring element of the specified size and shape for morphological operations.
	mask2 = getStructuringElement(MORPH_RECT, Size(5, 5));

	VideoCapture video("video.avi");
	if (!video.isOpened())
		cout << "fail to open!" << endl;

	BackgroundSubtractorMOG backgroundSubs;
	//namedWindow("showimg", 1);
	int count = 1, new_w = 0, new_h = 0;

	for (int i = 1; video.read(frame) != NULL; i++){
		//Background Subtractor
		backgroundSubs.operator()(frame, output, 0.01);

		// Morphology
		dilate(output, output, mask2);
		erode(output, output, mask2);

		Mat outputTmp;
		output.copyTo(outputTmp);

		findContours(output, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		if (i>55 && i<75 && contours.size() != 0){
			int top = 0, down = 0, left = 0, right = 0, h = 0, w = 0;
			for (int p = 0; p<contours.size(); p++)
			{
				left = contours[0][0].x;
				right = contours[0][0].x;
				top = contours[0][0].y;
				down = contours[0][0].y;

				// find out the coordinates of the four corners.
				for (int q = 0; q<contours[p].size(); q++){
					if (contours[p][q].x > right)
						right = contours[p][q].x;
					else if (contours[p][q].x < left)
						left = contours[p][q].x;

					if (contours[p][q].y > down)
						down = contours[p][q].y;
					else if (contours[p][q].y < top)
						top = contours[p][q].y;
				}

				h = down - top;
				w = right - left;
				if (h && w){
					Mat ROI = outputTmp(Rect(left, top, w, h));
					Mat test;
					ROI.copyTo(test);

					if (test.rows > new_h)
						new_h = test.rows;
					if (test.cols > new_w)
						new_w = test.cols;

					imgbox.push_back(ROI);
					count++;
				}
			}
		}
	}
	new_w = new_w + 100;
	int pixelcount = 0, sum_x = 0, sum_y = 0, avg_x = 0, avg_y = 0;

	//get the center point
	for (int i = 0; i<imgbox.size(); i++){
		Mat newMat = cvCreateMat(new_h, new_w, CV_32FC1);
		pixelcount = 0;
		sum_x = 0, sum_y = 0, avg_x = 0, avg_y = 0;
		for (int p = 0; p<(int)new_h*0.13; p++)
		{
			for (int q = 0; q<imgbox[i].cols; q++)
			{
				if (imgbox[i].at<uchar>(p, q) != 0)
				{
					pixelcount++;
					sum_x = sum_x + q;
					sum_y = sum_y + p;
				}
			}
		}
		avg_x = (int)sum_x / pixelcount;
		avg_y = (int)sum_y / pixelcount;

		imgbox[i].copyTo(newMat(Rect(((int)newMat.cols / 2) - (int)avg_x, 0, imgbox[i].cols, imgbox[i].rows)));

		Mat resultInCenter;
		newMat.copyTo(resultInCenter);

		imwrite("74.jpg", resultInCenter);
		imshow("Aligned Foreground", resultInCenter);
		waitKey(40);
		newimgbox.push_back(resultInCenter);
	}

	Mat lastframe;
	for (int i = 0; i<newimgbox.size(); i++)
	{
		threshold(newimgbox[i], newimgbox[i], 128, 255, CV_THRESH_BINARY);
		if (i)
		{
			Mat diff;
			absdiff(newimgbox[i], newimgbox[i - 1], diff);
			difference.push_back(diff);
			lastframe = diff;
		}
	}

	Mat img_sum;
	for (int i = 0; i<difference.size(); i++)
	{
		if (i == 1)
			scaleAdd(difference[0], 1, difference[1], img_sum);
		else if (i > 1)
			scaleAdd(img_sum, 1, difference[i], img_sum);
	}
	imshow("MEI_74", img_sum);
	imwrite("MEI_74.jpg", img_sum);

	waitKey(0);
	system("pause");

	return 0;
}