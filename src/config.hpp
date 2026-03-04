#pragma once
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

struct ChannelConfig {
    std::string name;
    std::string type;
    std::string source_str;
    int source_int;
    bool enabled;
    int max_fps;
};

struct AppConfig {
    std::string model_path;
    int preprocess_workers;
    int wait_workers;
    int draw_workers;
    std::vector<ChannelConfig> channels;
};

inline AppConfig load_config(const std::string& config_path) {
    AppConfig config;
    YAML::Node config_node = YAML::LoadFile(config_path);
    if(config_node["model"]) {
        config.model_path = config_node["model"].as<std::string>();
    }
    if(config_node["channels"]) {
        for(const auto& ch : config_node["channels"]) {
            ChannelConfig c;
            c.name = ch["name"].as<std::string>("");
            c.type = ch["type"].as<std::string>("");
            c.enabled = ch["enabled"].as<bool>(false);
            if (!c.enabled) continue;
            
            c.max_fps = ch["max_fps"].as<int>(25);
            if (c.type == "camera") {
                c.source_int = ch["source"].as<int>(0);
            } else {
                c.source_str = ch["source"].as<std::string>("");
            }
            config.channels.push_back(c);
        }
    }
    return config;
}
