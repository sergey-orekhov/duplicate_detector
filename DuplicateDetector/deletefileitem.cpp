#include "deletefileitem.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

//////////////////////////////////////////////////////////////////

DDDeleteFileItem::DDDeleteFileItem(const QString& fileName, QWidget *parent)
    : QWidget(parent)
    , fileNameValue(fileName)
{
    QHBoxLayout* fileLayout = new QHBoxLayout(this);
    fileLayout->setMargin(0);
    fileLayout->setSpacing(3);

    QPushButton* deleteButton = new QPushButton("Delete", this);
    deleteButton->setMaximumWidth(50);
    deleteButton->setMaximumHeight(22);
    QObject::connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(DeleteItem()));

    QLabel* fileLabel = new QLabel(fileName, this);
    fileLayout->addWidget(deleteButton);
    fileLayout->addWidget(fileLabel);

    this->setLayout(fileLayout);
}

//////////////////////////////////////////////////////////////////

void DDDeleteFileItem::DeleteItem()
{
    emit DeleteFileItem(this);
}

//////////////////////////////////////////////////////////////////
