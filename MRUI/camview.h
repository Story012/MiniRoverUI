#ifndef CAMVIEW_H
#define CAMVIEW_H

#include <QCamera>
#include <QImageCapture>

#include <QWidget>
#include <QObject>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMediaRecorder>
#include <QAudioInput>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QDateTime>
#include <QDir>
#include <QUrl> 

namespace Ui{
    class Widget;
}

class QCamera;
class QCameraImageCapture;
class QVBoxLayout;
class QCheckbox;
class QLabel;
class QLineEdit;
class QPushbutton;

class CamView : public QVideoWidget {
    Q_OBJECT
public:
    explicit CamView(int idx, QWidget *parent = nullptr);
    QVideoWidget *m_view_finder;
    static QList<QCamera*> cameras();
    static QCamera *camera(int i);
    void play(int device);
    void play(const QString &file);
    static QString currPath() {return QDir::currentPath();}
    QString fileName();

public slots:
    void takeImage();
    void record();
    void stop();

private slots:
    // void setCamera(const QCameraDevice &cameraDevice);
    void setupCamera(QCamera * selected_cam);
    void startCamera();
    void stopCamera();
    void updateCameras();

private:
    QActionGroup *m_video_devices_group = nullptr;
    QMediaDevices m_devices;
    QMediaCaptureSession *m_capture_session;
    QCamera *m_camera;
    QSharedPointer<QMediaRecorder> m_media_recorder;
    QSharedPointer<QAudioInput> m_audio_input;
    QImageCapture *m_img_cap;
    QBoxLayout *m_layout_;
    QMediaPlayer *m_player;
    static QList<QCameraDevice> s_devices;
    static QList<QCamera*> s_cameras;

    bool m_isCapturingImage = false;
    bool m_doImageCapture = true;

    int m_cameras_count = 0;
    int m_idx;
};

#endif // CAMVIEW_H

