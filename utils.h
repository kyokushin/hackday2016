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
	const int numOfImages = -1, const double scale = 1.0, const std::string& outputPath = "", const bool showFrame=false);
#endif
