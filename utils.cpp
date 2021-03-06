﻿#include "utils.h"

#include <string>
#include <sstream>
#include <iomanip>

#include <algorithm>

using namespace std;


bool ysutils::endsWith(const std::string& src, const std::string& ends, const bool compareWithLowerCase ){
	if (src.length() < ends.length()) return false;

	if (compareWithLowerCase){
		string tmpLower = src.substr(src.length() - ends.length());
		transform(
			tmpLower.begin(),
			tmpLower.end(),
			tmpLower.begin(), ::tolower);
		return tmpLower == ends;
	}
	else {
		return src.substr(src.length() - ends.length()) == ends;
	}
}




void ysutils::ImageFileName::saveTmpFile(ImageFileName& fname, const cv::Mat& image){
	stringstream sstr;
	sstr << PREFIX << setw(8) << setfill('0') << save_counter++ << EXT << flush;
	fname = sstr.str();
	cout << "save file name:" << fname << endl;
	if (!cv::imwrite(fname, image)){
		cerr << "failed to save image:" << fname << endl;
	}
}


ysutils::ImageFileName::operator cv::Mat() const {
	cv::Mat image = cv::imread(*this);
	if (image.empty()){
		cerr << "failed to read image:" << *this << endl;
	}
	cout << "read image:" << *this << endl;
	return image;
}

int ysutils::ImageFileName::save_counter = 0;
const std::string ysutils::ImageFileName::EXT = ".jpg";
const std::string ysutils::ImageFileName::PREFIX = "_ImageFileName";

cv::Scalar ysutils::randColor(cv::RNG& rng){

	int color = rng();

	return cv::Scalar(color & 0xff, color >> 8 & 0xff, color >> 16 & 0xff);
}

