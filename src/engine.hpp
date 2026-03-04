#pragma once

#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include <dxrt/dxrt_api.h>
#include "yolov26_postprocess.h"

struct FrameMeta {
    int channel_id;
    int frame_index;
    cv::Mat original_frame;
    int pad_top;
    int pad_left;
    float ratio;
};

class YOLO26Engine {
public:
    YOLO26Engine(const std::string& model_path);
    ~YOLO26Engine();

    dxrt::InferenceEngine* get_ie() const { return ie_; }
    YOLOv26PostProcess* get_ypp() const { return ypp_; }

    void preprocess(const cv::Mat& frame_bgr, std::vector<uint8_t>& input_buffer, FrameMeta& meta);

    int get_input_width() const { return input_width_; }
    int get_input_height() const { return input_height_; }

private:
    void letterbox(const cv::Mat& img, cv::Mat& bordered_img, int& pad_top, int& pad_left, float& r);

    dxrt::InferenceEngine* ie_;
    YOLOv26PostProcess* ypp_;
    
    int input_width_;
    int input_height_;
    float score_threshold_;
};
