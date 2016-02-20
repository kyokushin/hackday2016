#include "antishake.h"

#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;

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

#ifdef _DEBUG
	const std::string wname("debug");
#endif

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

#ifdef _DEBUG
	cv::imshow(wname, dst);
	cv::waitKey();

	cv::imshow(wname, dstFilled);
	cv::waitKey();
#endif
	

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
#ifdef _DEBUG
		cv::imshow(wname, dst);
		cv::waitKey();
#endif

	}
}

