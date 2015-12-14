#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QString>
#include <QPixmap>
#include <QTextEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QGraphicsView>
#include "pluginmanager.h"

#include "modellistener.h"

namespace Ui {
class MainWindow;
}

class LogWriter: public ModelListener
{
    QTextEdit *log;

public:
    void setOutput(QTextEdit *te);

    virtual void handleEvent(const Observed &obs);
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onOpen();
    void onExit();
    void onRepair();
    void onRefresh();
    void onChoose();
    void onNoFilter();
    void onFilter(int);

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QString image_path;
    Observed observed;
    PluginManager PManager;

    bool is_repaired;
    int filter_used;

    void setImage();
    void setObserver();
    void setButtonColors();
    void setActions();
    void setPlugins();
};

#endif // MAINWINDOW_H
