#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbGenerate_clicked();

    void on_pbBrowse_clicked();

    void on_leFileName_textChanged(const QString &arg1);

    void on_spTrain_valueChanged(int arg1);

    void on_spTime_valueChanged(int arg1);

    void on_spCounter_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QSettings *settings;
};

#endif // MAINWINDOW_HPP
