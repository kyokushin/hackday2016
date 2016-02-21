/***********************************************
* OpenHackDay3で使用したアルゴリズムのオリジナル版
*/
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

struct FrameComposerOrig{

	int m_width, m_height, m_nImgs;
	std::vector<std::string> m_filenames;
    FrameComposerOrig(){
    }
	void setFile(std::vector<std::string>& filenames){

		m_filenames = filenames;
		m_nImgs = filenames.size();
	}

	cv::Mat exec(){

		std::vector<cv::Mat> images;
		for (int i = 0; i < (int)m_filenames.size(); ++i) {
			cv::Mat img = cv::imread(m_filenames[i], 0);
			images.push_back(img);
		}
		cv::Mat temp = images[0];
		int img_cols = temp.cols;
		int img_rows = temp.rows;
		int pixels = img_cols * img_rows;
		cv::Mat dst = cv::Mat::zeros(temp.size(), CV_8UC1);

		int image_num = 1;      // ���͉摜�̖���
		int channels[] = { 0 }; // cv::Mat�̉��Ԗڂ̃`���l�����g�����@����͔����摜�Ȃ̂�0�Ԗڂ̃`���l���ȊO�I�����Ȃ�
		cv::Mat hist;         // �����Ƀq�X�g�O�������o�͂����
		int dim_num = 1;        // �q�X�g�O�����̎�����
		int bin_num = 64;       // �q�X�g�O�����̃r���̐�
		int bin_nums[] = { bin_num };      // �����1�����̃q�X�g�O���������̂ŗv�f���͈��
		float range[] = { 0, 256 };        // �����f�[�^�̍ŏ��l�A�ő�l�@����͋P�x�f�[�^�Ȃ̂Œl���[0, 255]
		const float *ranges[] = { range };
		int bin_interval = 256 / bin_num;

		for (int i = 0; i < pixels; i++) {
			cv::Mat frames = cv::Mat::zeros(images.size(), 1, CV_8U);
			int x = i % img_cols;
			int y = i / img_cols;

			for (int j = 0; j < (int)images.size(); ++j) {
				//frames.at<unsigned char>(j, 0) = images[j].at<unsigned char>(y, x);
				frames.data[j] = images[j].ptr(y)[x];
			}

			cv::calcHist(&frames, image_num, channels, cv::Mat(), hist, dim_num, bin_nums, ranges);

		// hist�̍ő�bin�����߂�
			double max_val = 0.0;
			int bin_idx = 0;
			for (int j = 0; j < hist.rows; ++j) {
				//double val = hist.at<float>(j, 0);
				double val = *((float*)hist.data + j);
				if (val > max_val){
					max_val = val;
					bin_idx = j;
				}
			}

		// bin���܂܂�錳�摜�̃C���f�b�N�X��Ԃ�
			unsigned char bin_val = (unsigned char)(bin_idx * bin_interval);
			for (int j = 0; j < frames.rows; ++j){
				//unsigned char val = frames.at<unsigned char>(j, 0);
				unsigned char val = frames.ptr(j)[0];
				if (val >= bin_val && val < bin_val + bin_interval) {
					//dst.at<unsigned char>(y, x) = (unsigned char)j;
					dst.ptr(y)[x] = (unsigned char)j;
					break;
				}
			}
		}
		// �Y�����錳�摜�����f�l���擾����
		cv::Mat result = cv::Mat::zeros(temp.rows, temp.cols, CV_8UC3);
		for (int i = 0; i < m_nImgs; ++i) {
			cv::Mat img = cv::imread(m_filenames[i]);
			//cv::Mat img = images[i];
			for (int j = 0; j < pixels; ++j) {
    			int x = j % img_cols;
			    int y = j / img_cols;
		    	//int idx = (int)dst.at<unsigned char>(y, x);
			    int idx = (int)dst.ptr(y)[x];
			    if (i == idx){
				    //result.at<cv::Vec3b>(y, x) = img.at<cv::Vec3b>(y, x);
				    unsigned char *resultPix = result.data + y * 3 * result.cols + 3 * x;
				    unsigned char *imgPix = img.data + y * 3 * img.cols + 3 * x;

				    resultPix[0] = imgPix[0];
				    resultPix[1] = imgPix[1];
				    resultPix[2] = imgPix[2];
			    }
			}
		}

		return result;
	}

};