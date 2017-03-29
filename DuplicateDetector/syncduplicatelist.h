#ifndef SYNCDUPLICATELIST_H
#define SYNCDUPLICATELIST_H

#include <QMutexLocker>
#include <QMutex>
#include <QMap>

/**
 * @brief The DDSyncDuplicateList class
 * Provides syncronized data structure for storing list of duplicates.
 * Duplicates stored in QMap<QString, QList>, where key is a hash value of value
 * QList the list of file names that have appropriate hash.
 */
class DDSyncDuplicateList
{
public:
    DDSyncDuplicateList();
    ~DDSyncDuplicateList();

    /**
     * @brief Add add new file
     * @param hash hash value of the file
     * @param fileName the file name
     */
    void Add(const QString& hash, const QString& fileName);

    /**
     * @brief Count returns the number of duplicates
     * @return int
     */
    int Count();

    /**
     * @brief GetDuplicates returns the list of duplicates
     * @return
     */
    const QMap<QString, QList<QString>>& GetDuplicates();

private:
    QMutex lock;
    QMap<QString, QString> singleMap;
    QMap<QString, QList<QString> > duplicateMap;
};

#endif // SYNCDUPLICATELIST_H
