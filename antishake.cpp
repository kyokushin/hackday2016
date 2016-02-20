#include "antishake.h"

#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;

int max(const cv::Mat& mat){

	int max = 0;
	for (int i = 0; i < mat.rows; i++){
		const int *line = mat.ptr<int>(i);
		for (int j = 0; j < mat.cols; j++){
			if (max < line[j]){
				max = line[j];
			}
		}
	}

	return max;
}

void ohd3PlusOhd4(const cv::Mat& frameCompRes,
	const vector<cv::Mat>& labelImages,
	const vector<cv::Mat>& srcImages,
	cv::Mat& dst){

#ifdef _DEBUG
	const std::string wname("debug");
#endif

	if (dst.size() != labelImages[0].size() || dst.type() != srcImages[0].type()){
		cout << "initialize dst" << endl;
		dst = cv::Mat::zeros(srcImages[0].rows, srcImages[0].cols, srcImages[0].type());
	}

	cv::Mat dstFilled = cv::Mat(dst.rows, dst.cols, CV_8UC1);
	dstFilled.setTo(255);
	for (int i = 0; i < labelImages.size(); i++){
		cout << "label " << i << endl;
 		const cv::Mat& label = labelImages[i];

		cout << "find max index" << flush;
		const int maxIdx = max(label);
		cout << maxIdx << endl;

		cout << "init counter" << endl;
		vector<vector<int>> countsPerLabel;
		for (int j = 0; j <= maxIdx; j++){

			vector<int> tmp;
			for (int k = 0; k < labelImages.size(); k++){
				tmp.push_back(0);
			}
			countsPerLabel.push_back(tmp);
		}
		cout << "count image index in super pix" << endl;
		for (int j = 0; j < label.rows; j++){
			const int* line_label = label.ptr<int>(j);
			const unsigned char* line_fcmp = frameCompRes.ptr(j);

			for (int k = 0; k < label.cols; k++){
				int val_label = line_label[k];
				unsigned char val_fcmp = line_fcmp[k];

				countsPerLabel[val_label][val_fcmp]++;
			}
		}

		cout << "create label 2 image map" << endl;
		vector<int> label2image;
		for (int j = 0; j < countsPerLabel.size(); j++){
			vector<int>& counts = countsPerLabel[j];
			int maxCount = 0;
			int maxIdx = 0;
			for (int k = 0; k < counts.size(); k++){
				if (maxCount < counts[k]){
					maxCount = counts[k];
					maxIdx = k;
				}
			}
			label2image.push_back(maxIdx);
		}

		/*
		for (int j = 0; j < label2image.size(); j++){
			cout << "label " << j << "->image " << label2image[j] << endl;
		}
		*/

		cout << "apply result" << endl;
		for (int j = 0; j < label.rows; j++){
			//cout << "line " << j << endl;
			const int* line_label = label.ptr<int>(j);
			unsigned char* line_dst = dst.ptr(j);
			unsigned char* line_filled = dstFilled.ptr(j);

			for (int k = 0; k < label.cols; k++){
				//cout << "\tpx " << k << endl;
				int val_label = line_label[k];
				//cout << "\t\tlabel " << (int)val_label << endl;
				int useImageIdx = label2image[val_label];
				if (useImageIdx != i) continue;
				//cout << "\t\timage idx " << useImageIdx << endl;

				const cv::Mat& src = srcImages[useImageIdx];
				const unsigned char* px_src = src.ptr(j) + 3 * k;
				unsigned char* px_dst = line_dst + 3 * k;
				px_dst[0] = px_src[0];
				px_dst[1] = px_src[1];
				px_dst[2] = px_src[2];
				line_filled[k] = 0;
			}
		}
	}

#ifdef _DEBUG
	cv::imshow(wname, dst);
	cv::waitKey();

	cv::imshow(wname, dstFilled);
	cv::waitKey();
#endif
	

	{//remain area
		cv::Mat filledLabel;
		cv::connectedComponents(dstFilled, filledLabel, 4, CV_32S);
		cout << "find max index" << flush;
		const int maxIdx = max(filledLabel);
		cout << maxIdx << endl;

		cout << "init counter" << endl;
		vector<vector<int>> countsPerLabel;
		for (int j = 0; j < maxIdx; j++){

			vector<int> tmp;
			for (int k = 0; k < labelImages.size(); k++){
				tmp.push_back(0);
			}
			countsPerLabel.push_back(tmp);
		}
		cout << "count image index in super pix" << endl;
		for (int j = 0; j < filledLabel.rows; j++){
			const int* line_label = filledLabel.ptr<int>(j);
			const unsigned char* line_fcmp = frameCompRes.ptr(j);

			for (int k = 0; k < filledLabel.cols; k++){
				int val_label = line_label[k];
				unsigned char val_fcmp = line_fcmp[k];
				if (val_label == 0) continue;

				countsPerLabel[val_label-1][val_fcmp]++;
			}
		}
		cout << "create label 2 image map" << endl;
		vector<int> label2image;
		for (int j = 0; j < countsPerLabel.size(); j++){
			vector<int>& counts = countsPerLabel[j];
			int maxCount = 0;
			int maxIdx = 0;
			for (int k = 0; k < counts.size(); k++){
				if (maxCount < counts[k]){
					maxCount = counts[k];
					maxIdx = k;
				}
			}
			label2image.push_back(maxIdx);
		}

		/*
		for (int j = 0; j < label2image.size(); j++){
			cout << "label " << j << "->image " << label2image[j] << endl;
		}
		*/

		cout << "apply result" << endl;
		for (int j = 0; j < filledLabel.rows; j++){
			//cout << "line " << j << endl;
			const int* line_label = filledLabel.ptr<int>(j);
			unsigned char* line_dst = dst.ptr(j);
			unsigned char* line_filled = dstFilled.ptr(j);

			for (int k = 0; k < filledLabel.cols; k++){
				//cout << "\tpx " << k << endl;
				int val_label = line_label[k];
				if (val_label == 0) continue;
				//cout << "\t\tlabel " << (int)val_label << endl;
				int useImageIdx = label2image[val_label-1];
				//cout << "\t\timage idx " << useImageIdx << endl;

				const cv::Mat& src = srcImages[useImageIdx];
				const unsigned char* px_src = src.ptr(j) + 3 * k;
				unsigned char* px_dst = line_dst + 3 * k;
				px_dst[0] = px_src[0];
				px_dst[1] = px_src[1];
				px_dst[2] = px_src[2];
				*line_filled = 0;
			}
		}
#ifdef _DEBUG
		cv::imshow(wname, dst);
		cv::waitKey();
#endif

	}
}

