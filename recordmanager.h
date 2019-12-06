#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include <unordered_map>
#include <queue>
#include <list>
#include <fstream>
#include <cassert>
#include <vector>
#include "record.h"
#include "diskbtreenode.h"

class RecordManager
{
public:
    RecordManager(const char * dataFilePath, const char * freeRecordPath, int maxValues, int maxChars, unsigned minimum);
    ~RecordManager();

    DiskBTreeNode* getEmptyNode();
    DiskBTreeNode* getNode(unsigned recordNumber, bool empty=false, bool cache=true);
    void insertDeletedRecordNum(unsigned recNum);
    void writeNodeToFile(DiskBTreeNode* node);

private:
    bool fileExists(const char* fName);
    bool initFileHForRW(const char * fName);
    void dumpPreviousRecordsToFile();
    void getPreviousRecordsFromFile();

    bool removeFromCache(unsigned recNum);

    std::unordered_map<unsigned, DiskBTreeNode*> _nodeCache;
    std::list<unsigned> _cacheLimiter;

    Record * _rec;
    std::fstream * _prevRecordsFileH;
    std::string _prevRecordsPath;
    std::vector<unsigned> _previousRecords;
    unsigned _minimum;
    unsigned _maxCacheSize;
};


#endif // RECORDMANAGER_H
