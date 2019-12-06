#ifndef RECORD_H
#define RECORD_H
#include <string>
#include <vector>
#include <fstream>
#include "constants.h"
class Record
{
public:
    Record(unsigned maxValues, unsigned maxChars, const char * fPath);
    ~Record();
    //init _record to null
    void initBuffer();

    //return pos in file.
    int writeToFile(const std::vector<std::string>& theFields, int index=-1);

    //return gcount.
    int readFromFile(std::vector<std::string>& theFields, int recordNum = -1);

    void replaceField(std::string& replacement, unsigned fieldNumber, unsigned startPos);
    bool initFileHForRW(const char * fName);
    bool fileExists(const char * fName);
    bool isFileGood();
    void resetFStream();

private:
    char * _record;
    int _charsPerRecord;
    unsigned _fieldWidth;
    unsigned _fieldCount;
    std::string _fileName;
    std::fstream * _fileH;
};

#endif // RECORD_H