template<typename T, typename S>
void antiShake(const vector<T>& images, vector<S>& dst){

	cv::Ptr<cv::FeatureDetector> detector;
	//detector = cv::BRISK::create();
	detector = cv::AKAZE::create();

	cout << "detect key points" << endl;

	const cv::Mat& image0 = images[0];
	if (image0.empty()){
		cerr << "empty image" << endl;
		return;
	}

	vector<cv::KeyPoint> keypoints0;
	detector->detect(image0, keypoints0);

	cout << "extract feature 0" << endl;
	cv::Mat desc0;
	detector->compute(image0, keypoints0, desc0);

	vector<cv::Point2f> imageRectPoints;
	imageRectPoints.push_back(cv::Point2f(0, 0));
	imageRectPoints.push_back(cv::Point2f(0, image0.rows));
	imageRectPoints.push_back(cv::Point2f(image0.cols, 0));
	imageRectPoints.push_back(cv::Point2f(image0.cols, image0.rows));

	cv::Mat resImage;
	cv::drawKeypoints(image0, keypoints0, resImage);
	if (USE_GUI){
		cv::Mat show;
		cv::resize(resImage, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
		cv::imshow(wname, show);
		cv::waitKey();
	}
	else{
		stringstream sstr;
		sstr << setw(5) << setfill('0') << 0 << "_" << 0 << "_image_draw_keypoint.jpg" << flush;
		cv::imwrite(sstr.str(), resImage);
	}

	//********************
	//image rect
	int left(0), right(image0.cols), top(0), bottom(image0.rows);

	vector<cv::Mat> transformedImages;
	for (int j = 1; j < images.size(); j++){
		int operation_count = 0;
		cout << "current image " << j << endl;

		cout << "\tdetect key points" << endl;

		const cv::Mat& image1 = images[j];
		if (image1.empty()){
			cerr << "empty image " << j << endl;
			return;
		}

		vector<cv::KeyPoint> keypoints1;
		detector->detect(image1, keypoints1);

		cv::drawKeypoints(image1, keypoints1, resImage);
		if (USE_GUI){
			cv::Mat show;
			cv::resize(resImage, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
			cv::imshow(wname, show);
			cv::waitKey();
		}
		else{
			stringstream sstr;
			sstr << setw(5) << setfill('0') << j << "_" << operation_count++ << "_image_draw_keypoint.jpg" << flush;
			cv::imwrite(sstr.str(), resImage);
		}

		cout << "\tdetect feature" << endl;

		cv::Mat desc1;
		detector->compute(image1, keypoints1, desc1);

		cout << "\tblout fource match" << endl;
		cv::BFMatcher matcher;
		vector<cv::DMatch> matches;
		matcher.match(desc1, desc0, matches);

		cout << "\tdraw matches" << endl;
		cv::drawMatches(image1, keypoints1, image0, keypoints0, matches, resImage);

		if (USE_GUI){
			cv::Mat show;
			cv::resize(resImage, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
			cv::imshow(wname, show);
			cv::waitKey();
		}
		else{
			stringstream sstr;
			sstr << setw(5) << setfill('0') << j << "_" << operation_count++ << "_match_result.jpg" << flush;
			cv::imwrite(sstr.str(), resImage);
		}

		cout << "\tfind fundamental mat" << endl;

		cv::Mat mask;
		vector<cv::Point2f> points0;
		vector<cv::Point2f> points1;
		for (int i = 0; i < matches.size(); i++){
			points0.push_back(keypoints0[matches[i].trainIdx].pt);
			points1.push_back(keypoints1[matches[i].queryIdx].pt);
		}

		cv::findFundamentalMat(points0, points1, CV_RANSAC, 3.0, 0.9999999, mask);

		vector<cv::Point2f> resImagePoints0;
		vector<cv::Point2f> resImagePoints1;
		for (int i = 0; i < points0.size(); i++){
			if (mask.at<unsigned char>(0, i)){
				resImagePoints0.push_back(points0[i]);
				resImagePoints1.push_back(points1[i]);
			}
		}

		cout << "\tshow find fundamental result" << endl;
		cv::RNG rng;
		resImage = cv::Mat(image0.rows, image0.cols + image1.cols, CV_8UC3);
		image0.copyTo(cv::Mat(resImage, cv::Rect(0, 0, image0.cols, image0.rows)));
		image1.copyTo(cv::Mat(resImage, cv::Rect(image0.cols, 0, image1.cols, image1.rows)));

		if (USE_GUI){
			cv::Mat show;
			cv::resize(resImage, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
			cv::imshow(wname, show);
			cv::waitKey();
		}
		else{
			stringstream sstr;
			sstr<< setw(5) << setfill('0') << j << "_" << operation_count++ << "_pair_image.jpg" << flush;
			cv::imwrite(sstr.str(), resImage);
		}

		cout << "\tdraw matches" << resImagePoints0.size() << endl;

		for (int i = 0; i < resImagePoints0.size(); i++){
			const cv::Point2f &p0 = resImagePoints0[i];
			const cv::Point2f &p1 = cv::Point2f(resImagePoints1[i].x + image0.cols, resImagePoints1[i].y);
			cv::circle(resImage, p0, 2, randColor(rng), -1);
			cv::circle(resImage, p1, 2, randColor(rng), -1);
			cv::line(resImage, p0, p1, randColor(rng));
		}

		if (USE_GUI){
			cv::Mat show;
			cv::resize(resImage, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
			cv::imshow(wname, show);
			cv::waitKey();
		}
		else{
			stringstream sstr;
			sstr<< setw(5) << setfill('0') << j << "_" << operation_count++ << "_findFundamentalMat_result.jpg" << flush;
			cv::imwrite(sstr.str(), resImage);
		}

		cout << "\tfind homography" << endl;
		cv::Mat homography1to0 = cv::findHomography(resImagePoints1, resImagePoints0, CV_RANSAC, 1.0);
		cv::Mat image1resImage;
		cv::warpPerspective(image1, image1resImage, homography1to0, image1.size());

		cout << "\timage area transformation use homography" << endl;
		vector<cv::Point2f> imageRectPointsDst;
		cv::perspectiveTransform(imageRectPoints, imageRectPointsDst, homography1to0);

		//more internal rect pos
		// 0 : left top
		// 1 : left bottom
		// 2 : right top
		// 3 : right bottom
		//left
		{
			int tmpLeft = imageRectPointsDst[0].x;
			if (tmpLeft < imageRectPointsDst[1].x){
				tmpLeft = imageRectPointsDst[1].x;
			}
			if (left < tmpLeft){
				left = tmpLeft;
			}
		}
		//right
		{
			int tmpRight = imageRectPointsDst[2].x;
			if (tmpRight > imageRectPointsDst[3].x){
				tmpRight = imageRectPointsDst[3].x;
			}
			if (right > tmpRight){
				right = tmpRight;
			}
		}
		//top
		{
			int tmpTop = imageRectPointsDst[0].y;
			if (tmpTop < imageRectPointsDst[2].y){
				tmpTop = imageRectPointsDst[2].y;
			}
			if (top < tmpTop){
				top = tmpTop;
			}
		}
		//bottom
		{
			int tmpBottom = imageRectPointsDst[1].y;
			if (tmpBottom > imageRectPointsDst[3].y){
				tmpBottom = imageRectPointsDst[3].y;
			}
			if (bottom > tmpBottom){
				bottom = tmpBottom;
			}
		}

		cout << "\tdraw result" << endl;
		resImage = image1resImage / 2 + image0;

		if (USE_GUI){
			cv::Mat show;
			cv::resize(resImage, show, cv::Size(), 0.5, 0.5);
			cv::imshow(wname, show);
			cv::waitKey();
		}
		else{
			stringstream sstr;
			sstr << setw(5) << setfill('0') << j << "_" << operation_count++  <<"_findHomography_result_" << ".jpg" << flush;
			cv::imwrite(sstr.str(), resImage);

			resImage = image0 / 2 + image1 / 2;
			sstr.str("");
			sstr <<  setw(5) << setfill('0') << j << operation_count++ <<  "_findHomography_original.jpg" << flush;
			cv::imwrite(sstr.str(), resImage);

		}
		transformedImages.push_back(image1resImage);
	}

	cout << "crop result images" << endl;
	cv::Rect internalRect(left,top, right - left, bottom - top);
	cout << internalRect << endl;
	stringstream sstr;
	{
		dst.push_back(cv::Mat(images[0], internalRect));
	}
	for (int i = 0; i < transformedImages.size(); i++){
		dst.push_back( cv::Mat(transformedImages[i], internalRect));
	}
}
