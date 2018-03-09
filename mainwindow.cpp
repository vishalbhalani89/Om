#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QDebug>
#include <QDateTime>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->setApplicationName("Om");
    settings = new QSettings(QString("%1/om.ini").arg(QDir::homePath()), QSettings::IniFormat, this);
    ui->leFileName->setText(settings->value("om/fileName", "").toString());
    ui->spCounter->setValue(settings->value("om/counter", 1).toInt());
    ui->spTime->setValue(settings->value("om/timeIncremental", 1).toInt());
    ui->spTrain->setValue(settings->value("om/trainIncremental", 1).toInt());
    statusBar()->showMessage(tr("ready to generate files"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbGenerate_clicked()
{
    QFile xmlFile(ui->leFileName->text());
    if(!xmlFile.exists())
    {
        QMessageBox::warning(this, tr("Om"), tr("File %1 does not exist.").arg(xmlFile.fileName()), QMessageBox::Ok);
        return;
    }

    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Om"), tr("Couldn't open %1 to read data").arg(xmlFile.fileName()), QMessageBox::Ok);
        return;
    }

    auto reader = new QXmlStreamReader(&xmlFile);

    for(int i = 0; i < ui->spCounter->value(); ++i)
    {
        QString trainName = "";
        qint64 trainNumber = 0;
        QFileInfo info(xmlFile);
        QString filename = QString("%1/temp.trn").arg(info.absolutePath());
        QFile *file = new QFile(filename);
        QXmlStreamWriter *writer = new QXmlStreamWriter(file);
        writer->setAutoFormatting(true);
        if(!file->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this,tr("Om"), tr("Couldn't create %1 to write data").arg(file->fileName()), QMessageBox::Ok);
            return;
        }
        xmlFile.seek(0);
        reader->setDevice(reader->device());
        while(!reader->atEnd() && !reader->hasError())
        {
            QXmlStreamReader::TokenType token = reader->readNext();
            if(token == QXmlStreamReader::Invalid)
                continue;
            if(token == QXmlStreamReader::StartElement)
            {
                if(reader->name() == "Zug")
                {
                    foreach(auto &attr, reader->attributes())
                    {
                        if(attr.name().toString() == "Gattung")
                            trainName = attr.value().toString();
                        if(attr.name().toString() == "Nummer")
                            trainNumber = attr.value().toLongLong();
                    }
                    filename = QString("%1/%2%3.trn").arg(info.absolutePath()).arg(trainName).arg(trainNumber+(ui->spTrain->value() * (i+1)));
                    statusBar()->showMessage(tr("generating file :%1").arg(filename));
                    writer->writeCurrentToken(*reader);
                    continue;
                }

                if(!writer)
                {
                    qDebug()<<reader->name().toString();
                    continue;
                }
                writer->writeStartElement(reader->name().toString());

                if(reader->name() == "FahrplanEintrag")
                {
                    foreach(auto &attr, reader->attributes())
                    {
                        QXmlStreamAttribute attribute;
                        if(attr.name().toString() == "Ank" || attr.name().toString() == "Abf")
                            attribute = QXmlStreamAttribute(attr.name().toString(), QDateTime::fromString(attr.value().toString(), "yyyy-MM-dd hh:mm:ss").addSecs(ui->spTime->value() * 60 * (i+1)).toString("yyyy-MM-dd hh:mm:ss"));
                        else
                            attribute = QXmlStreamAttribute(attr.name().toString(), attr.value().toString());
                        writer->writeAttribute(attribute);
                    }
                }
                else
                {
                    foreach(auto &attr, reader->attributes())
                    {
                        QXmlStreamAttribute attribute = QXmlStreamAttribute(attr.name().toString(), attr.value().toString());
                        writer->writeAttribute(attribute);
                    }
                }
            }
            else
                writer->writeCurrentToken(*reader);
        }
        writer->writeEndDocument();
        QFile::remove(filename);
        file->rename(filename);
    }
    xmlFile.close();
    statusBar()->showMessage(tr("finished"));
    QMessageBox::information(this, tr("Om"), tr("Files generation process is finished successfully"), QMessageBox::Ok);
}

void MainWindow::on_pbBrowse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), QDir::homePath(), tr("Train (*.trn)"));
    ui->leFileName->setText(fileName);
}

void MainWindow::on_leFileName_textChanged(const QString &arg1)
{
    settings->setValue("om/fileName", arg1);
}

void MainWindow::on_spTrain_valueChanged(int arg1)
{
    settings->setValue("om/trainIncremental", arg1);
}

void MainWindow::on_spTime_valueChanged(int arg1)
{
    settings->setValue("om/timeIncremental", arg1);
}

void MainWindow::on_spCounter_valueChanged(int arg1)
{
    settings->setValue("om/counter", arg1);
}
