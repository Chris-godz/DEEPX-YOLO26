#include <QApplication>
#include "mainwindow.hpp"
#include "workers.hpp"
#include "config.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    std::string config_path = "config/default.yaml";
    if (argc > 1) {
        config_path = argv[1];
    }
    
    std::cout << "Loading config from " << config_path << std::endl;
    AppConfig config = load_config(config_path);
    
    if (config.channels.empty()) {
        std::cerr << "No enabled channels found in config!" << std::endl;
        return -1;
    }

    Workers workers(config.model_path, config.channels);
    workers.start();
    
    MainWindow w(&workers, config);
    w.show();

    int ret = a.exec();

    workers.stop();
    return ret;
}
