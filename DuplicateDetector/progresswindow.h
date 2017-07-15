#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QFuture>
#include <QFileInfo>
#include <QFileInfoList>
#include <QThreadPool>

#include <atomic>

namespace Ui {
class DDProgressWindow;
}

/**
 * @brief The DDProgressWindow class
 * Window displays the search progress
 */
class DDProgressWindow : public QMainWindow, public QRunnable
{
    Q_OBJECT

public:
    /**
     * @brief DDProgressWindow
     * @param dirs the list of directories
     * @param parent
     */
    explicit DDProgressWindow(const QStringList& dirs, QWidget *parent = 0);
    ~DDProgressWindow();

    /**
     * @brief GetDuplicates returns the list of found duplicates
     * @return
     */
    const QList<QList<QString>>& GetDuplicates();

    /**
     * @brief overrides QRunnable::run, used to calculate hash in parallel
     */
    virtual void run() override;

signals:
    /**
     * @brief SearchFinished occurs when search is finished
     */
    void SearchFinished();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void UpdateUI();

private:
    void RunSearch();
    void Cancel();

private:
    Ui::DDProgressWindow* ui;

    QStringList dirList;

    QList<QList<QString>> duplicates;

    QFuture<void> runningFuture;
    std::atomic_bool cancel;

    QList<QFileInfo> fileList;
    QMultiMap<QString, QString> fileHashMap;

    QThreadPool* thPool;
    std::atomic<float> progress = 0.0f;
    float progressStep = 0.0f;
    std::atomic_uint32_t duplicatesCount = 0;
    QMutex hashMutex;
};

#endif // PROGRESSWINDOW_H
