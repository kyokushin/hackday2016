#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>

#include <configure.h>

#include "FrameComposer.h"
#include "FrameComposer_orig.h"
#include "superpixel.h"

using namespace std;

const string wname = "test";

const bool USE_GUI = false;
const double IMAGE_SCALE = 0.5;

const int SPX_REGION_SIZE = 10;
const int SPX_MIN_SIZE = 10;

const double SOURCE_IMAGE_SCALE = 0.7;
const int VIDEO_INTERVAL = 15;
const int VIDEO_OUTPUT_FRAME_NUM = 30;

bool endsWith(const std::string& src, const std::string& ends, const bool compareWithLowerCase = false){
	if (src.length() < ends.length()) return false;

	if (compareWithLowerCase){
		string tmpLower = src.substr(src.length() - ends.length());
		transform(
			tmpLower.begin(),
			tmpLower.end(),
			tmpLower.begin(), tolower);
		return tmpLower == ends;
	}
	else {
		return src.substr(src.length() - ends.length()) == ends;
	}
}

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
	cv::imshow(wname, dst);
	cv::waitKey();

	cv::imshow(wname, dstFilled);
	cv::waitKey();
	

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
		cv::imshow(wname, dst);
		cv::waitKey();

	}
}

class ImageFileName : public std::string {

private:
	static int save_counter;
	static const std::string EXT;
	static const std::string PREFIX;
	int savedId;
	
	static void saveTmpFile(ImageFileName& fname, const cv::Mat& image){
		stringstream sstr;
		sstr << PREFIX << setw(8) << setfill('0') << save_counter++ << EXT << flush;
		fname = sstr.str();
		cout << "save file name:" << fname << endl;
		if (!cv::imwrite(fname, image)){
			cerr << "failed to save image:" << fname << endl;
		}
	}

public:
	ImageFileName() : std::string(){
	}
	ImageFileName(const ImageFileName& ifn){
		*this = ifn;
	}
	ImageFileName(const cv::Mat& image){
		saveTmpFile(*this, image);
	}

	ImageFileName(const std::string& str): std::string(str){
	}

	operator cv::Mat() const {
		cv::Mat image = cv::imread(*this);
		if (image.empty()){
			cerr << "failed to read image:" << *this << endl;
		}
		cout << "read image:" << *this << endl;
		return image;
	}

	void operator=(std::string str){
		std::string::operator=(str);
	}


	void operator=(const cv::Mat& image){
		saveTmpFile(*this, image);
	}
};

int ImageFileName::save_counter = 0;
const std::string ImageFileName::EXT = ".jpg";
const std::string ImageFileName::PREFIX = "_ImageFileName";

