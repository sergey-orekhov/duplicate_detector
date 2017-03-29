#ifndef DELETEFILEITEM_H
#define DELETEFILEITEM_H

#include <QObject>
#include <QWidget>

/**
 * @brief The DDDeleteFileItem class
 * Represents a single line in the list of duplicates
 * includes a button and label inside horisontal layout
 */

class DDDeleteFileItem : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief DDDeleteFileItem
     * @param fileName displayed file name
     * @param parent
     */
    explicit DDDeleteFileItem(const QString& fileName, QWidget *parent = 0);

    /**
     * @brief GetFileName
     * @return QString the file name
     */
    const QString& GetFileName() const { return fileNameValue; }

signals:
    /**
     * @brief DeleteFileItem called when the button pressed
     * @param fileItemWidget this
     */
    void DeleteFileItem(const DDDeleteFileItem* fileItemWidget);

public slots:
    /**
     * @brief DeleteItem listens the button click
     */
    void DeleteItem();

private:
    /**
     * @brief fileNameValue keeps the name of the file
     */
    QString fileNameValue;
};

#endif // DELETEFILEITEM_H
