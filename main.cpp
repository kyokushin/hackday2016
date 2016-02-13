#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>

#include <configure.h>

using namespace std;

const string wname = "test";

void antiShake(vector<cv::Mat>& images, cv::Mat& dst){

	cv::FeatureDetector *detector = cv::ORB::create();

	cv::Mat& image = images[0];

	vector<cv::KeyPoint> keypoints;
	detector->detect(image, keypoints);
	cv::Mat descriptor;
	detector->compute(image, keypoints, descriptor);

	cv::Mat show = image.clone();
	cv::drawKeypoints(image, keypoints, show);

	cv::imshow(wname, show);
	cv::waitKey();

}

int main(int argc, char** argv){

	vector<cv::Mat> images;
	images.push_back(cv::imread(IMAGE_FILE_0));
	images.push_back(cv::imread(IMAGE_FILE_1));
	images.push_back(cv::imread(IMAGE_FILE_2));

	cv::Mat dst;
	antiShake(images, dst);

}