cv::Scalar randColor(cv::RNG& rng){

	int color = rng();

	return cv::Scalar(color & 0xff, color >> 8 & 0xff, color >> 16 & 0xff);
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

template<typename T>
void videoSplitter(const std::string& fname,
	vector<T>& dst, const int interval=1,
	const int numOfImages=-1, const double scale = 1.0, const std::string& outputPath = ""){

	cv::VideoCapture cap(fname);

	if (!cap.isOpened()){
		cerr << "failed to open video>" << fname << endl;
		return;
	}
	cout << "success open" << endl;

	string path = outputPath;
	if (path.size() > 0){
		//path += PATH_SEPARATOR;
		path += "/";
	}

	cv::Mat frame;
	stringstream sstr;
	int output_counter = 0;
	int interval_counter = 0;
	while (cap.grab()){
		if (numOfImages != -1 && output_counter >= numOfImages){
			return;
		}

		interval_counter++;
		if (interval > interval_counter){
			continue;
		}
		interval_counter = 0;

		cap.retrieve(frame);
		if (USE_GUI){
			cv::Mat show;
			cv::resize(frame, show, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
			cv::imshow(wname, show);
			cv::waitKey();
		}
		sstr.str("");
		sstr << path << "frame_" << setw(5) << setfill('0') << output_counter << ".jpg" << flush;
		string fname = sstr.str();
		cout << "save frame as " << fname << endl;
		if (scale != 1.0){
			cv::Mat small;
			cv::resize(frame, small, cv::Size(), scale, scale);
			cv::imwrite(fname, small);
		}
		else {
			cv::imwrite(fname, frame);
		}
		dst.push_back(fname);
		output_counter++;
	}
}


struct MouseEventParams{
	vector<cv::Mat> srcImages;//antiShaking and crop internal area
	vector<cv::Mat> labeledImages;

	double scale = 1.0;
	cv::Mat selectedMap;
	int currentImageIdx = 0;
	cv::Mat result;
	cv::Mat resultFilled;
	cv::Mat showImage;
	const string wname = "result";

	cv::Mat mask;

	cv::Scalar mouseColor = cv::Scalar(0,0,255);

	bool leftDown = false;
	bool rightDown = false;
	bool working = false;

	void init(vector<cv::Mat>& srcImages, vector<cv::Mat>& labeledImages){
		if (labeledImages.size() == 0){
			cerr << "labeled images is 0" << endl;
			return;
		}

		this->srcImages = srcImages;
		this->labeledImages = labeledImages;

		cv::Mat& image = labeledImages[0];
		selectedMap = cv::Mat(image.rows, image.cols, CV_32SC1);
		result = srcImages[0].clone();
		resultFilled = cv::Mat(image.rows, image.cols, CV_8UC1);
		resultFilled.setTo(0);
	}

};


cv::Mutex mutex;
void onMouseEvent(int event, int x, int y, int flags, void *params){


	MouseEventParams& p= *(MouseEventParams*)params;

	cout << "event:" << event << endl
		<< "x:" << x << endl
		<< "y:" << y << endl
		<< "flags:" << flags << endl
		;

	if (event == 1){
		p.leftDown = true;
	}
	else if (event == 4){
		p.leftDown = false;
	}
	else if (event == 2){
		p.rightDown = true;
	}
	else if (event == 5){
		p.rightDown = false;
	}

	if (p.working || (!p.leftDown && !p.rightDown)){

		cv::resize(p.result, p.showImage, cv::Size(), p.scale, p.scale);
		cv::circle(p.showImage, cv::Point(x, y), 3, p.mouseColor, -1);
		cv::imshow(p.wname, p.showImage);
		cv::waitKey(33);

		cout << "working:" << p.working << endl
			<< "leftDown:" << p.leftDown << endl
			<< "rightDown:" << p.rightDown << endl;
		cout << "\tskip" << endl;
		return;
	}

	cv::AutoLock lock(mutex);

	//event
	//1 : left down
	//5 : left up
	//flags
	//2 : move

	if (p.leftDown){

		p.working = true;
		cout << "\tproc" << endl;

		int realX = x / p.scale;
		int realY = y / p.scale;

		cout << "selected pos:" << realX << "," << realY << endl;
		cout << "current image idx" << p.currentImageIdx << endl;

		cv::Mat& labelMat = p.labeledImages[p.currentImageIdx];
		cv::Mat& src = p.srcImages[p.currentImageIdx];
		int label = labelMat.at<int>(realY%labelMat.rows, realX%labelMat.cols);

		cout << "selected label " << label << endl;

		cv::Mat mask = labelMat == label;
		cout << "set label" << endl;
		p.selectedMap.setTo(p.currentImageIdx * 10000 + label, mask);
		p.resultFilled.setTo(0, mask);
		cout << "copy area to result" << endl;
		src.copyTo(p.result, mask);
	}
	else if (p.rightDown){

	}

	cv::resize(p.result, p.showImage, cv::Size(), p.scale, p.scale);
	cv::imshow(p.wname, p.showImage);
	cv::waitKey(33);

	p.working = false;
}

const int MINIMUM_IMAGE_NUM = 2;

const int MODE_UNDEFINED = -1;
const int MODE_AUTO_OHD3 = 0;
const int MODE_AUTO_OHD4 = 1;
const int MODE_MANUAL_OHD4 = 2;
const int MODE_ONLY_ANTI_SHAKE = 3;

const int USE_UNDEFINED = -1;
const int USE_VIDEO = 0;
const int USE_IMAGE = 1;

int main(int argc, char** argv){

	const string keys =
		"{mode m |manual_ohd4| select from auto_ohd3, auto_ohd4, manual_ohd4 and only_antishake}"
		"{source s|| video file or directory including 1 sequence images }"
		"{lowmemory |false| execute on low memory mode which output images on memory as files}"
		"{help h usage| | show this message}"
		;

	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("");

	if (!parser.check()){
		cout << "in check" << endl;
		parser.printErrors();
		getchar();
		return 0;
	}

	if (parser.has("help")){
		cout << "in help" << endl;
		parser.printMessage();
		getchar();
		return 0;
	}

	int source = USE_UNDEFINED;
	vector<cv::String> sourcePaths;
	if (parser.has("source")){
		string sourcePath = parser.get<string>("source");

		replace(sourcePath.begin(), sourcePath.end(), '\\', '/');

		cv::VideoCapture tmpVideo(sourcePath);
		if (tmpVideo.isOpened()){//confirm source is video
			source = USE_VIDEO;
			tmpVideo.release();

			sourcePaths.push_back(sourcePath);
		}
		else{
			if (!endsWith(sourcePath, "/")){
				sourcePath += '/';
			}
			
			cv::glob(sourcePath += "*.jpg", sourcePaths);
			if (sourcePaths.size() >= MINIMUM_IMAGE_NUM){
				source = USE_IMAGE;
			}
		}
	}

	if (source == USE_UNDEFINED){
		parser.printMessage();
		return 0;
	}

	//*************************
	// parse to use algorithm
	//*************************
	int mode = MODE_UNDEFINED;
	if (parser.has("mode")){
		const string modeStr = parser.get<string>("mode");
		if (modeStr == "auto_ohd3") mode = MODE_AUTO_OHD3;
		else if (modeStr == "auto_ohd4") mode == MODE_AUTO_OHD4;
		else if (modeStr == "manual_ohd4") mode == MODE_MANUAL_OHD4;
		else if (modeStr == "only_antishake") mode == MODE_ONLY_ANTI_SHAKE;
	}
	if(mode == MODE_UNDEFINED){
		parser.printErrors();
		return 0;
	}

	cv::namedWindow(wname);


	vector<ImageFileName> dst;
	if (source == USE_VIDEO){
		cout << "start video split" << endl;
		vector<ImageFileName> fnames;
		videoSplitter(VIDEO_FILE_PATH, fnames, VIDEO_INTERVAL, VIDEO_OUTPUT_FRAME_NUM, SOURCE_IMAGE_SCALE);

		cout << "start anti shake" << endl;
		//vector<cv::Mat> dst;
		//antiShake<cv::Mat>(images, dst);
		//antiShake<ImageFileName, cv::Mat>(fnames, dst);

		antiShake<ImageFileName, ImageFileName>(fnames, dst);
	}
	else if(source == USE_IMAGE){
		ifstream ifs(IMAGES_LIST.c_str());
		string line;
		while (ifs>> line){
			dst.push_back(IMAGES_DIR + line);
		}

		//TODO anti shake algorithm
		
	}
	else {
		CV_Error_(CV_StsBadArg, ("source is not video or images"));
	}

	if(mode == MODE_AUTO_OHD4){

		//OHD4 algorithm
		cout<< "exec super pix" <<endl;
		int regionSize = SPX_REGION_SIZE;
		vector<cv::Mat> resizeDst;
		for(int i=0; i<dst.size(); i++){
			cv::Mat image = dst[i];
			cv::Mat resized;
			cv::resize(image, resized, cv::Size(image.cols/regionSize * regionSize, image.rows/regionSize * regionSize));
			resizeDst.push_back(resized);
			cout<< "\t" << i << ":" << resized.size() <<endl;
		}

		cout << "start super pixel" << endl;
		Slic spx(regionSize);
		vector<cv::Mat> labelImages;
		vector<cv::Mat> contourImages;
		spx.calcSuperPixel(resizeDst, labelImages, contourImages, 10.0f, 10, SPX_MIN_SIZE);

		for (int i = 0; i < contourImages.size(); i++){
			cout << "\tdraw contour" << endl;
			cv::Mat image = resizeDst[i].clone();
			image.setTo( cv::Scalar(0,0,255), contourImages[i]);
			cv::resize(image, image, cv::Size(), IMAGE_SCALE, IMAGE_SCALE);
			cv::imshow(wname, image);
			cv::waitKey();
		}

		cout << "ohd3 algorithm" << endl;
		//OHD3 algorithm
		FrameComposer fcomp;
		vector<string> resizeDst_str;
		for (int i = 0; i < resizeDst.size(); i++){
			stringstream sstr;
			sstr << "spx_result_" << setw(4) << setfill('0') << i << ".jpg" << flush;
			string fname = sstr.str();
			cout << "save:" << fname << endl;
			cv::imwrite( fname, resizeDst[i]);

			resizeDst_str.push_back(fname);
		}
		fcomp.setFile(resizeDst_str);
		cv::Mat fcompRes = fcomp.exec();


		cout << "start combined algorithm" << endl;
		cv::Mat dst;
		ohd3PlusOhd4(fcompRes, labelImages, resizeDst, dst);
		
		cv::imwrite("resulg_ohd3_ohd4.jpg", dst);

	}
	else if (MODE_AUTO_OHD3){
		FrameComposerOrig fcomp;
		vector<string> resultDst;
		for (int i = 0; i < dst.size(); i++){
			resultDst.push_back(dst[i]);
		}
		fcomp.setFile(resultDst);
		cv::Mat res = fcomp.exec();

		cv::imwrite("result_erase_move_object.jpg", res);

	}
	else if(MODE_MANUAL_OHD4){

		cout<< "exec super pix" <<endl;
		int regionSize = 50;
		vector<cv::Mat> resizeDst;
		for(int i=0; i<dst.size(); i++){
			cv::Mat image = dst[i];
			cv::Mat resized;
			cv::resize(image, resized, cv::Size(image.cols/regionSize * regionSize, image.rows/regionSize * regionSize));
			resizeDst.push_back(resized);
			cout<< "\t" << i << ":" << resized.size() <<endl;
		}

		Slic spx(regionSize);
		
		vector<cv::Mat> labelImages;
		vector<cv::Mat> contourImages;
		spx.calcSuperPixel(resizeDst, labelImages, contourImages, 10.0f, 10, 10);

		cout<< "output super pix result" <<endl;
		stringstream sstr;
		for(int i=0; i<labelImages.size(); i++){
			sstr.str("");
			sstr << "labeled_image_" << i << ".jpg" <<flush;
			string fname = sstr.str();
			cout<< "\tlabeled:" << fname <<endl;
			cv::imwrite(fname, contourImages[i]);
		}

		cout << "draw spx result" << endl;
		vector<cv::Mat> spxImages;
		cv::Mat white = resizeDst[0].clone();
		for (int i = 0; i < contourImages.size(); i++){
			cout << "\tdraw contour" << endl;
			cv::Mat image = resizeDst[i].clone();
			image.setTo( cv::Scalar(0,0,255), contourImages[i]);
			sstr.str("");
			sstr << "draw_spx_contour_" << i << ".jpg" << flush;
			cv::imwrite(sstr.str(), image);
			spxImages.push_back(image);
		}

		MouseEventParams params;
		cv::setMouseCallback(wname, onMouseEvent, &params);

		params.currentImageIdx = 0;
		params.init(resizeDst, labelImages);
		params.scale = IMAGE_SCALE;

		cout << "start selecting ui" << endl;
		int key = -1;
		char ch_key = -1;
		cv::Mat showImage;
		while (key != 0x1b){
			if (params.currentImageIdx < 0){
				params.currentImageIdx = 0;
			}
			else if (spxImages.size() <= params.currentImageIdx){
				params.currentImageIdx = spxImages.size() - 1;
			}

			cout << "current image idx " << params.currentImageIdx << endl;

			cv::resize(spxImages[params.currentImageIdx], showImage, cv::Size(), params.scale, params.scale);
			cv::imshow(wname, showImage);
			key = cv::waitKey();
			ch_key = (char)key;

			cout << "key:" << key << endl;

			if (key == 2555904){
				params.currentImageIdx++;
			}
			else if (key == 2424832){
				params.currentImageIdx--;
			}
			else if (ch_key == 'i'){
				cout << "show result" << endl;
				cv::imshow(params.wname, params.result);
				cv::waitKey();
				cout << "show filled" << endl;
				cv::imshow(params.wname, params.resultFilled);
				cv::waitKey();

				cout << "proc inpaint" << endl;
				cv::Mat result;
				cv::inpaint(params.result, params.resultFilled, result, 10, CV_INPAINT_NS);
				cv::imshow(params.wname, result);
				cv::waitKey();
				cv::imwrite("inpainting_result.jpg", result);
			}
		}

	}
	else{
		CV_Error(CV_StsBadArg, "selecting algorithm is not found");
	}

	cout<< "end program" <<endl;

}
