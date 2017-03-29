#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QFuture>

#include <atomic>

#include "syncfilelist.h"
#include "syncduplicatelist.h"

namespace Ui {
class DDProgressWindow;
}

/**
 * @brief The DDProgressWindow class
 * Window displays the search progress
 */
class DDProgressWindow : public QMainWindow
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
    const QMap<QString, QList<QString>>& GetDuplicates();

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
    void Run();
    void Cancel();

    void ProcessFile();

private:
    Ui::DDProgressWindow* ui;

    QStringList dirList;

    QFuture<void> runningFuture;
    std::atomic_bool cancel;

    int initialListSize;
    DDSyncFileList fileList;
    DDSyncDuplicateList duplicateList;
};

#endif // PROGRESSWINDOW_H
