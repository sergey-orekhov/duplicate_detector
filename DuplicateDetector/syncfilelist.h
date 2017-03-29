#ifndef SYNCFILELIST_H
#define SYNCFILELIST_H

#include <QMutexLocker>
#include <QMutex>
#include <QFileInfo>
#include <QFileInfoList>

/**
 * @brief The DDSyncFileList class
 * Represents syncronised list of file names
 */
class DDSyncFileList
{
public:
    /**
     * @brief DDSyncFileList
     */
    DDSyncFileList();

    /**
      * @brief ~DDSyncFileList
      */
    ~DDSyncFileList();

    /**
     * @brief Append appends the items from passed list into internal list
     * @param files const QFileInfoList&
     */
    void Append(const QFileInfoList& files);

    /**
     * @brief TakeFirst returns and remove the first item from the list
     * @return QFileInfo
     */
    QFileInfo TakeFirst();

    /**
     * @brief IsEmpty check is the internal list is empty
     * @return true is the list is empty, false otherwise.
     */
    bool IsEmpty();

    /**
     * @brief Count return the number elements in the list
     * @return int
     */
    int Count();

private:
    /**
     * @brief lock mutex instance used for syncronizing
     */
    QMutex lock;

    /**
     * @brief fileList internal list of files
     */
    QFileInfoList fileList;
};

#endif // SYNCFILELIST_H
