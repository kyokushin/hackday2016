#include "superpixel.h"
#include "opencv2/ximgproc/slic.hpp"

#include <iostream>

using namespace std;

Slic::Slic(int region_size) : m_region_size(region_size){
} 

Slic::~Slic() {
}

void Slic::calcSuperPixel(const std::vector<cv::Mat>& src_images, std::vector<cv::Mat>& label_images, std::vector<cv::Mat>& contour_images, const float ruler, const int num_iterations, const int min_element_size) {

	cout<< "proc super pixel" <<endl;
	for (int i=0; i<src_images.size(); i++) {
		cout<< "\tcurrent:" << i << endl;
		const cv::Mat& image = src_images[i];
		cv::Ptr<cv::ximgproc::SuperpixelSLIC> pslic = cv::ximgproc::createSuperpixelSLIC(image, cv::ximgproc::SLIC, m_region_size, ruler);

		pslic->iterate(num_iterations);

		pslic->enforceLabelConnectivity(min_element_size);

		cv::Mat label_image;
		pslic->getLabels(label_image);
		label_images.push_back(label_image);

		cv::Mat contour_image;
		pslic->getLabelContourMask(contour_image);
		contour_images.push_back(contour_image);
	}
	cout<< "end super pix" << endl;
}
