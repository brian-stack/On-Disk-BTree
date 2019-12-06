#include "diskbtreenode.h"

DiskBTreeNode::~DiskBTreeNode()
{
    delete [] _data;
    delete [] _subsets;
}

//constructor for a new (empty) node.
DiskBTreeNode::DiskBTreeNode(unsigned minimum, int rec)
    : _dataCount(0), _childCount(0), _recordNum(rec)
{
    _minimum = minimum;
    _maximum = 2* _minimum;
    _data = new int[_maximum+1];
    _subsets = new unsigned[_maximum+2];
}

//preconditions: a record must be structured as follows:
//  dataCount  : record[0]
//  childCount : record[1]
//  data items : record[2:2*minimum+3)
//  subsets    : record[2*minimum+3: 4*minimum+5)
//  All records contain 5 + minimum * 4 entries, if a data or subset are free,
//  a placeholder will be present in the record vector.
DiskBTreeNode::DiskBTreeNode(const vector<string>& record, const unsigned& rec) : _recordNum(rec)
{
    _minimum = (record.size() - 5) / 4;
    _maximum = _minimum * 2;

    assert(_minimum > 0);

    _data = new int[_maximum + 1];
    _subsets = new unsigned[_maximum + 2];
    _dataCount = stoi(record.at(0));
    _childCount = stoi(record.at(1));

    for(unsigned i = 0; i < _dataCount; ++i)
        _data[i] = stoi(record.at(i+2));

    unsigned offset = _maximum + 3;
    for(unsigned i = 0; i < _childCount; ++i)
        _subsets[i] = stoi(record.at(i+offset));
}

vector<string> DiskBTreeNode::nodeAsRecord(unsigned minimum)
{
    vector<string> exportV;
    exportV.push_back(to_string(_dataCount));
    exportV.push_back(to_string(_childCount));

    for(unsigned i = 0; i < _dataCount; ++i)
        exportV.push_back(to_string(_data[i]));

    for(unsigned i = 0; i < (minimum*2+1 - _dataCount); ++i)
        exportV.push_back(UNITSEPERATOR);

    for(unsigned i = 0; i < _childCount; ++i)
        exportV.push_back(to_string(_subsets[i]));

    for(unsigned i = 0; i < (minimum*2+2 - _childCount); ++i)
        exportV.push_back(UNITSEPERATOR);

    return exportV;
}
