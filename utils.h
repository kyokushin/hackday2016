#ifndef utils_h
#define utils_h

#include <string>
#include<opencv2/opencv.hpp>
#include <vector>


bool endsWith(const std::string& src, const std::string& ends, const bool compareWithLowerCase = false);


class ImageFileName : public std::string {

private:
	static int save_counter;
	static const std::string EXT;
	static const std::string PREFIX;
	int savedId;
	
	static void saveTmpFile(ImageFileName& fname, const cv::Mat& image);

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

	operator cv::Mat() const;

	void operator=(std::string str){
		std::string::operator=(str);
	}


	void operator=(const cv::Mat& image){
		saveTmpFile(*this, image);
	}
};

cv::Scalar randColor(cv::RNG& rng);


template<typename T>
void videoSplitter(const std::string& fname,
	std::vector<T>& dst, const int interval = 1,
	const int numOfImages = -1, const double scale = 1.0, const std::string& outputPath = "", const bool showFrame = false){

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
#endif
