#pragma once

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>
#include "safe_queue.hpp"
#include "engine.hpp"
#include "config.hpp"

struct DrawTask {
    FrameMeta meta;
    std::vector<YOLOv26Result> detections;
};

class Workers {
public:
    void set_active_classes(const std::vector<bool>& active);
    Workers(const std::string& model_path, const std::vector<ChannelConfig>& channels);
    ~Workers();

    void start();
    void stop();

    bool get_latest_frame(int channel_id, cv::Mat& out_frame);

private:
    void capture_thread(int channel_id, const ChannelConfig& config);
    void preprocess_thread();
    void Wait_thread();
    void draw_thread();

    std::string model_path_;
    std::vector<ChannelConfig> channels_;
    std::vector<bool> active_classes_;
    std::mutex active_mutex_;
    std::unique_ptr<YOLO26Engine> engine_;
    
    std::atomic<bool> running_;

    // Queues
    SafeQueue<FrameMeta> pre_queue_;
    SafeQueue<int> Wait_queue_; // Using request_id or directly storing vectors
    SafeQueue<DrawTask> draw_queue_;

    // Thread management
    std::vector<std::thread> cap_threads_;
    std::thread pre_thread_;
    std::thread Wait_thread__;
    std::thread draw_thread__;

    // Display state
    std::mutex display_mutex_;
    std::vector<cv::Mat> latest_frames_;
};
