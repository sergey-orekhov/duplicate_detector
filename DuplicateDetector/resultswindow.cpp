#include "resultswindow.h"
#include "ui_resultswindow.h"
#include "deletefileitem.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

//////////////////////////////////////////////////////////////////

#define LIST_MARGIN 20
#define FILE_WIDGET_HEIGHT 30

//////////////////////////////////////////////////////////////////

DDResultsWindow::DDResultsWindow(const QList<QList<QString> > &duplicates, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DDResultsWindow)
{
    ui->setupUi(this);

    // common widget and layout
    QWidget* scrollWidget = new QWidget(this);
    QVBoxLayout* commonLayout = new QVBoxLayout(scrollWidget);
    commonLayout->setSpacing(15);
    commonLayout->setMargin(3);

    // for each duplicate item
    // display separated widget with layout
    int count = 0;
    for (auto files: duplicates) {
        QWidget* keyWidget = new QWidget(scrollWidget);
        QVBoxLayout* KeyLayout = new QVBoxLayout(keyWidget);
        KeyLayout->setMargin(0);
        KeyLayout->setSpacing(0);

        QLabel* labelItem = new QLabel("Item " + QString::number(count++), keyWidget);
        labelItem->setStyleSheet("margin-left: 3px");
        KeyLayout->addWidget(labelItem);

        for (QString fileName: files) {
            // single line for each file
            DDDeleteFileItem* fileWidget = new DDDeleteFileItem(fileName, keyWidget);
            QObject::connect(fileWidget, SIGNAL(DeleteFileItem(const DDDeleteFileItem*)), this, SLOT(DeleteFileItem(const DDDeleteFileItem*)));
            KeyLayout->addWidget(fileWidget);
        }

        keyWidget->setLayout(KeyLayout);
        commonLayout->addWidget(keyWidget);
    }

    commonLayout->setAlignment(Qt::AlignTop);
    scrollWidget->setLayout(commonLayout);
    ui->scrollArea->setWidget(scrollWidget);
    ui->scrollArea->setAlignment(Qt::AlignTop);
}

//////////////////////////////////////////////////////////////////

DDResultsWindow::~DDResultsWindow()
{
    delete ui;
}

//////////////////////////////////////////////////////////////////

void DDResultsWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
    emit OnCLosed();
}

//////////////////////////////////////////////////////////////////

void DDResultsWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    int h = event->size().height();
    int w = event->size().width();

    QRect scrollAreaGeom = ui->scrollArea->geometry();
    scrollAreaGeom.setLeft(LIST_MARGIN);
    scrollAreaGeom.setRight(w - LIST_MARGIN);
    scrollAreaGeom.setTop(LIST_MARGIN);
    scrollAreaGeom.setBottom(h - LIST_MARGIN);
    ui->scrollArea->setGeometry(scrollAreaGeom);
}

//////////////////////////////////////////////////////////////////

void DDResultsWindow::DeleteFileItem(const DDDeleteFileItem* fileItemWidget)
{
    // remove one single item
    qRegisterMetaType<const DDDeleteFileItem *>("const DDDeleteFileItem *");
    QMetaObject::invokeMethod(this, "DeleteItem", Qt::QueuedConnection, Q_ARG(const DDDeleteFileItem *, fileItemWidget));
}

//////////////////////////////////////////////////////////////////

void DDResultsWindow::DeleteItem(const DDDeleteFileItem *fileItemWidget)
{
    QWidget* parent = static_cast<QWidget*>(fileItemWidget->parent());
    QLayout* layout = parent->layout();

    QString fileName = fileItemWidget->GetFileName();
    QFile file(fileName);
    if (file.exists()) {
        if (!file.remove()) {
            qDebug() << "Unable to delete file " << fileName;
        }
    }

    layout->removeWidget((QWidget*)fileItemWidget);
    delete fileItemWidget;

    // remove parent when only one file in the list, all other already removed
    if (layout->count() <=2) { // label item + another file
        delete parent;
    }
}

//////////////////////////////////////////////////////////////////
