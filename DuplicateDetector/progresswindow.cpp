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

#define MAX_THREAD_NUM 4

#define RETURN_IF_CANCELED \
    if (cancel.load())\
    {\
        return;\
    }\

#define DONE \
    emit SearchFinished(); \
    return; \

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
    runningFuture = QtConcurrent::run(this, &DDProgressWindow::RunSearch);
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

void DDProgressWindow::RunSearch()
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

    if (fileSet.empty())
    {
        DONE;
    }

    // filter files by size
    QMultiMap<qint64, QFileInfo> sizeFileMap;
    for (auto file: fileSet)
    {
        RETURN_IF_CANCELED;

        sizeFileMap.insert(file.size(), file);
    }

    fileSet.clear();

    // remove unique items from sizeFileMap
    for (auto key: sizeFileMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;

        if (sizeFileMap.values(key).size() <= 1)
        {
            sizeFileMap.remove(key);
        }
    }

    if (sizeFileMap.empty())
    {
        DONE;
    }

    // filter files by content (some first bytes)
    QMultiMap<QString, QFileInfo> contentFileMap;
    uint readBytes = sizeFileMap.firstKey();
    if (readBytes > MAX_READ_SIZE)
    {
        readBytes = MAX_READ_SIZE;
    }

    for(auto file: sizeFileMap.values())
    {
        RETURN_IF_CANCELED;

        if (file.exists() && !file.isDir())
        {
            QString absPath = file.absoluteFilePath();
            QFile f(absPath);
            if (f.open(QIODevice::ReadOnly))
            {
                QString key(f.read(readBytes).toHex());
                contentFileMap.insert(key, file);
                f.close();
            }
        }
    }

    sizeFileMap.clear();

    // remove unique items from contentFileMap
    for (auto key: contentFileMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;

        if (contentFileMap.values(key).size() <= 1)
        {
            contentFileMap.remove(key);
        }
    }

    if (contentFileMap.empty())
    {
        DONE;
    }

    fileList = contentFileMap.values();
    contentFileMap.clear();

    // filter by hash
    hashFileMap.clear();
    progress = 0.0f;
    progressStep = 100.0f / fileList.size();
    duplicatesCount = 0;

    thPool = new QThreadPool();
    thPool->setMaxThreadCount(MAX_THREAD_NUM);
    this->setAutoDelete(false);
    for(int i = 0; i < MAX_THREAD_NUM; i++)
    {
        thPool->start(this);
    }

    thPool->waitForDone();
    delete thPool;

    fileList.clear();

    // remove unique items from hashFileMap
    for (auto key: hashFileMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;

        if (hashFileMap.values(key).size() <= 1)
        {
            hashFileMap.remove(key);
        }
    }

    for (auto key: hashFileMap.uniqueKeys())
    {
        RETURN_IF_CANCELED;
        duplicates.append(hashFileMap.values(key));
    }

    DONE;
}

//////////////////////////////////////////////////////////////////

void DDProgressWindow::run()
{
    QMutexLocker locker(&hashMutex);

    // sync access to fileList
    while (!fileList.empty())
    {
        if (cancel.load())
        {
            break;
        }

        auto file = fileList.takeFirst();

        locker.unlock();

        // async hash calculation
        if (file.exists() && !file.isDir())
        {
            QString absPath = file.absoluteFilePath();
            QFile f(absPath);
            if (f.size() <= MAX_FILE_ZISE)
            {
                if (f.open(QIODevice::ReadOnly))
                {
                    QCryptographicHash hasher(QCryptographicHash::Sha1);
                    hasher.addData(&f);
                    QString sHash(hasher.result().toHex());

                    locker.relock();

                    // sync access to hashFileMap
                    hashFileMap.insert(sHash, file.absoluteFilePath());
                    uint filesForHash = hashFileMap.values(sHash).size();
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

        // sync access to fileList in a loop
        locker.relock();

        progress = progress + progressStep;
        QMetaObject::invokeMethod(this, "UpdateUI", Qt::QueuedConnection);
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
    this->ui->labelCount->setText(QString::number(duplicatesCount));
    this->ui->progressBar->setValue(static_cast<int>(progress));
}

//////////////////////////////////////////////////////////////////

const QList<QList<QString> >& DDProgressWindow::GetDuplicates()
{
    return duplicates;
}

//////////////////////////////////////////////////////////////////
