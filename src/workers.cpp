#include "workers.hpp"
#include <iostream>

extern const std::vector<std::string> COCO_CLASS_NAMES = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

void Workers::set_active_classes(const std::vector<bool>& active) {
    std::lock_guard<std::mutex> lock(active_mutex_);
    active_classes_ = active;
}

Workers::Workers(const std::string& model_path, const std::vector<ChannelConfig>& channels)
    : model_path_(model_path), channels_(channels), running_(false) {
    engine_ = std::make_unique<YOLO26Engine>(model_path_);
    latest_frames_.resize(channels.size());
    active_classes_.resize(COCO_CLASS_NAMES.size(), true);
}

Workers::~Workers() {
    stop();
}

void Workers::start() {
    running_ = true;

    for (size_t i = 0; i < channels_.size(); ++i) {
        cap_threads_.emplace_back(&Workers::capture_thread, this, i, channels_[i]);
    }
    
    pre_thread_ = std::thread(&Workers::preprocess_thread, this);
    Wait_thread__ = std::thread(&Workers::Wait_thread, this);
    draw_thread__ = std::thread(&Workers::draw_thread, this);
}

void Workers::stop() {
    running_ = false;
    for (auto& t : cap_threads_) {
        if (t.joinable()) t.join();
    }
    if (pre_thread_.joinable()) pre_thread_.join();
    if (Wait_thread__.joinable()) Wait_thread__.join();
    if (draw_thread__.joinable()) draw_thread__.join();
}

bool Workers::get_latest_frame(int channel_id, cv::Mat& out_frame) {
    std::lock_guard<std::mutex> lock(display_mutex_);
    if (channel_id >= 0 && channel_id < static_cast<int>(latest_frames_.size()) && !latest_frames_[channel_id].empty()) {
        latest_frames_[channel_id].copyTo(out_frame);
        return true;
    }
    return false;
}

void Workers::capture_thread(int channel_id, const ChannelConfig& config) {
    cv::VideoCapture cap;
    auto open_source = [&]() -> bool {
        if (config.type == "camera") {
            return cap.open(config.source_int);
        }
        return cap.open(config.source_str);
    };

    bool logged_open_fail = false;
    int reopen_backoff_ms = 500;
    int frame_index = 0;
    while (running_) {
        if (!cap.isOpened()) {
            if (!open_source()) {
                if (!logged_open_fail) {
                    std::cerr << "Failed to open video source: " << config.name << ", retrying..." << std::endl;
                    logged_open_fail = true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(reopen_backoff_ms));
                continue;
            }
            logged_open_fail = false;
        }

        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            if (config.type == "video") {
                cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                cap >> frame;
                if (frame.empty()) {
                    cap.release();
                }
            } else {
                cap.release();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }
        
        FrameMeta meta;
        meta.channel_id = channel_id;
        meta.frame_index = frame_index++;
        meta.original_frame = frame.clone();

        pre_queue_.try_push(std::move(meta), std::chrono::milliseconds(1));
        
        int fps = (config.max_fps > 0) ? config.max_fps : 25;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::max(1, 1000 / fps)));
    }
}

void Workers::preprocess_thread() {
    while (running_) {
        FrameMeta meta;
        if (!pre_queue_.try_pop(meta, std::chrono::milliseconds(100))) {
            continue;
        }
        
        std::vector<uint8_t> input_tensor;
        engine_->preprocess(meta.original_frame, input_tensor, meta);
        
        int request_id = engine_->get_ie()->RunAsync(input_tensor.data(), nullptr, nullptr);

        WaitTask task;
        task.request_id = request_id;
        task.meta = std::move(meta);
        Wait_queue_.push(std::move(task));
    }
}

void Workers::Wait_thread() {
    auto ie = engine_->get_ie();
    auto ypp = engine_->get_ypp();
    
    while (running_) {
        WaitTask wait_task;
        if (!Wait_queue_.try_pop(wait_task, std::chrono::milliseconds(100))) {
            continue;
        }
        
        auto outputs = ie->Wait(wait_task.request_id);
        
        std::vector<YOLOv26Result> detections = ypp->postprocess(outputs);
        
        DrawTask task;
        task.meta = std::move(wait_task.meta);
        task.detections = detections;
        
        draw_queue_.push(task);
    }
}

void Workers::draw_thread() {
    auto ypp = engine_->get_ypp();
    (void)ypp;
    while (running_) {
        DrawTask task;
        if (!draw_queue_.try_pop(task, std::chrono::milliseconds(100))) {
            continue;
        }
        
        cv::Mat draw_frame = task.meta.original_frame.clone();
        
        std::vector<int> pad {task.meta.pad_top, task.meta.pad_left};
        std::vector<float> ratio {task.meta.ratio, task.meta.ratio};
        
        // draw
        std::vector<bool> active_classes;
        {
            std::lock_guard<std::mutex> lock(active_mutex_);
            active_classes = active_classes_;
        }
        for(auto& det : task.detections) {
            if (det.class_id >= 0 && det.class_id < static_cast<int>(active_classes.size()) && !active_classes[det.class_id]) continue;
            std::vector<float> box = det.box;
            box[0] = (box[0] - pad[1]) / ratio[0];  // pad[1] is pad_left
            box[1] = (box[1] - pad[0]) / ratio[1];  // pad[0] is pad_top
            box[2] = (box[2] - pad[1]) / ratio[0];
            box[3] = (box[3] - pad[0]) / ratio[1];

            int class_id = det.class_id;
            cv::Scalar color(
                ((class_id * 37) % 255),
                ((class_id * 83) % 255),
                ((class_id * 149) % 255)
            );

            cv::rectangle(draw_frame, cv::Point(box[0], box[1]), cv::Point(box[2], box[3]), color, 2);
            
            std::string label = (class_id >= 0 && class_id < static_cast<int>(COCO_CLASS_NAMES.size()) ? COCO_CLASS_NAMES[class_id] : "unknown") + 
                                " " + cv::format("%.2f", det.confidence);
            
            int baseLine = 0;
            cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
            cv::Point labelOrg(std::max(0.0f, box[0]), std::max(0.0f, box[1]) - 5);
            cv::rectangle(draw_frame, labelOrg + cv::Point(0, baseLine), labelOrg + cv::Point(labelSize.width, -labelSize.height), color, cv::FILLED);
            cv::putText(draw_frame, label, labelOrg, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
        
        {
            std::lock_guard<std::mutex> lock(display_mutex_);
            latest_frames_[task.meta.channel_id] = draw_frame;
        }
    }
}
