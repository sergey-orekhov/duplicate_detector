#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "aboutwindow.h"
#include "progresswindow.h"
#include "resultswindow.h"

#include <QMainWindow>
#include <QStringList>

namespace Ui {
    class DDMainWindow;
}

class QFileDialog;

/**
 * @brief The DDMainWindow class
 * The main application window.
 */
class DDMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DDMainWindow(QWidget *parent = 0);
    ~DDMainWindow();

public slots:
    /**
     * @brief AddDirs adds directory into the list
     * @param selected
     */
    void AddDirs(const QStringList& selected);

    /**
     * @brief RemoveDir removes selected directory from the list
     */
    void RemoveDir();

    /**
     * @brief RemoveAllDirs clears the list of the directories
     */
    void RemoveAllDirs();

    /**
     * @brief SelectionChanged selected item changed
     */
    void SelectionChanged();

    /**
     * @brief RunSearch runs duplicates search
     */
    void RunSearch();

    /**
     * @brief SearchFinished occurs when search is finished
     */
    void SearchFinished();

    /**
     * @brief ResultWindowCLosed occurs when the result window closed
     */
    void ResultWindowCLosed();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Ui::DDMainWindow* ui;
    QFileDialog* dirDialog;
    DDAboutWindow* aboutWindow;
    DDProgressWindow* progressWindow;
    DDResultsWindow* resultsWindow;
};

#endif // MAINWINDOW_H
