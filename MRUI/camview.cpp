#include <QCamera>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QActionGroup>
#include <QAudioInput>
#include <QVBoxLayout>
#include <QMediaPlayer>
#include <QDateTime>
#include <QFileInfo>
#include <QUrl> 

#include "camview.h"

QList<QCameraDevice> CamView::s_devices;
QList<QCamera*> CamView::s_cameras;

CamView::CamView(int idx, QWidget *parent) : QVideoWidget(parent){
    m_idx = idx;
    m_capture_session = new QMediaCaptureSession;
    m_video_devices_group = new QActionGroup(this);
    m_video_devices_group->setExclusive(true);
    updateCameras();
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &CamView::updateCameras);
    play(m_idx);   
    m_cameras_count = s_devices.length();
}

void CamView::setupCamera(QCamera * selected_cam){
    m_camera = selected_cam;     // reset from QSharedPointer
    m_capture_session->setCamera(m_camera); // main functionality

    if (!m_media_recorder) {
        m_media_recorder.reset(new QMediaRecorder);
        m_capture_session->setRecorder(m_media_recorder.data());
    }
    m_img_cap = new QImageCapture;
    m_capture_session->setImageCapture(m_img_cap);
    m_capture_session->setVideoOutput(this);
    if (m_camera->cameraFormat().isNull())
    {
        auto formats = m_camera->cameraDevice().videoFormats();
        if (!formats.isEmpty()) {
            QCameraFormat bestFormat;
            for (const auto &fmt : formats) {
                if (bestFormat.maxFrameRate() < 29 && fmt.maxFrameRate() > bestFormat.maxFrameRate())
                    bestFormat = fmt;
                else if (bestFormat.maxFrameRate() == fmt.maxFrameRate() &&
                         bestFormat.resolution().width()*bestFormat.resolution().height() <
                             fmt.resolution().width()*fmt.resolution().height())
                    bestFormat = fmt;
            }

            m_camera->setCameraFormat(bestFormat);
            m_media_recorder->setVideoFrameRate(bestFormat.maxFrameRate());
        }
    }
    m_camera->start();
}
void CamView::updateCameras(){
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    for (const QCameraDevice &camera_device : devices) {
        QAction *video_device_action = new QAction(camera_device.description(), m_video_devices_group);
        video_device_action->setCheckable(true);
        video_device_action->setData(QVariant::fromValue(camera_device));
        if (camera_device == QMediaDevices::defaultVideoInput())
            video_device_action->setChecked(true);
    }
}

void CamView::startCamera(){
    m_camera->start();
}

void CamView::stopCamera(){
    m_camera->stop();
    qDebug() << "buttonpressed";
}

void CamView::takeImage(){
    m_img_cap->captureToFile(fileName());
    qDebug() << "Image capture saved as" << fileName();
}

void CamView::record(){
    m_media_recorder->setOutputLocation(QUrl::fromLocalFile(fileName()));
    m_media_recorder->record();
}

void CamView::stop(){
    m_media_recorder->stop();
}

QList<QCamera*> CamView::cameras() {
    if (CamView::s_devices.isEmpty()) { // Only once!
        s_devices = QMediaDevices::videoInputs();
        for (const auto &device : s_devices) {
            qDebug() << "Found " << device.description();
            s_cameras.append( new QCamera(device) );
        }
    }
    return CamView::s_cameras;
}

QCamera *CamView::camera(int i) {
    auto cams = cameras();
    if (i >= cams.count()) {
        qDebug() << "Ops, camera" << i << "not found!";
        return nullptr;
    }
    return cams[i];
}

void CamView::play(int i) { // Play camera!
    auto c = camera(i);
    if (!c) {
        qDebug() << "Ops, cannot play camera," << i << "doesn't exit!";
        return;
    }
    this->setupCamera(c);
    qDebug() << "Playing camera" << i;
}

void CamView::play(const QString &file) {
    if (!QFileInfo::exists(file)) {
        qDebug() << "Ops, cannot play file," << file << "doesn't exit!";
        return;
    }
    m_player = new QMediaPlayer;
    m_player->setSource(file);
    m_player->setVideoOutput( this );
    m_player->play();
    qDebug() << "Playing video" << file;
}

QString CamView::fileName(){
    QString file_name = CamView::currPath();
    file_name += "/"+ QDateTime::currentDateTime().toString("yy-MM-dd_hh~mm-")
               + QString::number(m_idx);
    return file_name;
}