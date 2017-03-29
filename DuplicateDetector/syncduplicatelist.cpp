#include "syncduplicatelist.h"

//////////////////////////////////////////////////////////////////

DDSyncDuplicateList::DDSyncDuplicateList()
{
}

//////////////////////////////////////////////////////////////////

DDSyncDuplicateList::~DDSyncDuplicateList()
{
    QMutexLocker(&(this->lock));
    singleMap.clear();
    duplicateMap.clear();
}

//////////////////////////////////////////////////////////////////

void DDSyncDuplicateList::Add(const QString& hash, const QString& fileName)
{
    QMutexLocker(&(this->lock));

    // we have two maps
    // it allows to return the list of duplicates
    // without filtration of single items

    if (duplicateMap.contains(hash))
    {
        // if duplicateMap already include such hash, just add new file into the list
        duplicateMap[hash].append(fileName);
    }
    else
    {
        if (singleMap.contains(hash))
        {
            // if singleMap already include the has, move it into duplicateMap
            // together with new item
            QString prevFileName = singleMap.take(hash);
            if (!duplicateMap.contains(hash))
            {
                duplicateMap.insert(hash, QList<QString>());
            }

            duplicateMap[hash].append(prevFileName);
            duplicateMap[hash].append(fileName);
        }
        else
        {
            // just put to singleMap
            singleMap.insert(hash, fileName);
        }
    }
}

//////////////////////////////////////////////////////////////////

int DDSyncDuplicateList::Count()
{
    QMutexLocker(&(this->lock));
    return duplicateMap.uniqueKeys().count();
}

//////////////////////////////////////////////////////////////////

const QMap<QString, QList<QString> >& DDSyncDuplicateList::GetDuplicates()
{
    QMutexLocker(&(this->lock));
    return duplicateMap;
}

//////////////////////////////////////////////////////////////////
