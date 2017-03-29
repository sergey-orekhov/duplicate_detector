#include "progresswindow.h"
#include "ui_progresswindow.h"

#include <QtConcurrent>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>

//////////////////////////////////////////////////////////////////

#define MAX_FILE_ZISE 1000000000 // 1Gb

//////////////////////////////////////////////////////////////////

DDProgressWindow::DDProgressWindow(const QStringList &dirs, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::DDProgressWindow)
    , dirList(dirs)
    , initialListSize(0)
{
    ui->setupUi(this);
    // run async task to do search
    runningFuture = QtConcurrent::run(this, &DDProgressWindow::Run);
    cancel = ATOMIC_VAR_INIT(false);
}

//////////////////////////////////////////////////////////////////

DDProgressWindow::~DDProgressWindow()
{
    // stop async task first
    Cancel();
    delete ui;
}

//////////////////////////////////////////////////////////////////

void DDProgressWindow::closeEvent(QCloseEvent *event)
{
    // stop async task
    Cancel();
    QMainWindow::closeEvent(event);
}

//////////////////////////////////////////////////////////////////

void DDProgressWindow::Run()
{
    // get the list of files from each directory
    for (int i=0; i<dirList.count(); i++)
    {
        const QString& dirName = dirList[i];

        if (cancel.load())
        {
            return;
        }

        // internal directories
        QFileInfoList list = QDir(dirName).entryInfoList(QStringList(), QDir::NoDotAndDotDot|QDir::Dirs);
        for (QFileInfo info: list)
        {
            dirList.append(info.absoluteFilePath());
        }

        fileList.Append(QDir(dirName).entryInfoList(QStringList(), QDir::Files));
    }

    initialListSize = fileList.Count();

    while(!fileList.IsEmpty())
    {
        if (cancel.load())
        {
            return;
        }

        // process each file
        this->ProcessFile();

        // update progress
        QMetaObject::invokeMethod(this, "UpdateUI", Qt::QueuedConnection);
    }

    // inform done
    emit SearchFinished();
}

//////////////////////////////////////////////////////////////////

void DDProgressWindow::ProcessFile()
{
    QFileInfo fInfo = fileList.TakeFirst();
    if (fInfo.exists() && !fInfo.isDir())
    {
        QString absPath = fInfo.absoluteFilePath();
        QFile f(absPath);
        if (f.size() > MAX_FILE_ZISE)
        {
            // too long
            // ignore it
            return;
        }

        if (f.open(QIODevice::ReadOnly))
        {
            QCryptographicHash hasher(QCryptographicHash::Sha1);
            hasher.addData(&f);
            QString sHash(hasher.result().toHex());
            // put to duplicate list
            duplicateList.Add(sHash, absPath);
            f.close();
        }
    }
}

//////////////////////////////////////////////////////////////////

void DDProgressWindow::Cancel()
{
    if (runningFuture.isRunning())
    {
        cancel.store(true);
        runningFuture.waitForFinished();
    }
}

//////////////////////////////////////////////////////////////////

void DDProgressWindow::UpdateUI()
{
    this->ui->labelCount->setText(QString::number(duplicateList.Count()));

    // update progress file
    if (fileList.IsEmpty())
    {
        this->ui->progressBar->setValue(100);
    }
    else
    {
        int val = 99;
        int size = fileList.Count();
        if (size > 0)
        {
            val = (int)(val * (1.0f - ((float)size / (float)initialListSize)));
        }

        this->ui->progressBar->setValue(val);
    }
}

//////////////////////////////////////////////////////////////////

const QMap<QString, QList<QString>>& DDProgressWindow::GetDuplicates()
{
    return duplicateList.GetDuplicates();
}

//////////////////////////////////////////////////////////////////
