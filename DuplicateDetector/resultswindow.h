#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QList>

namespace Ui {
class DDResultsWindow;
}

class DDDeleteFileItem;

/**
 * @brief The DDResultsWindow class
 * The window to display the result of the search of duplicates
 */
class DDResultsWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief DDResultsWindow
     * @param duplicates const QMap<QString, QList<QString>>& the list of duplicates to display
     * @param parent
     */
    explicit DDResultsWindow(const QList<QList<QString>>& duplicates, QWidget *parent = 0);
    ~DDResultsWindow();

public slots:
    /**
     * @brief DeleteFileItem deletes an instance of DDDeleteFileItem from the view
     * @param fileItemWidget
     */
    void DeleteFileItem(const DDDeleteFileItem *fileItemWidget);

signals:
    /**
     * @brief OnCLosed occurs when window is closing
     */
    void OnCLosed();

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void DeleteItem(const DDDeleteFileItem *fileItemWidget);

private:
    Ui::DDResultsWindow *ui;
};

#endif // RESULTSWINDOW_H
