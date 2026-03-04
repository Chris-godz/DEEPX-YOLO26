#include "engine.hpp"
#include <iostream>

YOLO26Engine::YOLO26Engine(const std::string& model_path) {
    ie_ = new dxrt::InferenceEngine(model_path);
    auto input_info = ie_->GetInputs();
    
    // NCHW or NHWC abstraction (YOLO typically NHWC 1xHxWxC or NCHW 1xCxHxW)
    if (input_info[0].shape().size() == 4) {
        if (input_info[0].shape()[1] == 3) {
            // NCHW
            input_height_ = input_info[0].shape()[2];
            input_width_ = input_info[0].shape()[3];
        } else {
            // NHWC
            input_height_ = input_info[0].shape()[1];
            input_width_ = input_info[0].shape()[2];
        }
    } else {
        input_height_ = 640; // Default fallback
        input_width_ = 640;
    }

    score_threshold_ = 0.4f;
    ypp_ = new YOLOv26PostProcess(input_width_, input_height_, score_threshold_, ie_->IsOrtConfigured());
}

YOLO26Engine::~YOLO26Engine() {
    delete ypp_;
    delete ie_;
}

void YOLO26Engine::letterbox(const cv::Mat& img, cv::Mat& bordered_img, int& pad_top, int& pad_left, float& r) {
    int h = img.rows;
    int w = img.cols;
    
    double r_double = std::min((double)input_height_ / h, (double)input_width_ / w);
    r = r_double;
    int new_unpad_w = std::rint(w * r_double);
    int new_unpad_h = std::rint(h * r_double);
    
    int dw = input_width_ - new_unpad_w;
    int dh = input_height_ - new_unpad_h;
    
    pad_left = dw / 2;
    pad_top = dh / 2;
    int pad_right = dw - pad_left;
    int pad_bottom = dh - pad_top;

    cv::Mat resized_img;
    if (w != new_unpad_w || h != new_unpad_h) {
        cv::resize(img, resized_img, cv::Size(new_unpad_w, new_unpad_h), 0, 0, cv::INTER_LINEAR);
    } else {
        resized_img = img;
    }

    cv::copyMakeBorder(resized_img, bordered_img, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));
}

void YOLO26Engine::preprocess(const cv::Mat& frame_bgr, std::vector<uint8_t>& input_buffer, FrameMeta& meta) {
    cv::Mat bordered_img;
    letterbox(frame_bgr, bordered_img, meta.pad_top, meta.pad_left, meta.ratio);
    
    // Color conversion
    cv::Mat rgb_img;
    cv::cvtColor(bordered_img, rgb_img, cv::COLOR_BGR2RGB);

    auto input_info = ie_->GetInputs();
    
    std::vector<uint8_t> buffer(rgb_img.data, rgb_img.data + rgb_img.total() * rgb_img.elemSize());
    
    input_buffer = buffer;
}
