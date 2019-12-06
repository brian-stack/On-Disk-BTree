#include "recordmanager.h"

RecordManager::RecordManager(const char * dataFilePath, const char * freeRecordPath,
                             int maxValues, int maxChars, unsigned minimum)
    : _minimum(minimum), _prevRecordsPath(string(freeRecordPath))
{

    //This should be dynamic in the future.
    _maxCacheSize = max((100000 / (maxValues * maxChars)), 20);
    _rec = new Record(maxValues, maxChars, dataFilePath);

    //Load previous records (if any).
    _prevRecordsFileH = new std::fstream();
    getPreviousRecordsFromFile();
    initFileHForRW(freeRecordPath);
}

RecordManager::~RecordManager()
{
    while(!_cacheLimiter.empty())
    {
        removeFromCache(_cacheLimiter.front());
        _cacheLimiter.pop_front();
    }

    dumpPreviousRecordsToFile();
    delete _rec;
    if(_prevRecordsFileH->is_open())
        _prevRecordsFileH->close();
    delete _prevRecordsFileH;
}

bool RecordManager::removeFromCache(unsigned recNum)
{
    bool inCache = (_nodeCache.find(recNum) != _nodeCache.end());
    if(inCache)
    {
        DiskBTreeNode * removed = _nodeCache[recNum];
        _nodeCache.erase(recNum);
        writeNodeToFile(removed);
        delete removed;
        removed = nullptr;
    }
    return inCache;
}

//preconditions: fName is a valid file path
//postconditions: fileH will be returned by reference and
// will now be associated with the provided file name.
// Returns true if fileH was able to open the file for RW, otherwise returns false.
bool RecordManager::initFileHForRW(const char * fName)
{
    //Close the current file opened by fileH (if one is open)
    if(_prevRecordsFileH->is_open())
        _prevRecordsFileH->close();

    if(fileExists(fName))
        _prevRecordsFileH->open(fName,std::fstream::binary | std::fstream::out | std::fstream::in | std::fstream::app);
    else
    {
        _prevRecordsFileH->open(fName, std::fstream::binary | std::fstream::out | std::fstream::app);
        _prevRecordsFileH->close();
        _prevRecordsFileH->open(fName,std::fstream::binary | std::fstream::out | std::fstream::in | std::fstream::app);
    }
    return _prevRecordsFileH->is_open();
}

//preconditions: none
//postconditions: returns true if the file described by 'path' exists
// (and is accessable), otherwise returns false.
bool RecordManager::fileExists(const char* fName)
{
    std::ifstream temp(fName);
    bool exists = temp.is_open();
    temp.close();
    return exists;
}

DiskBTreeNode* RecordManager::getEmptyNode()
{
    DiskBTreeNode * newNode = nullptr;
    unsigned recordNum;

    if(_previousRecords.size() > 0)
    {
        recordNum = _previousRecords.back();
        _previousRecords.pop_back();
        newNode = new DiskBTreeNode(_minimum, recordNum);
    }
    else
    {
        newNode = new DiskBTreeNode(_minimum, -1);
        recordNum = _rec->writeToFile(newNode->nodeAsRecord(_minimum),-1);
        newNode->setRecordNumber(recordNum);
    }
    return newNode;
}

DiskBTreeNode* RecordManager::getNode(unsigned recordNumber, bool empty, bool cache)
{
    std::vector<string> fromFile;
    DiskBTreeNode * node = nullptr;

    if(!empty && _nodeCache.find(recordNumber) != _nodeCache.end())
    {
        node = _nodeCache[recordNumber];

        //Testing, when a node is requested, its position is reset in the cache limiter.
        _cacheLimiter.remove(recordNumber);
        _cacheLimiter.push_back(recordNumber);
    }
    else
    {
        if(!empty)
        {
            _rec->readFromFile(fromFile, recordNumber);
            if(!_rec->isFileGood())
                _rec->resetFStream();
        }
        if(fromFile.empty())
        {
            node = getEmptyNode();
            recordNumber = node->getRecordNumber();
        }
        else
            node = new DiskBTreeNode(fromFile,recordNumber);

        if(cache)
        {
            //If cache is at capacity, write the oldest item to file to make room.
            if(_cacheLimiter.size() == _maxCacheSize)
            {
                removeFromCache(_cacheLimiter.front());
                _cacheLimiter.pop_front();
            }

            //Insert recently read node to cache.
            _nodeCache.insert(std::pair<unsigned, DiskBTreeNode*>(recordNumber, node));
            _cacheLimiter.push_back(recordNumber);
        }
    }
    return node;
}

void RecordManager::insertDeletedRecordNum(unsigned recNum)
{
    removeFromCache(recNum);
    _cacheLimiter.remove(recNum);
    _previousRecords.push_back(recNum);

}

void RecordManager::writeNodeToFile(DiskBTreeNode* node)
{
    static unsigned count = 0;
    std::vector<string> toFile = node->nodeAsRecord(_minimum);
    _rec->writeToFile(toFile ,node->getRecordNumber());
    count += 1;
    std::cout << count << std::endl;
}

void RecordManager::getPreviousRecordsFromFile()
{
    //Close file if it is open, and reopen in read mode.
    if(_prevRecordsFileH->is_open())
        _prevRecordsFileH->close();
    _prevRecordsFileH->open(_prevRecordsPath.c_str(), ios_base::in);

    int current;
    std::string temp;
    while(!_prevRecordsFileH->eof() && _prevRecordsFileH->good())
    {
        *_prevRecordsFileH >> current;
        if(!_prevRecordsFileH->eof())
            _previousRecords.push_back(current);
    }
}

void RecordManager::dumpPreviousRecordsToFile()
{
    //Close file if it is open, and reopen in truncate mode.
    if(_prevRecordsFileH->is_open())
    {
        _prevRecordsFileH->flush();
        _prevRecordsFileH->close();
    }
    _prevRecordsFileH->open(_prevRecordsPath.c_str(), ios_base::out | ios_base::trunc);

    //Write vector contents to file and clear out vector.
    if(_prevRecordsFileH->is_open())
    {
        for(unsigned & item : _previousRecords)
            *_prevRecordsFileH << item << std::endl;

        _prevRecordsFileH->close();
        _previousRecords.clear();
    }
}
