#ifndef SUPERPIXEL_H
#define SUPERPIXEL_H

#include "opencv2/opencv.hpp"
#include <vector>

class Slic{
	private:
		int m_region_size;

	public:
		Slic(int region_size);
		~Slic();

		void calcSuperPixel(
			const std::vector<cv::Mat>& src_images,
			std::vector<cv::Mat>& label_images,
			std::vector<cv::Mat>& contour_images,
			const float ruler = 10.0f,
			const int num_iterations = 10,
			const int min_element_size = 25);
};

#endif
