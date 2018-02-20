#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>

#include "BlackEdgeEffect.h"
#include "Image.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:
    void on_openImageButton_clicked();
    void on_blurButton_clicked();
    void on_clarityButton_clicked();
    void on_sobelButton_clicked();
    void on_priutButton_clicked();
    void on_gaussButton_clicked();
    void on_edgeEffectComboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QScopedPointer<Image> image;

    void showImage();
    void enableButtons(bool enable);
};

#endif // MAINWINDOW_H
