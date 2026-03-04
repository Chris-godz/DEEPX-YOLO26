#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>
#include <QGridLayout>
#include <QScrollArea>
#include <vector>
#include "workers.hpp"
#include "config.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(Workers* workers, const AppConfig& config, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void update_frames();
    void on_class_toggled(int id, bool checked);
    void on_select_all();
    void on_clear_all();

private:
    Workers* workers_;
    AppConfig config_;
    QTimer* timer_;
    
    std::vector<QLabel*> video_labels_;
    std::vector<QLabel*> fps_labels_;
    std::vector<QCheckBox*> class_checkboxes_;

    void setup_ui();
};
