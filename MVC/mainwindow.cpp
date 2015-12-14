#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QString>
#include <QPixmap>
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <Qt>
#include <QtGui>
#include <QAction>
#include <QDesktopWidget>
#include <QTime>
#include <QGraphicsView>
#include <QDockWidget>
#include <QFrame>
#include <QPushButton>
#include <QSignalMapper>

#include "src/model.h"
#include "pluginmanager.h"

#include <string>

void LogWriter::setOutput(QTextEdit *te)
{
    log = te;
}

void LogWriter::handleEvent(const Observed &obs)
{
    QTime clock;

    log->append(clock.currentTime().toString("[hh:mm:ss.zzz] ")+QString(obs.getMsg().c_str()));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    is_repaired(true),
    filter_used(-1)
{
    ui->setupUi(this);

    PManager.refreshPlugins();

    setImage();
    setObserver();
    setButtonColors();
    setActions();
    setPlugins();
}

void MainWindow::setImage()
{
    imageLabel = new QLabel;
    ui->scrollArea->setWidget(imageLabel);
}

void MainWindow::setObserver()
{
    LogWriter *writer = new LogWriter;
    writer->setOutput(ui->logView);

    observed.setListener(writer);
}

void MainWindow::setPlugins()
{
    QAction *refresh = new QAction(QString("Обновить плагины"), ui->menuPlugins);
    connect(refresh, SIGNAL(triggered()), this, SLOT(onRefresh()));

    QAction *no_filter = new QAction(QString("Не использовать фильтр"), ui->menuPlugins);
    connect(no_filter, SIGNAL(triggered()), this, SLOT(onNoFilter()));

    vector<Filter*> filters = PManager.getFilters();

    ui->menuPlugins->clear();

    ui->menuPlugins->addAction(refresh);

    for (uint it = 0; it < filters.size(); ++it)
    {
        QAction *filter_action = new QAction(QString((filters[it]->getName()).c_str()), ui->menuPlugins);
        //connect(filter_action, SIGNAL(triggered()), this, SLOT(onFilter(int)));

        ui->menuPlugins->addAction(filter_action);

        QSignalMapper *signalMapper = new QSignalMapper(this);
        connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(onFilter(int)));

        signalMapper->setMapping(filter_action, it);
        connect(filter_action, SIGNAL(triggered()), signalMapper, SLOT(map()));
    }

    ui->menuPlugins->addAction(no_filter);
}

void MainWindow::onNoFilter()
{
    filter_used = -1;
    ui->filterButton->setText(QString("Фильтр не используется"));
}

void MainWindow::setActions()
{
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onExit()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpen()));
    connect(ui->actionRepair, SIGNAL(triggered()), this, SLOT(onRepair()));
    connect(ui->filterButton, SIGNAL(clicked()), this, SLOT(onChoose()));
    connect(ui->repairButton, SIGNAL(clicked()), this, SLOT(onRepair()));
}

void MainWindow::setButtonColors()
{
    QPalette palette;

    //SET BACKGROUND TO REPAIR BITTON
    ui->repairButton->setAutoFillBackground(true);
    palette.setColor(QPalette::Button,QColor(123,250,114));
    ui->repairButton->setPalette(palette);

    //SET BACKGROUND TO CHOOSE_FILTER BITTON
    ui->filterButton->setAutoFillBackground(true);
    palette.setColor(QPalette::Button,QColor(255,243,117));
    ui->filterButton->setPalette(palette);
}


void MainWindow::onExit()
{
    QApplication::quit();
}

void MainWindow::onOpen()
{
    image_path.clear();

    image_path = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "/", tr("Image Files (*.bmp)"));

    if (!image_path.isEmpty())
    {
        QPixmap pix(image_path);

        imageLabel->setPixmap(pix);
        ui->left_window->setStyleSheet("background-color:black;");

        observed.send(QString("Image opened:\n") + image_path + QString("\n"));
        is_repaired = false;
    }
}

void MainWindow::onChoose()
{
    ui->menuPlugins->exec();
}

void MainWindow::onFilter(int num)
{
    filter_used = num;
    ui->filterButton->setText(QString("Используется: ") + QString(PManager.getFilters()[num]->getName().c_str()));
}

void MainWindow::onRepair()
{
    if (!image_path.isEmpty() && !is_repaired)
    {
        QString temp_path(image_path);
        temp_path.insert(temp_path.length()-4, QString("_tmp"));

        MechanismReparer reparer;
        reparer.setInputPath(image_path.toStdString());
        reparer.setOutputPath(temp_path.toStdString());
        reparer.setObserved(observed);
        if (filter_used != -1)
            reparer.setFilter(PManager.getFilters()[filter_used]);

        try
        {
            reparer.repair();
        }
        catch (string s)
        {
            std::cout << "###" << s << "###" << std::endl;
        }

        ui->logView->append(QString("###################################\n"));

        QPixmap pix(temp_path);
        imageLabel->setPixmap(pix);

        //std::remove(temp_path.toStdString().c_str());
        is_repaired = true;
    }
}

void MainWindow::onRefresh()
{
    PManager.refreshPlugins();
    setPlugins();
}

MainWindow::~MainWindow()
{
    delete ui;
}
