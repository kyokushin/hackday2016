#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>

#include "FrameComposer.h"
#include "FrameComposer_orig.h"
#include "superpixel.h"
#include "antishake.h"

#include "utils.h"
#include "options.h"


using namespace std;

const string wname = "debug";

/*
const double IMAGE_SCALE = 0.5;

const int SPX_REGION_SIZE = 10;
const int SPX_MIN_SIZE = 10;

const double SOURCE_IMAGE_SCALE = 0.7;
const int VIDEO_INTERVAL = 15;
const int VIDEO_OUTPUT_FRAME_NUM = 30;
*/



struct MouseEventParams{
	vector<cv::Mat> srcImages;//antiShaking and crop internal area
	vector<cv::Mat> labeledImages;

	double scale;
	cv::Mat selectedMap;
	int currentImageIdx ;
	cv::Mat result;
	cv::Mat resultFilled;
	cv::Mat showImage;
	const string wname;

	cv::Mat mask;

	cv::Scalar mouseColor;

	bool leftDown ;
	bool rightDown ;
	bool working;

	MouseEventParams()
		:scale(1.0), currentImageIdx(0), wname("result"), mouseColor(cv::Scalar(0,0,255)),
		leftDown(false), rightDown(false), working(false)
	{}

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


int opencvErrorHandler(int status, const char* func_name, const char* err_msg, const char* file_name, int line, void* userdata){
	return 0;
}

void showGuiHelp(){
	cout << "usage" << endl
		<< "\t<-(left arrow) | change using frame index" << endl
		<< "\t->(right arrow) | change using frame index" << endl
		<< "\ti | inpaint blank space(experimental)" << endl
		<< "\ts | save result window image" <<endl
		<< endl;
}


int main(int argc, char** argv){

#ifndef _DEBUG
	cv::redirectError(opencvErrorHandler);
#endif

	const string keys =
		"{mode m |manual_ohd4| select from auto_ohd3, auto_ohd4, manual_ohd4 and only_antishake}"
		"{@source| | video file or directory including 1 sequence images }"
		"{display_scale|0.5| display image size in manual_ohd4}"
		"{spx_region_size| 10 | super pixel processing size}"
		"{spx_min_size| 10 | if super pixel smaller than this value, combine other super pixel}"
		"{resize_scale| 0.7 | resize processed image scale}"
		"{video_interval| 15 | grab flame by this interval}"
		"{frame_num | 30 | processed num of images}"
		"{intermediate_images | nothing | show or save or nothing intermediate images}"
		"{help h usage| | show this message}"
		;

	cv::CommandLineParser parser(argc, argv, keys);
	//parser.about("");
	cout << "parse created" << endl;


	if (parser.has("help")){
		cout << "in help" << endl;
		parser.printMessage();
		getchar();
		return 0;
	}

	int source = USE_UNDEFINED;
	vector<cv::String> sourcePaths;
	cout << "source:" << parser.get<string>(0) << endl;
	if ( parser.get<string>(0).size() > 0){
	//if (parser.has("source") && parser.get<string>("source") != "true"){
		string sourcePath = parser.get<string>(0);

		replace(sourcePath.begin(), sourcePath.end(), '\\', '/');

		try{
			cv::VideoCapture tmpVideo(sourcePath);
			if (tmpVideo.isOpened()){//confirm source is video
				source = USE_VIDEO;
				tmpVideo.release();
				sourcePaths.push_back(sourcePath);
				cout << "source is video" << endl;
			}
		}
		catch (...){
		}
		if (source == USE_UNDEFINED){
			try{
				if (!ysutils::endsWith(sourcePath, "/")){
					sourcePath += '/';
				}

				cv::glob(sourcePath += "*.jpg", sourcePaths);
				if (sourcePaths.size() >= MINIMUM_IMAGE_NUM){
					source = USE_IMAGE;
					cout << "source is images" << endl;
				}
			}
			catch (...){}
		}
	}
	if (source == USE_UNDEFINED){
		cerr << "failed to open source :" << parser.get<string>(0) << endl;
		parser.printMessage();
		return 0;
	}

	//*************************
	// parse using algorithm
	//*************************
	int mode = MODE_UNDEFINED;
	const string modeStr = parser.get<string>("mode");
	cout << "mode:" << modeStr << endl;
	if (modeStr == "auto_ohd3") mode = MODE_AUTO_OHD3;
	else if (modeStr == "auto_ohd4") mode = MODE_AUTO_OHD4;
	else if (modeStr == "manual_ohd4") mode = MODE_MANUAL_OHD4;
	else if (modeStr == "only_antishake"){
		mode = MODE_ONLY_ANTI_SHAKE;
		cerr << "sorry. this function not implemented yet." << endl;
		return -1;
	}
	if(mode == MODE_UNDEFINED){
		cerr << "unsupported mode : " << modeStr << endl;
		parser.printMessage();
		return 0;
	}

	cv::namedWindow(wname);

	//**************************
	// parse source parameters 
	//**************************
	const int videoInterval = parser.get<int>("video_interval");
	const int frameNum = parser.get<int>("frame_num");
	const double resizeScale = parser.get<double>("resize_scale"); 

	//************************
	// parse other parameter
	//************************
	const double displayScale = parser.get<double>("display_scale");
	const int spxRegionSize = parser.get<int>("spx_region_size");
	const int spxMinSize = parser.get<int>("spx_min_size");
	const string intermediateMethodStr = parser.get<string>("intermediate_images");
	int intermediateMethod;
	if (intermediateMethodStr == "nothing"){
		intermediateMethod = INTERMEDIATE_NOTHING;
	}
	else if (intermediateMethodStr == "show"){
		intermediateMethod = INTERMEDIATE_SHOW;
	}
	else if (intermediateMethodStr == "save"){
		intermediateMethod = INTERMEDIATE_SAVE;
	}
	else {
		cerr << "undefined method:" << parser.get<string>("intermediate_images");
	}

	if (!parser.check()){
		cout << "in check" << endl;
		parser.printErrors();
		getchar();
		return 0;
	}

	//*********************
	// parse used source
	//*********************
	vector<ysutils::ImageFileName> dst;
	if (source == USE_VIDEO){
		cout << "start video split" << sourcePaths[0] << endl;
		vector<ysutils::ImageFileName> fnames;
		videoSplitter(sourcePaths[0], fnames, videoInterval, frameNum, resizeScale);

		cout << "start anti shake" << endl;
		//vector<cv::Mat> dst;
		//antiShake<cv::Mat>(images, dst);
		//antiShake<ysutils::ImageFileName, cv::Mat>(fnames, dst);

		antishake::antiShake<ysutils::ImageFileName, ysutils::ImageFileName>(fnames, dst, intermediateMethod, displayScale);
	}
	else if(source == USE_IMAGE){
		//TODO anti shake algorithm
		
		vector<ysutils::ImageFileName> fnames;
		for (int i = 0; i < sourcePaths.size(); i++){
			fnames.push_back((string)sourcePaths[i]);
		}
		antishake::antiShake<ysutils::ImageFileName, ysutils::ImageFileName>(fnames, dst, intermediateMethod, displayScale);
	}
	else {
		cerr << "processed source is unrecognized" << endl;
		parser.printMessage();
		return 0;
	}

	if(mode == MODE_AUTO_OHD4){

		//OHD4 algorithm
		cout<< "exec super pix" <<endl;
		vector<cv::Mat> resizeDst;
		for(int i=0; i<dst.size(); i++){
			cv::Mat image = dst[i];
			cv::Mat resized;
			cv::resize(image, resized, cv::Size(image.cols/spxRegionSize* spxRegionSize, image.rows/spxRegionSize* spxRegionSize));
			resizeDst.push_back(resized);
			cout<< "\t" << i << ":" << resized.size() <<endl;
		}

		cout << "start super pixel" << endl;
		Slic spx(spxRegionSize);
		vector<cv::Mat> labelImages;
		vector<cv::Mat> contourImages;
		spx.calcSuperPixel(resizeDst, labelImages, contourImages, 10.0f, 10, spxMinSize);

		for (int i = 0; i < contourImages.size(); i++){
			cout << "\tdraw contour" << endl;
			cv::Mat image = resizeDst[i].clone();
			image.setTo( cv::Scalar(0,0,255), contourImages[i]);
			cv::resize(image, image, cv::Size(), displayScale, displayScale);
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
		antishake::ohd3PlusOhd4(fcompRes, labelImages, resizeDst, dst);
		
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
		vector<cv::Mat> spxImages;
		spx.calcSuperPixel(resizeDst, labelImages, contourImages, 10.0f, 10, 10);

		if (INTERMEDIATE_SAVE == intermediateMethod){
			cout << "output super pix result" << endl;
			stringstream sstr;
			for (int i = 0; i < labelImages.size(); i++){
				sstr.str("");
				sstr << "labeled_image_" << i << ".jpg" << flush;
				string fname = sstr.str();
				cout << "\tlabeled:" << fname << endl;
				cv::imwrite(fname, contourImages[i]);
			}
		}

		cout << "draw spx result" << endl;
		for (int i = 0; i < contourImages.size(); i++){
			cout << "\tdraw contour" << endl;
			cv::Mat image = resizeDst[i].clone();
			image.setTo( cv::Scalar(0,0,255), contourImages[i]);
			if (INTERMEDIATE_SAVE == intermediateMethod){
				stringstream sstr;
				sstr << "draw_spx_contour_" << i << ".jpg" << flush;
				cv::imwrite(sstr.str(), image);
			}
			spxImages.push_back(image);
		}

		MouseEventParams params;
		cv::setMouseCallback(wname, onMouseEvent, &params);

		params.currentImageIdx = 0;
		params.init(resizeDst, labelImages);
		params.scale = displayScale;

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

			if (ch_key == 'h'){
				showGuiHelp();
			}
			else if (key == 2555904){
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
			else if (ch_key == 's'){
				cout << "save result" << endl;
				cv::imwrite("result.jpg", params.result);
			}
		}

	}
	else{
		CV_Error(CV_StsBadArg, "selecting algorithm is not found");
	}

	cout<< "end program" <<endl;

}
