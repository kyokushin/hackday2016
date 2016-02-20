#include "utils.h"

#include <string>
#include <sstream>
#include <iomanip>

using namespace std;


bool endsWith(const std::string& src, const std::string& ends, const bool compareWithLowerCase ){
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




void ImageFileName::saveTmpFile(ImageFileName& fname, const cv::Mat& image){
	stringstream sstr;
	sstr << PREFIX << setw(8) << setfill('0') << save_counter++ << EXT << flush;
	fname = sstr.str();
	cout << "save file name:" << fname << endl;
	if (!cv::imwrite(fname, image)){
		cerr << "failed to save image:" << fname << endl;
	}
}


ImageFileName::operator cv::Mat() const {
	cv::Mat image = cv::imread(*this);
	if (image.empty()){
		cerr << "failed to read image:" << *this << endl;
	}
	cout << "read image:" << *this << endl;
	return image;
}

int ImageFileName::save_counter = 0;
const std::string ImageFileName::EXT = ".jpg";
const std::string ImageFileName::PREFIX = "_ImageFileName";

cv::Scalar randColor(cv::RNG& rng){

	int color = rng();

	return cv::Scalar(color & 0xff, color >> 8 & 0xff, color >> 16 & 0xff);
}

template<typename T>
void videoSplitter(const std::string& fname,
	vector<T>& dst, const int interval=1,
	const int numOfImages, const double scale, const std::string& outputPath, const bool showFrame){

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
		if (showFrame){
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
