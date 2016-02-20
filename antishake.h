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
void antiShake(const std::vector<T>& images, std::vector<S>& dst);

#endif
