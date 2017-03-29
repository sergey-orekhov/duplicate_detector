#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QResizeEvent>

//////////////////////////////////////////////////////////////////

#define FORM_MARGIN 20
#define MENU_HIGHT 20
#define SCROLL_TOP 40
#define BUTTON_MARGIN 20
#define BUTTON_HEIGHT 25
#define BUTTON_WIDTH 80

#define SCROLL_BOTTOM_OFFSET FORM_MARGIN - BUTTON_HEIGHT - BUTTON_MARGIN - MENU_HIGHT
#define BUTTON_TOP_OFFSET FORM_MARGIN - BUTTON_HEIGHT - MENU_HIGHT

//////////////////////////////////////////////////////////////////

DDMainWindow::DDMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DDMainWindow)
    , dirDialog(nullptr)
    , aboutWindow(nullptr)
    , progressWindow(nullptr)
    , resultsWindow(nullptr)
{
    ui->setupUi(this);

    // create an instance of About window
    aboutWindow = new DDAboutWindow(this);
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), aboutWindow, SLOT(show()));

    // create file dialog instance
    dirDialog = new QFileDialog(this, Qt::Dialog);
    dirDialog->setFileMode(QFileDialog::FileMode::DirectoryOnly);
    QObject::connect(ui->buttonAdd, SIGNAL(clicked(bool)), dirDialog, SLOT(show()));
    QObject::connect(dirDialog, SIGNAL(filesSelected(const QStringList&)), this, SLOT(AddDirs(const QStringList&)));

    QObject::connect(ui->actionAdd_directory, SIGNAL(triggered(bool)), dirDialog, SLOT(show()));
    QObject::connect(ui->actionRemove_All, SIGNAL(triggered(bool)), this, SLOT(RemoveAllDirs()));
    QObject::connect(ui->actionRun, SIGNAL(triggered(bool)), this, SLOT(RunSearch()));

    QObject::connect(ui->buttonRemove, SIGNAL(clicked(bool)), this, SLOT(RemoveDir()));
    QObject::connect(ui->buttonRemoveAll, SIGNAL(clicked(bool)), this, SLOT(RemoveAllDirs()));
    QObject::connect(ui->buttonRun, SIGNAL(clicked(bool)), this, SLOT(RunSearch()));

    QObject::connect(ui->listDirs, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()));
}

//////////////////////////////////////////////////////////////////

DDMainWindow::~DDMainWindow()
{
    delete dirDialog;
    delete aboutWindow;

    delete ui;
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::AddDirs(const QStringList& selected)
{
    for(QString dirName: selected)
    {
        ui->listDirs->addItem(dirName);
    }

    ui->buttonRemoveAll->setDisabled(false);
    ui->buttonRun->setDisabled(false);
    ui->actionRemove_All->setDisabled(false);
    ui->actionRun->setDisabled(false);
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::RemoveDir()
{
    for (QListWidgetItem* selectedItem: ui->listDirs->selectedItems())
    {
        QListWidgetItem* item = ui->listDirs->takeItem(ui->listDirs->row(selectedItem));
        delete item;
    }

    ui->buttonRemoveAll->setDisabled(ui->listDirs->count() == 0);
    ui->buttonRun->setDisabled(ui->listDirs->count() == 0);
    ui->actionRemove_All->setDisabled(ui->listDirs->count() == 0);
    ui->actionRun->setDisabled(ui->listDirs->count() == 0);
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::RemoveAllDirs()
{
    for (int i=ui->listDirs->count()-1; i>=0; i--)
    {
        QListWidgetItem* item = ui->listDirs->takeItem(i);
        delete item;
    }

    ui->buttonRemoveAll->setDisabled(true);
    ui->buttonRun->setDisabled(true);
    ui->actionRemove_All->setDisabled(true);
    ui->actionRun->setDisabled(true);
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::SelectionChanged()
{
    ui->buttonRemove->setDisabled(ui->listDirs->selectedItems().count() == 0);
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::RunSearch()
{
    QStringList dirs;

    for (int i=0; i<ui->listDirs->count(); i++)
    {
        QListWidgetItem* wi = ui->listDirs->item(i);
        if (wi)
        {
            dirs.append(wi->text());
        }
    }

    if (progressWindow != nullptr) {
        delete progressWindow;
    }
    // create an instance of progress window, that starts the search
    progressWindow = new DDProgressWindow(dirs, this);
    QObject::connect(progressWindow, SIGNAL(SearchFinished()), this, SLOT(SearchFinished()));
    progressWindow->show();
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::resizeEvent(QResizeEvent *event)
{
    // process window resize
    QMainWindow::resizeEvent(event);

    int h = event->size().height();
    int w = event->size().width();

    QRect scrollGeom = ui->listDirs->geometry();
    scrollGeom.setLeft(FORM_MARGIN);
    scrollGeom.setRight(w - FORM_MARGIN);
    scrollGeom.setTop(SCROLL_TOP);
    scrollGeom.setBottom(h - SCROLL_BOTTOM_OFFSET);
    ui->listDirs->setGeometry(scrollGeom);

    QRect addGeom = ui->buttonAdd->geometry();
    addGeom.setTop(h - BUTTON_TOP_OFFSET);
    addGeom.setHeight(BUTTON_HEIGHT);
    ui->buttonAdd->setGeometry(addGeom);

    QRect removeGeom = ui->buttonRemove->geometry();
    removeGeom.setY(h - BUTTON_TOP_OFFSET);
    removeGeom.setHeight(BUTTON_HEIGHT);
    ui->buttonRemove->setGeometry(removeGeom);

    QRect removeAllGeom = ui->buttonRemoveAll->geometry();
    removeAllGeom.setY(h - BUTTON_TOP_OFFSET);
    removeAllGeom.setHeight(BUTTON_HEIGHT);
    ui->buttonRemoveAll->setGeometry(removeAllGeom);

    QRect runGeom = ui->buttonRun->geometry();
    runGeom.setY(h - BUTTON_TOP_OFFSET);
    runGeom.setX(w - FORM_MARGIN - BUTTON_WIDTH);
    runGeom.setHeight(BUTTON_HEIGHT);
    runGeom.setWidth(BUTTON_WIDTH);
    ui->buttonRun->setGeometry(runGeom);
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::SearchFinished()
{
    progressWindow->hide();
    if (resultsWindow != nullptr) {
        delete resultsWindow;
    }

    // create another window to display results
    resultsWindow = new DDResultsWindow(progressWindow->GetDuplicates(), this);
    resultsWindow->show();
    QObject::connect(resultsWindow, SIGNAL(OnCLosed()), this, SLOT(ResultWindowCLosed()));
    delete progressWindow;
    progressWindow = nullptr;
}

//////////////////////////////////////////////////////////////////

void DDMainWindow::ResultWindowCLosed()
{
    delete resultsWindow;
    resultsWindow = nullptr;
}

//////////////////////////////////////////////////////////////////
