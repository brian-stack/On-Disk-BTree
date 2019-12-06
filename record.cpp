#include "record.h"
#include <iostream>
Record::Record(unsigned fieldCount, unsigned fieldWidth, const char * fPath)
{
    //Setup file i/o stream
    _fileName = std::string(fPath);
    _fileH = new std::fstream;

    initFileHForRW(fPath);

    _fieldCount = fieldCount;
    _fieldWidth = fieldWidth;
    _charsPerRecord = fieldCount * fieldWidth;
    _record = new char[_charsPerRecord];
}

void Record::resetFStream()
{
    initFileHForRW(_fileName.c_str());
}

bool Record::isFileGood()
{
    return _fileH->good();
}

Record::~Record()
{
    delete [] _record;
    if(_fileH->is_open())
        _fileH->close();
    delete _fileH;
}

//preconditions: none
//postconditions: fills _record with null terminators
void Record::initBuffer()
{
    for(int i = 0; i < _charsPerRecord; ++i)
        _record[i] = '\0';
}

//preconditions: fouts must be open.
//postconditions: copy the contents of theFields to _record,
// and write to file; returns the record number.
int Record::writeToFile(const std::vector<std::string>& theFields, int index)
{
    initBuffer();
    assert(_fileH->is_open());
    assert(theFields.size() <= _fieldCount);

    for(unsigned i = 0; i < theFields.size(); i++)
        std::strncpy(_record + (i*_fieldWidth),theFields.at(i).c_str(),_fieldWidth);

    //Set the internal pointer of the file stream to the begining of the ith record.
    // unless index < 0, then set the pointer to the end of the file.

    if(index < 0 && _fileH->tellp() > 0)
        _fileH->seekp(0, std::ios::end);
    else if(index >= 0)
        _fileH->seekp(index * _charsPerRecord, std::ios::beg);

    _fileH->write(_record,_charsPerRecord);
    _fileH->flush();

    return (_fileH->tellp() / _charsPerRecord) - 1;
}

//preconditions: fins must be open.
//postconditions: returns the number of characters read with this call to read.
//  if we are not eof, returns theFields containing all values from the record.
int Record::readFromFile(std::vector<std::string>& theFields, int recordNum)
{
    initBuffer();
    assert(_fileH->is_open());

    //if recordNum < 0, continue reading from the
    // last position saved internally by the ifstream.
    if(recordNum >= 0)
        _fileH->seekg((recordNum * _charsPerRecord), std::ios_base::beg);

    _fileH->read(_record, _charsPerRecord);
    if(_fileH->good())
    {
        for(int i = 0; i < _fieldCount; i++)
        {
            char * temp = new char[_fieldWidth+1];
            std::strncpy(temp, (_record + (i*_fieldWidth)), _fieldWidth);
            std::string item(temp);
            if(item.size() > 0)
                theFields.push_back(item);
            else
                theFields.push_back(UNITSEPERATOR);

            delete [] temp;
        }
    }
    return _fileH->gcount();
}

void Record::replaceField(std::string& replacement, unsigned fieldIndex, unsigned startPos)
{
    assert(_fileH->is_open());
    assert(replacement.size() < _fieldWidth);
    _fileH->seekp(startPos + (fieldIndex*_fieldWidth), std::ios::beg);
    _fileH->write(replacement.c_str(),_fieldWidth);
}

//preconditions: fName is a valid file path
//postconditions: fileH will be returned by reference and
// will now be associated with the provided file name.
// Returns true if fileH was able to open the file for RW, otherwise returns false.
bool Record::initFileHForRW(const char * fName)
{
    //Close the current file opened by fileH (if one is open)
    if(_fileH->is_open())
        _fileH->close();

    if(fileExists(fName))
        _fileH->open(fName,std::fstream::binary | std::fstream::out | std::fstream::in);
    else
    {
        _fileH->open(fName, std::fstream::binary | std::fstream::out | std::fstream::app);
        _fileH->close();
        _fileH->open(fName,std::fstream::binary | std::fstream::out | std::fstream::in);
    }
    return _fileH->is_open();
}

//preconditions: none
//postconditions: returns true if the file described by 'path' exists
// (and is accessable), otherwise returns false.
bool Record::fileExists(const char* fName)
{
    std::ifstream temp(fName);
    bool exists = temp.is_open();
    temp.close();
    return exists;
}
