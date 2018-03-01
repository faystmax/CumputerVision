#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QImageReader>
#include <QGraphicsPixmapItem>

#include "ImageUtil.cpp"
#include "KernelCreator.h"
#include "ImageConverter.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->ui->graphicsView->setScene(new QGraphicsScene());

    // Fill combobox edgeEffects
    ui->edgeEffectComboBox->addItem("Mirror");
    ui->edgeEffectComboBox->addItem("Repeat");
    ui->edgeEffectComboBox->addItem("Black");
    ui->edgeEffectComboBox->addItem("Wrapping");

    // Disable all active buttons
    enableButtons(false);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_openImageButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", nullptr, "Image Files (*.png *.jpg *.bmp)");
    QImage qImage(fileName);

    if (!qImage.isNull()) {
        // Init our Image
        this->image = constructImage(qImage);
        ui->edgeEffectComboBox->setCurrentIndex(0);

        // Show image in window
        showImage(this->image);

        // Enable all active buttons
        enableButtons(true);
    }
}

void MainWindow::on_blurButton_clicked() {
    this->image = ImageConverter::convolution(this->image, KernelCreator::getBlur());
    showImage(this->image);
}

void MainWindow::on_clarityButton_clicked() {
    this->image = ImageConverter::convolution(this->image, KernelCreator::getClarity());
    showImage(this->image);
}

void MainWindow::on_sobelButton_clicked() {
    this->image = ImageConverter::sobel(this->image);
    showImage(this->image);
}

void MainWindow::on_priutButton_clicked() {
    this->image = ImageConverter::priut(this->image);
    showImage(this->image);
}

void MainWindow::on_gaussButton_clicked() {
    this->image = ImageConverter::convolution(this->image, KernelCreator::getGauss(5, 5, 4));
    showImage(this->image);
}

void MainWindow::on_pyramidButton_clicked() {
    pyramid.generate(this->image, this->ui->scalesSpinBox->value(), this->ui->sigmaSpinBox->value(),
                     this->ui->sigmaStartSpinBox->value());
    curPyramidIdex = 0;
    showImage(pyramid.getItem(curPyramidIdex).image);
    showPyramidInfo(pyramid.getItem(curPyramidIdex));

    //Enable buttons
    this->ui->pyramidLeftButton->setEnabled(true);
    this->ui->pyramidRightButton->setEnabled(true);
}

void MainWindow::on_pyramidLeftButton_clicked() {
    if (curPyramidIdex > 0) curPyramidIdex--;
    showImage(pyramid.getItem(curPyramidIdex).image);
    showPyramidInfo(pyramid.getItem(curPyramidIdex));
}

void MainWindow::on_pyramidRightButton_clicked() {
    if (curPyramidIdex < pyramid.getItemsSize() - 1) curPyramidIdex++;
    showImage(pyramid.getItem(curPyramidIdex).image);
    showPyramidInfo(pyramid.getItem(curPyramidIdex));
}

void MainWindow::on_generateLImageButton_clicked() {
    double sigma = this->ui->sigmaLSpinBox->value();
    QImage result = createFromL(image, pyramid, sigma);
    showImage(result);
}

void MainWindow::showPyramidInfo(const Item &item) {
    this->ui->infoPyramidTextEdit->setText(
            QString::fromStdString("Octave:     ") + QString::number(item.octave) + QString::fromStdString("<br>") +
            QString::fromStdString("Scale:      ") + QString::number(item.scale) + QString::fromStdString("<br>") +
            QString::fromStdString("SigmaScale: ") + QString::number(item.sigmaScale) +
            QString::fromStdString("<br>") +
            QString::fromStdString("SigmaEffect:") + QString::number(item.sigmaEffect) +
            QString::fromStdString("<br>"));
}


void MainWindow::on_edgeEffectComboBox_currentIndexChanged(int index) {
    switch (index) {
        case (0):
            return this->image.setEdgeEffect(Image::EdgeEffect::Mirror);
            break;
        case (1):
            return this->image.setEdgeEffect(Image::EdgeEffect::Repeat);
            break;
        case (2):
            return this->image.setEdgeEffect(Image::EdgeEffect::Black);
            break;
        case (3):
            return this->image.setEdgeEffect(Image::EdgeEffect::Wrapping);
            break;
    }
}

/**
 * @brief Show image in graphicsView
 * @param image
 */
void MainWindow::showImage(const Image &image) {
    this->ui->graphicsView->scene()->clear();
    QImage outputImage = getOutputImage(image);
    this->ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem(QPixmap::fromImage(outputImage)));
}

/**
 * @brief Show image in graphicsView
 * @param image
 */
void MainWindow::showImage(const QImage &image) {
    this->ui->graphicsView->scene()->clear();
    this->ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem(QPixmap::fromImage(image)));
}

/**
 * @brief Enable or disable active buttons on widget
 * @param enable
 */
void MainWindow::enableButtons(bool enable) {
    this->ui->blurButton->setEnabled(enable);
    this->ui->clarityButton->setEnabled(enable);
    this->ui->gaussButton->setEnabled(enable);
    this->ui->priutButton->setEnabled(enable);
    this->ui->sobelButton->setEnabled(enable);
    this->ui->pyramidButton->setEnabled(enable);
    this->ui->generateLImageButton->setEnabled(enable);
}
