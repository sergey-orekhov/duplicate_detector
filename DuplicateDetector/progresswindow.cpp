#include "progresswindow.h"
#include "ui_progresswindow.h"

#include <QtConcurrent>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QSet>

//////////////////////////////////////////////////////////////////

#define MAX_FILE_ZISE 1000000000 // 1Gb

#define MAX_READ_SIZE 32

#define RETURN_IF_CANCELED \
    if (cancel.load())\
    {\
        return;\
    }\

//////////////////////////////////////////////////////////////////

uint qHash(QFileInfo key) {
    return qHash(key.absoluteFilePath());
}

//////////////////////////////////////////////////////////////////

DDProgressWindow::DDProgressWindow(const QStringList &dirs, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::DDProgressWindow)
    , dirList(dirs)
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
    // discovery the list of unique files
    QSet<QFileInfo> fileSet;
    while (!dirList.isEmpty()) {
        RETURN_IF_CANCELED;

        QDir dir(dirList.takeFirst());

        // internal directories
        auto inDirs = dir.entryInfoList(QStringList(), QDir::NoDotAndDotDot|QDir::Dirs);
        for (auto inDir: inDirs)
        {
            RETURN_IF_CANCELED;
            dirList.append(inDir.absoluteFilePath());
        }

        // files
        fileSet.unite(QSet<QFileInfo>::fromList(dir.entryInfoList(QStringList(), QDir::Files)));
    }

    // filter files by size
    QMultiMap<qint64, QFileInfo> fileSizeMap;
    for (auto file: fileSet)
    {
        RETURN_IF_CANCELED;

        fileSizeMap.insert(file.size(), file);
    }

    fileSet.clear();

    // remove unique items from fileSizeMap
    for (auto key: fileSizeMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;

        if (fileSizeMap.values(key).size() <= 1)
        {
            fileSizeMap.remove(key);
        }
    }

    // filter files by content (some first bytes)
    QMultiMap<QString, QFileInfo> fileContentMap;
    uint readBytes = fileSizeMap.firstKey();
    if (readBytes > MAX_READ_SIZE)
    {
        readBytes = MAX_READ_SIZE;
    }

    for(auto file: fileSizeMap.values())
    {
        RETURN_IF_CANCELED;

        if (file.exists() && !file.isDir())
        {
            QString absPath = file.absoluteFilePath();
            QFile f(absPath);
            if (f.open(QIODevice::ReadOnly))
            {
                QString key(f.read(readBytes).toHex());
                fileContentMap.insert(key, file);
                f.close();
            }
        }
    }

    fileSizeMap.clear();

    // remove unique items from fileContentMap
    for (auto key: fileContentMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;

        if (fileContentMap.values(key).size() <= 1)
        {
            fileContentMap.remove(key);
        }
    }

    // filter by hash
    QMultiMap<QString, QString> fileHashMap;
    float progress = 0.0f;
    float progressStep = 100.0f / fileContentMap.size();
    uint duplicatesCount = 0;
    for(auto file: fileContentMap.values())
    {
        RETURN_IF_CANCELED;

        if (file.exists() && !file.isDir())
        {
            QString absPath = file.absoluteFilePath();
            QFile f(absPath);
            if (f.size() <= MAX_FILE_ZISE)
            {
                if (f.open(QIODevice::ReadOnly))
                {
                    // it would be better to use Sha1 instead of Md5, but Md5 is a bit faster.
                    QCryptographicHash hasher(QCryptographicHash::Md5);
                    hasher.addData(&f);
                    QString sHash(hasher.result().toHex());
                    fileHashMap.insert(sHash, file.absoluteFilePath());

                    uint filesForHash = fileHashMap.values(sHash).size();
                    if (filesForHash == 2)
                    {
                        // at least two new duplicates found
                        duplicatesCount += 2;
                    }
                    else
                    {
                        duplicatesCount++;
                    }

                    f.close();
                }
            }
            else
            {
                // too long
                // just ignore it
            }
        }

        QMetaObject::invokeMethod(this, "UpdateUI", Qt::QueuedConnection, Q_ARG(float, progress += progressStep), Q_ARG(uint, duplicatesCount));
    }

    fileSizeMap.clear();

    // remove unique items from fileHashMap
    for (auto key: fileHashMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;

        if (fileHashMap.values(key).size() <= 1)
        {
            fileHashMap.remove(key);
        }
    }

    for (auto key: fileHashMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;
        duplicates.append(fileHashMap.values(key));
    }

    // inform done
    emit SearchFinished();
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

void DDProgressWindow::UpdateUI(float progress, uint count)
{
    this->ui->labelCount->setText(QString::number(count));
    this->ui->progressBar->setValue(static_cast<int>(progress));
}

//////////////////////////////////////////////////////////////////

const QList<QList<QString> >& DDProgressWindow::GetDuplicates()
{
    return duplicates;
}

//////////////////////////////////////////////////////////////////
