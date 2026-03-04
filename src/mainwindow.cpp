#include "mainwindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QImage>
#include <QPixmap>

extern const std::vector<std::string> COCO_CLASS_NAMES;

MainWindow::MainWindow(Workers* workers, const AppConfig& config, QWidget *parent)
    : QMainWindow(parent), workers_(workers), config_(config) {
    setup_ui();
    
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::update_frames);
    timer_->start(30);
}

MainWindow::~MainWindow() {}

void MainWindow::setup_ui() {
    QWidget* central_widget = new QWidget(this);
    setCentralWidget(central_widget);
    
    QHBoxLayout* main_layout = new QHBoxLayout(central_widget);
    
    // Left side: Video grid
    QWidget* grid_widget = new QWidget(this);
    QGridLayout* grid_layout = new QGridLayout(grid_widget);
    
    int num_channels = config_.channels.size();
    int cols = (num_channels > 1) ? 2 : 1;
    
    for (int i = 0; i < num_channels; ++i) {
        QVBoxLayout* channel_layout = new QVBoxLayout();
        
        QLabel* title_label = new QLabel(QString::fromStdString(config_.channels[i].name));
        title_label->setStyleSheet("color: green; font-weight: bold; background-color: black;");
        
        QLabel* video_label = new QLabel();
        video_label->setMinimumSize(640, 360);
        video_label->setStyleSheet("background-color: black;");
        video_label->setScaledContents(true);
        
        channel_layout->addWidget(title_label);
        channel_layout->addWidget(video_label);
        
        grid_layout->addLayout(channel_layout, i / cols, i % cols);
        video_labels_.push_back(video_label);
        fps_labels_.push_back(title_label);
    }
    
    main_layout->addWidget(grid_widget, 1);
    
    // Right side: Classes panel
    QScrollArea* scroll_area = new QScrollArea(this);
    scroll_area->setFixedWidth(250);
    scroll_area->setWidgetResizable(true);
    
    QWidget* panel_widget = new QWidget();
    QVBoxLayout* panel_layout = new QVBoxLayout(panel_widget);
    
    QHBoxLayout* btn_layout = new QHBoxLayout();
    QPushButton* btn_select_all = new QPushButton("Select All");
    QPushButton* btn_clear_all = new QPushButton("Clear All");
    connect(btn_select_all, &QPushButton::clicked, this, &MainWindow::on_select_all);
    connect(btn_clear_all, &QPushButton::clicked, this, &MainWindow::on_clear_all);
    btn_layout->addWidget(btn_select_all);
    btn_layout->addWidget(btn_clear_all);
    panel_layout->addLayout(btn_layout);
    
    for (size_t i = 0; i < COCO_CLASS_NAMES.size(); ++i) {
        QCheckBox* cb = new QCheckBox(QString("%1: %2").arg(i).arg(QString::fromStdString(COCO_CLASS_NAMES[i])));
        cb->setChecked(true);
        connect(cb, &QCheckBox::toggled, [this, i](bool checked){ on_class_toggled(i, checked); });
        panel_layout->addWidget(cb);
        class_checkboxes_.push_back(cb);
    }
    panel_layout->addStretch();
    
    scroll_area->setWidget(panel_widget);
    main_layout->addWidget(scroll_area, 0);
    
    setWindowTitle("YOLO26 Multi-Channel Demo (C++)");
    resize(1400, 800);
}

void MainWindow::update_frames() {
    for (size_t i = 0; i < config_.channels.size(); ++i) {
        cv::Mat frame;
        if (workers_->get_latest_frame(i, frame)) {
            cv::Mat rgb;
            cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
            QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
            video_labels_[i]->setPixmap(QPixmap::fromImage(qimg));
        }
    }
}

void MainWindow::on_class_toggled(int id, bool checked) {
    (void)id;
    (void)checked;
    std::vector<bool> active(COCO_CLASS_NAMES.size(), false);
    for (size_t i = 0; i < class_checkboxes_.size(); ++i) {
        active[i] = class_checkboxes_[i]->isChecked();
    }
    workers_->set_active_classes(active);
}

void MainWindow::on_select_all() {
    for (auto cb : class_checkboxes_) cb->setChecked(true);
}

void MainWindow::on_clear_all() {
    for (auto cb : class_checkboxes_) cb->setChecked(false);
}
