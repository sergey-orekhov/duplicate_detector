#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QMainWindow>

namespace Ui {
class DDAboutWindow;
}

/**
 * @brief The DDAboutWindow class
 * Represent the About window.
 */
class DDAboutWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief DDAboutWindow constructor
     * @param parent parent widget
     */
    explicit DDAboutWindow(QWidget *parent = 0);
    ~DDAboutWindow();

private:
    Ui::DDAboutWindow* ui;
};

#endif // ABOUTWINDOW_H
