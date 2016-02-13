#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>

#include <configure.h>

using namespace std;

const string wname = "test";
const bool USE_GUI = false;
const double IMAGE_SCALE = 0.1;

cv::Scalar randColor(cv::RNG& rng){

	int color = rng();

	return cv::Scalar(color & 0xff, color >> 8 & 0xff, color >> 16 & 0xff);

}

void antiShake(vector<cv::Mat>& images, cv::Mat& dst){

	cv::Ptr<cv::FeatureDetector> detector = cv::BRISK::create();

	cout << "detect key points" << endl;

	cv::Mat& image0 = images[0];
	cv::Mat& image1 = images[1];

	vector<cv::KeyPoint> keypoints0;
	detector->detect(image0, keypoints0);

	vector<cv::KeyPoint> keypoints1;
	detector->detect(image1, keypoints1);

	cv::drawKeypoints(image0, keypoints0, dst);

	if (USE_GUI){
		cv::Mat show;
		cv::resize(dst, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
		cv::imshow(wname, show);
		cv::waitKey();
	}
	else{
		cv::imwrite("image_0_draw_keypoint.jpg", dst);
		cv::drawKeypoints(image1, keypoints1, dst);
		cv::imwrite("image_1_draw_keypoint.jpg", dst);
	}


	cout << "extract feature" << endl;
	cv::Mat desc0;
	detector->compute(image0, keypoints0, desc0);
	cv::Mat desc1;
	detector->compute(image1, keypoints1, desc1);

	cv::BFMatcher matcher;
	vector<cv::DMatch> matches;
	matcher.match(desc0, desc1, matches);

	cv::drawMatches(image0, keypoints0, image1, keypoints1, matches, dst);

	if (USE_GUI){
		cv::Mat show;
		cv::resize(dst, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
		cv::imshow(wname, show);
		cv::waitKey();
	}
	else{
		cv::imwrite("match_result.jpg", dst);
	}

	cout << "find fundamental mat" << endl;

	cv::Mat mask;
	vector<cv::Point2f> points0;
	vector<cv::Point2f> points1;
	for (int i = 0; i < matches.size(); i++){
		points0.push_back(keypoints0[matches[i].queryIdx].pt);
		points1.push_back(keypoints1[matches[i].trainIdx].pt);
	}

	cv::findFundamentalMat(points0, points1, CV_RANSAC, 3.0, 0.9999999, mask);

	vector<cv::Point2f> dstPoints0;
	vector<cv::Point2f> dstPoints1;
	for (int i = 0; i < points0.size(); i++){
		if (mask.at<unsigned char>(0, i)){
			dstPoints0.push_back(points0[i]);
			dstPoints1.push_back(points1[i]);
		}
	}

	cout << "show find fundamental result" << endl;
	cv::RNG rng;
	cv::Mat dst(image0.rows, image0.cols + image1.cols, CV_8UC3);
	image0.copyTo(cv::Mat(dst, cv::Rect(0, 0, image0.cols, image0.rows)));
	image1.copyTo(cv::Mat(dst, cv::Rect(image0.cols, 0, image1.cols, image1.rows)));

	if (USE_GUI){
		cv::Mat show;
		cv::resize(dst, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
		cv::imshow(wname, show);
		cv::waitKey();
	}
	else{
		cv::imwrite("pair_image.jpg", dst);
	}

	cout << "dstPoints.size()" << dstPoints0.size() << endl;

	for (int i = 0; i < dstPoints0.size(); i++){
		cv::Point2f &p0 = dstPoints0[i];
		cv::Point2f &p1 = cv::Point2f(dstPoints1[i].x + image0.cols, dstPoints1[i].y);
		cv::circle(dst, p0, 2, randColor(rng), -1);
		cv::circle(dst, p1, 2, randColor(rng), -1);
		cv::line(dst, p0, p1, randColor(rng));
	}

	if (USE_GUI){
		cv::Mat show;
		cv::resize(dst, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
		cv::imshow(wname, show);
		cv::waitKey();
	}
	else{
		cv::imwrite("findFundamentalMat_result.jpg", dst);
	}

	cv::Mat homography = cv::findHomography(dstPoints0, dstPoints1, CV_RANSAC, 1.0);
	cv::Mat image0dst;
	cv::warpPerspective(image0, image0dst, homography, image0.size());

	dst = image0dst / 2 + image1;

	if (USE_GUI){
		cv::Mat show;
		cv::resize(dst, show, cv::Size(), 0.5, 0.5);
		cv::imshow(wname, show);
		cv::waitKey();
	}
	else{
		cv::imwrite("findHomography_result.jpg", dst);
		dst = image0 / 2 + image1 / 2;
		cv::imwrite("original.jpg", dst);
	}
}


int main(int argc, char** argv){

	vector<cv::Mat> images;
	images.push_back(cv::imread(IMAGE_FILE_0));
	images.push_back(cv::imread(IMAGE_FILE_1));
	images.push_back(cv::imread(IMAGE_FILE_2));

	for (int i = 0; i < images.size(); i++){
		if (images[0].empty()){
			cerr << "empty image:" << i << endl;
			return 1;
		}
	}

	cv::Mat dst;
	antiShake(images, dst);

}