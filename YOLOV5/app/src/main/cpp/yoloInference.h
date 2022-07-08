//
// Created by WIN on 2022/7/5.
//

#ifndef YOLOV5_YOLOINFERENCE_H
#define YOLOV5_YOLOINFERENCE_H

#include<opencv2/opencv.hpp>
#include<opencv2/dnn/dnn.hpp>
#include<MNN/Interpreter.hpp>
#include<MNN/Tensor.hpp>
#include<MNN/MNNDefine.h>
#include <sstream>
#include <iomanip>
#include <iostream>

//int detection(const char *files, cv::Mat &imgfile, std::vector<std::vector<float>> &results);
void detection(const char *files, cv::Mat imgfile,std::vector<std::vector<float>> &results);
void runSession(std::shared_ptr<MNN::Interpreter> model, MNN::Session *session, cv::Mat &image,
                cv::Mat &org_image, float scaleh,float scalew, std::vector<std::vector<float>> &results);
#endif //YOLOV5_YOLOINFERENCE_H



