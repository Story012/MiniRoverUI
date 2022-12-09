#include "mainwindow.h"
#include "camview.h"

#include <QMediaRecorder>
#include <QVideoWidget>
#include <QCameraDevice>
#include <QMediaMetaData>
#include <QMediaDevices>
#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QDockWidget>
#include <QListWidget>

QString video_loc =  "/home/josue/code/MediaPlayer/sample_1280x720.mp4";
QString video_loc2 = "/home/josue/videoplayback.mp4"; 

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){
    video_loc = "/home/josue/code/MediaPlayer/sample_1280x720.mp4";
    video_loc2 = "/home/josue/videoplayback.mp4";

    setWindowTitle("Miniature Inspection Rover");
    d_player2 = new QDockWidget(tr("Auxiliary Camera"), this); // Just declaring this makes it semi visible
    d_toolbar = new QDockWidget(tr("Tools"), this);
    w_player2 = new QWidget;
    l_player2 = new QVBoxLayout;
    w_toolbar = new QWidget;
    w_stack = new QStackedWidget;
    l_toolbar = new QVBoxLayout;
    w_stack = new QStackedWidget;
    w_record_stack = new QWidget;
    l_record_stack = new QVBoxLayout;
    w_playback_stack = new QWidget;
    l_playback_stack = new QVBoxLayout;
    w_toolbox = new QToolBox;
    status_bar = new QStatusBar;
    w_vid_list = new QListWidget;
    w_vid_list->setWindowTitle("Recorded Videos");
    
    main_view = new CamView(0);
    second_view = new CamView(1);
    label = new QLabel;
    arc_logo.load("../MRUI/ARCLogo.png");
    label->setPixmap(arc_logo.scaled(100,100));

    b_quit = new QPushButton("Quit") ;
    b_switch_cam = new QPushButton("Switch Cameras");

    b_capture = new QPushButton ;
    b_capture->setIcon(QIcon("../MRUI/shoot.png"));
    b_capture->setIconSize(QSize(50, 50));
    b_record = new QPushButton ;
    b_record->setIcon(QIcon("../MRUI/record.png"));
    b_record->setIconSize(QSize(50, 50));
    b_stop = new QPushButton;
    b_stop->setIcon(QIcon("../MRUI/stop.png"));    
    b_stop->setIconSize(QSize(50, 50));
    b_stop->setEnabled(false);

    d_toolbar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    d_player2->setAllowedAreas(Qt::NoDockWidgetArea);
        getVideos();    
    assign();
    populate();
}

MainWindow::~MainWindow(){
}

void MainWindow::assign(){
    connect(b_capture, SIGNAL(clicked()), main_view, SLOT(takeImage()));
    connect(b_capture, SIGNAL(clicked()), second_view, SLOT(takeImage()));
    connect(b_record, SIGNAL(clicked()), this, SLOT(nowRecording()));
    connect(b_record, SIGNAL(clicked()), main_view, SLOT(record()));
    connect(b_record, SIGNAL(clicked()), second_view, SLOT(record()));
    connect(b_stop, SIGNAL(clicked()), this, SLOT(RecordingStopped()));
    connect(b_stop, SIGNAL(clicked()), main_view, SLOT(stop()));
    connect(b_stop, SIGNAL(clicked()), second_view, SLOT(stop()));
    connect(b_quit, SIGNAL(clicked()), this, SLOT(quitApp()));
    connect(w_vid_list, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(playVideos(QListWidgetItem*)));

    connect(b_switch_cam, SIGNAL(clicked()), this, SLOT(swapCameras()));
}
void MainWindow::populate(){
    l_record_stack->addWidget(b_capture);
    l_record_stack->addWidget(b_record);
    l_record_stack->addWidget(b_stop);
    w_record_stack->setLayout(l_record_stack);
    w_toolbox->addItem(w_record_stack, "Capture Video and Images");
    w_toolbox->addItem(w_vid_list, "Play Recorded");
    l_toolbar->addWidget(label);
    l_toolbar->addWidget(b_switch_cam);
    l_toolbar->addWidget(w_toolbox);
    l_toolbar->addStretch(200);
    l_toolbar->addWidget(b_quit);
    w_toolbar->setLayout(l_toolbar);
    d_toolbar->setWidget(w_toolbar);
    l_player2->addWidget(second_view);
    w_player2->setLayout(l_player2);
    d_player2->setWidget(w_player2);
    d_player2->setFloating(true);
    d_player2->resize(400,300);

    addDockWidget(Qt::RightDockWidgetArea, d_toolbar);
    setCentralWidget(main_view);
    centralWidget()->resize(500,500);
    setStatusBar(status_bar);
}

void MainWindow::quitApp(){
    qApp->exit(0);
}

void MainWindow::swapCameras(){
    if (!swapped){
        if (playback) {
            main_view->play(video_loc2);
            second_view->play(video_loc);
        } else {
            main_view->play(1);
            second_view->play(0);
        }
        d_player2->setWindowTitle("Main Camera");
        swapped = true;
        status_bar->showMessage("Camera View Swapped");
    } else {
        if (playback) {
            main_view->play(video_loc);
            second_view->play(video_loc2);
        } else {
            main_view->play(0);
            second_view->play(1);
        }
        d_player2->setWindowTitle("Auxiliary Camera");
        swapped = false;        
        status_bar->showMessage("Main Camera");
    }
}

void MainWindow::playBack(){
    if (!playback){
        if (swapped) {
            main_view->play(video_loc2);
            second_view->play(video_loc);
        } else {
            main_view->play(video_loc);
            second_view->play(video_loc2);
        }
        status_bar->showMessage(video_loc);
        playback = true;
    } else {
        if (swapped) {
            main_view->play(1);
            second_view->play(0);
        } else {
            main_view->play(0);
            second_view->play(1);
        }
        status_bar->showMessage("Playing Live Video");
        playback = false;        
    }
}

void MainWindow::nowRecording(){
    status_bar->showMessage(main_view->fileName());
    b_stop->setEnabled(true);
    b_record->setEnabled(false);
}

void MainWindow::RecordingStopped(){
    status_bar->showMessage("recording saved");
    b_record->setEnabled(true);
    b_stop->setEnabled(false);
}

void MainWindow::swapMenu(){
    if (w_stack->currentIndex() == 0){
        w_stack->setCurrentIndex(1);
        getVideos();        
    } else {
        w_stack->setCurrentIndex(0);
    }
}

void MainWindow::getVideos(){
    w_vid_list->clear();
    QStringList namefilter;
    namefilter <<"*0.mp4";
    QDir dir(CamView::currPath());
        for (const QString &filename : dir.entryList(namefilter,QDir::Files)){
            w_vid_list->addItem(filename);
        }
}

void MainWindow::playVideos(QListWidgetItem *video){
    video_loc = video->text();
    video_loc2 = video_loc;
    video_loc2.replace("-0.mp4","-1.mp4");
    main_view->play(video_loc);
    second_view->play(video_loc2);
}

void MainWindow::saveSettings() {
    QSettings settings("FIU-ARC", "MediaPlayer");
    settings.beginGroup( "MainWindow" );
    settings.setValue( "geometry", saveGeometry() );
    settings.setValue( "saveState", saveState() );
    settings.setValue( "maximized", isMaximized() );
    settings.endGroup();
}

void MainWindow::reloadSettings() {
    QSettings settings("FIU-ARC", "MediaPlayer");
    settings.beginGroup( "MainWindow" );
    restoreGeometry(settings.value( "geometry", saveGeometry() ).toByteArray());
    restoreState(settings.value( "saveState", saveState() ).toByteArray());
    settings.endGroup();
}