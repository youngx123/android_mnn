//
// Created by WIN on 2022/7/5.
//

#include "yoloInference.h"
#include <MNN/ImageProcess.hpp>
#include <sstream>
#include <iomanip>

void detection(const char *files, cv::Mat imgfile, std::vector<std::vector<float>> &results)
{
    int INPUT_SIZE = 640;
    int forward = MNN_FORWARD_CPU;
    int precision = 0;
    int power = 0;
    int memory = 0;
    int threads = 1;
    //    MNN::Session *session;

    cv::Mat inputImg = imgfile.clone();
    cv::cvtColor(inputImg, inputImg, cv::COLOR_RGBA2RGB);
    cv::Mat image;
    // 图像预处理
    int height = inputImg.rows;
    int width = inputImg.cols;

    float scaleW = (float)width / INPUT_SIZE;
    float scaleH = (float)height / INPUT_SIZE;
    cv::resize(inputImg, image, cv::Size(INPUT_SIZE, INPUT_SIZE));
//    image.convertTo(image, CV_32FC3);
//    image = image / 255.0f;

    // set mnn model parameters
    // load mnn model
    std::shared_ptr<MNN::Interpreter> model(MNN::Interpreter::createFromFile(files));
    MNN::ScheduleConfig config;
    config.numThread = threads;
    config.type = static_cast<MNNForwardType>(forward);

//    // memory、power、precision分别为内存、功耗和精度偏好
    MNN::BackendConfig backendConfig;
    backendConfig.precision = (MNN::BackendConfig::PrecisionMode)precision;
    backendConfig.power = (MNN::BackendConfig::PowerMode) power;
    backendConfig.memory = (MNN::BackendConfig::MemoryMode) memory;

    config.backendConfig = &backendConfig;

    auto session = model->createSession(config);
    model->releaseModel();

    //    // DATA process
    MNN::Tensor *input_tensor = model->getSessionInput(session, nullptr);
    // pass image to model(rgb format, pixels minus by 0 and then divided by 255.0)
    const float mean_vals[3] = { 0.0, 0.0, 0.0 };
    const float norm_vals[3] = {1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0 };
    std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(MNN::CV::BGR, MNN::CV::RGB, mean_vals, 3, norm_vals, 3));
    pretreat->convert(image.data, INPUT_SIZE, INPUT_SIZE,INPUT_SIZE*3, input_tensor);

    auto tiem1 = std::chrono::high_resolution_clock::now();

//    std::vector<std::vector<float>> nms_result;
    runSession(model,session, image, inputImg, scaleH, scaleW, results);
    int res = results.size();
//    return res;


}

void runSession(std::shared_ptr<MNN::Interpreter> model, MNN::Session *session, cv::Mat &image, cv::Mat &org_image,
                float scaleh,float scalew, std::vector<std::vector<float>> &results)
{

//    int dynamic_height = image.rows;
//    int dynamic_width = image.cols;
//    std::vector<int> dims{ 1, dynamic_height, dynamic_width, 3 };
//    auto nhwc_Tensor = MNN::Tensor::create<float>(dims, NULL, MNN::Tensor::TENSORFLOW);
//
//    auto nhwc_data = nhwc_Tensor->host<float>();
//    auto nhwc_size = nhwc_Tensor->size();
//    std::cout << image.rows << " " << image.cols<<" "<<image.channels() << std::endl;
//    ::memcpy(nhwc_data, image.data, nhwc_size);

    std::string input_node = "images";
    std::string output_node = "output";

    // 数据预处理
    int INPUT_SIZE2 = image.rows;
    MNN::Tensor *input_tensor = model->getSessionInput(session, input_node.c_str());
    // pass image to model(rgb format, pixels minus by 0 and then divided by 255.0)
    const float mean_vals[3] = { 0.0, 0.0, 0.0 };
    const float norm_vals[3] = {1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0 };
    std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(MNN::CV::BGR, MNN::CV::RGB, mean_vals, 3, norm_vals, 3));
    pretreat->convert(image.data, INPUT_SIZE2, INPUT_SIZE2,INPUT_SIZE2*3, input_tensor);

    // 6. 运行会话
    model->runSession(session);

    // 7. 获取输出
    auto tensor_scores = model->getSessionOutput(session, output_node.c_str());
    std::cout << "output shape: \n";
    std::cout<< tensor_scores->shape()[0] << " "
             << tensor_scores->shape()[1] <<" " << tensor_scores->shape()[2] << std::endl;

    int category_num = tensor_scores->shape()[2];
    int bboxes_num = tensor_scores->shape()[1];

    // convert pred to array
    float* output_array = tensor_scores->host<float>();

    // post processing
    std::vector<float> output_vector_boxes{ output_array, output_array + 25200*85};
    std::cout<<output_vector_boxes.size();
    std::vector<cv::Rect> bboxs_total;
    std::vector<float> bboxs_score;
    std::vector<int> bboxs_category;
    for (int i = 0; i < bboxes_num-1; i++)
    {
        std::vector<float>::const_iterator firstConfs = output_vector_boxes.begin() + i * category_num;
        std::vector<float>::const_iterator lastConfs = output_vector_boxes.begin() + (i+1) * category_num;
        std::vector<float> prob_vector(firstConfs, lastConfs);
        float obj_score = prob_vector.at(4);
        float category_max_prob = 0;
        float category_index = 0;

        if (obj_score > 0.25)
        {
            for (int j = 5; j < category_num; j++)
            {
                if (prob_vector[j] > category_max_prob)
                {
                    category_max_prob = prob_vector[j];
                    category_index = j - 5;
                }
            }

            if (category_max_prob > 0.15)
            {
                std::vector<float> items;
                float x = prob_vector.at(0) * scalew;
                float y = prob_vector.at(1) * scaleh;
                float w = prob_vector.at(2) * scalew;
                float h = prob_vector.at(3) * scaleh;

                float x1 = x - w / 2;
                float y1 = y - h / 2;

                cv::Rect rect(x1, y1, w, h);
                bboxs_total.push_back(rect);
                bboxs_score.push_back(category_max_prob);
                bboxs_category.push_back(category_index);
            }
        }

    }

    // MNS result
    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(bboxs_total, bboxs_score, 0.2, 0.3, nms_result);

    std::cout << nms_result.size() << std::endl;
    // to origin image size
    float gain = MIN((float)image.rows / org_image.rows, (float)image.cols / org_image.cols);
    int pad1 = (image.rows - org_image.rows * gain) / 2;
    int pad2 = (image.cols - org_image.cols * gain) / 2;
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision(3);
    for(int index : nms_result)
    {
        cv::Rect rect = bboxs_total.at(index);
        float score = (bboxs_score.at(index));
        ss << score;
        score = std::stof(ss.str());
        float id = bboxs_category[index];
        std::vector<float> items{(float)rect.x, (float)rect.y, (float)(rect.x+rect.width), (float)(rect.y+rect.height), (float)(score), id};
        results.push_back(items);
    }
}