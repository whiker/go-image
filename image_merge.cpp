#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace cv;

using namespace std;


void match(Mat& left_desp, Mat& right_desp,
		   vector<DMatch>& matches);
void knnMatch(Mat& left_desp, Mat& right_desp, const float ratio,
			  vector<DMatch>& matches);
Mat ransac(vector<KeyPoint>& left_kpts, vector<KeyPoint>& right_kpts,
		   vector<DMatch>& matches);
Mat homography(vector<KeyPoint>& left_kpts, vector<KeyPoint>& right_kpts,
			   vector<DMatch>& matches);
void matchPoints(vector<KeyPoint>& il_kpts, vector<KeyPoint>& ir_kpts,
				 vector<DMatch>& matches, vector<Point2f>& ol_kpts, vector<Point2f>& or_kpts);
int cleanMatchOutliers(vector<uchar>& status, vector<DMatch>& matches, const char* msg = NULL);
void stitch(IplImage* left_img, IplImage* right_img, Mat& form);


void merge(IplImage* left_img, IplImage* right_img)
{
	// key points
	vector<KeyPoint> left_kpts, right_kpts;
	SurfFeatureDetector kpt_detector;
	kpt_detector.detect(left_img, left_kpts);
	kpt_detector.detect(right_img, right_kpts);

	// descriptor
	Mat left_desp, right_desp;
	SurfDescriptorExtractor desp_extractor;
	desp_extractor.compute(left_img, left_kpts, left_desp);
	desp_extractor.compute(right_img, right_kpts, right_desp);

	// knn-match
	vector<DMatch> matches;
	knnMatch(left_desp, right_desp, 0.8, matches);

	// filte
	Mat form;
	//form = ransac(left_kpts, right_kpts, matches);  // RANSAC
	form = homography(left_kpts, right_kpts, matches);  // 单应矩阵

	/*
	Mat ret, ret1;
	drawMatches(left_img, left_kpts, right_img, right_kpts, matches, ret1);
	resize(ret1, ret, Size(left_img->width*2, left_img->height));
	imshow("", ret);
	waitKey();
	//*/

	stitch(left_img, right_img, form);
}

void match(Mat& left_desp, Mat& right_desp,
		   vector<DMatch>& matches)
{
	BruteForceMatcher<L2<float>> matcher;
	matcher.match(left_desp, right_desp, matches);
}

void knnMatch(Mat& left_desp, Mat& right_desp, const float ratio,
			  vector<DMatch>& matches)
{
	vector<vector<DMatch>> matches_ori;
	BruteForceMatcher<L2<float>> matcher;
	matcher.knnMatch(left_desp, right_desp, matches_ori, 2);

	for (int i = 0; i < matches_ori.size(); i++)
	{
		float best = matches_ori[i][0].distance;
		float better = matches_ori[i][1].distance;

		if (best / better < ratio)
			matches.push_back(matches_ori[i][0]);
	}
	cout << "knn_match: " << matches_ori.size() << " --> " << matches.size() << endl;
}

// RANSAC
Mat ransac(vector<KeyPoint>& left_kpts, vector<KeyPoint>& right_kpts,
		   vector<DMatch>& matches)
{
	vector<Point2f> l_kpts, r_kpts;
	matchPoints(left_kpts, right_kpts, matches, l_kpts, r_kpts);

	vector<uchar> status;
	Mat funda = findFundamentalMat(l_kpts, r_kpts, status, FM_RANSAC);

	cleanMatchOutliers(status, matches, "ransac");
	return funda;
}

// 单应矩阵
Mat homography(vector<KeyPoint>& left_kpts, vector<KeyPoint>& right_kpts,
			   vector<DMatch>& matches)
{
	vector<Point2f> l_kpts, r_kpts;
	matchPoints(left_kpts, right_kpts, matches, l_kpts, r_kpts);

	vector<uchar> status;
	Mat H = findHomography(l_kpts, r_kpts, CV_FM_RANSAC, 3, status);

	cleanMatchOutliers(status, matches, "homography");
	return H;
}

void matchPoints(vector<KeyPoint>& il_kpts, vector<KeyPoint>& ir_kpts,
				 vector<DMatch>& matches, vector<Point2f>& ol_kpts, vector<Point2f>& or_kpts)
{
	int n_matches = matches.size();
	ol_kpts.resize(n_matches);
	or_kpts.resize(n_matches);

	for (int i = 0; i < n_matches; i++)
	{
		DMatch& m = matches[i];
		ol_kpts[i].x = il_kpts[m.queryIdx].pt.x;
		ol_kpts[i].y = il_kpts[m.queryIdx].pt.y;
		or_kpts[i].x = ir_kpts[m.trainIdx].pt.x;
		or_kpts[i].y = ir_kpts[m.trainIdx].pt.y;
	}
}

// 去除匹配点对中的外点(outlier)
int cleanMatchOutliers(vector<uchar>& status, vector<DMatch>& matches, const char* msg)
{
	int cnt = 0;
	int n_matches = matches.size();
	for (int i = 0; i < n_matches; i++)
	{
		if (status[i])
			matches[cnt++] = matches[i];
	}
	matches.resize(cnt);
	if (msg)
		cout << msg << ": " << n_matches << " --> " << cnt << endl;
	return cnt;
}

// 拼接
void stitch(IplImage* left_img, IplImage* right_img, Mat& form)
{
	int w1 = left_img->width, w2 = right_img->width;
	int h1 = left_img->height, h2 = right_img->height;

	Mat form_inv;
	invert(form, form_inv);
	CvMat form_mat = form_inv;

	IplImage* ret = cvCreateImage(cvSize(w1+w2,max(h1,h2)),
			left_img->depth, left_img->nChannels);
	cvWarpPerspective(right_img, ret, &form_mat);

	cvSetImageROI(ret, cvRect(0,0,w1,h1));
	cvCopy(left_img, ret);
	cvResetImageROI(ret);

	cvShowImage("", ret);
	waitKey();
}
