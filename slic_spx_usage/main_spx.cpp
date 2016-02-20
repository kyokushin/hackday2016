/*
* test_slic.cpp.
*
* Written by: Pascal Mettes.
*
* This file creates an over-segmentation of a provided image based on the SLIC
* superpixel algorithm, as implemented in slic.h and slic.cpp.
*/

#include "superpixel.h"
#include <filesystem>

int main(int argc, char *argv[]) {
	std::vector<std::string> files;
	//files.push_back("./images/_D8E8402.JPG");
	//files.push_back("./images/_D8E8403.JPG");
	//files.push_back("./images/_D8E8404.JPG");
	//files.push_back("./images/_D8E8405.JPG");
	files.push_back("./images/_D8E8401.JPG");

	std::vector<cv::Mat> src_images;
	for (std::string file : files) {
		cv::Mat image = cv::imread(file, 0);
		src_images.push_back(image);
	}
	files.clear();

	std::vector<cv::Mat> label_images;
	Slic* slic = new Slic(50);
	slic->calcSuperPixel(src_images, label_images);

	return 0;
}
