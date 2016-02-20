#ifndef antishake_h
#define antishake_h

#include <opencv2/opencv.hpp>
#include <vector>

int max(const cv::Mat& mat);

void ohd3PlusOhd4(const cv::Mat& frameCompRes,
	const std::vector<cv::Mat>& labelImages,
	const std::vector<cv::Mat>& srcImages,
	cv::Mat& dst);

template<typename T, typename S>
void antiShake(const std::vector<T>& images, std::vector<S>& dst, const bool use_gui=false){
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
	if (use_gui){
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
		if (use_gui){
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

		if (use_gui){
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

		if (use_gui){
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

		if (use_gui){
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

		if (use_gui){
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

#endif
