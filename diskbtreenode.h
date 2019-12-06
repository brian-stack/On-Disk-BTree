#ifndef DISKBTREENODE_H
#define DISKBTREENODE_H

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "constants.h"
#include "arrayutil.h"

using namespace std;

class DiskBTreeNode
{
    friend ostream& operator<<(ostream& outs, const DiskBTreeNode& node)
    {
        outs << "Data: ";
        printArray(node._data, node._dataCount);
        outs << endl << "Subsets: ";
        printArray(node._subsets, node._childCount);
        outs << endl;
        outs << "dc : " << node._dataCount << ", cc: " << node._childCount << ", rec#: " << node._recordNum << std::endl;
        return outs;
    }

public:
    DiskBTreeNode(unsigned minimum, int rec=-1); //constructor for a new node.
    DiskBTreeNode(const vector<string>& record, const unsigned & rec);//construct a new node from a record

    //Instances of DiskBPTNode cannot be copied since a node is associated with a unique file position.
    DiskBTreeNode(const DiskBTreeNode&) = delete;
    void operator=(const DiskBTreeNode&) = delete;

    ~DiskBTreeNode();
    inline bool isLeaf() {return (_childCount == 0);}
    vector<string> nodeAsRecord(unsigned minimum);

    inline void setRecordNumber(unsigned recNum) {_recordNum = recNum;}
    inline unsigned getRecordNumber() {return _recordNum;}

private:
    //node internals
    unsigned _dataCount;
    unsigned _childCount;
    unsigned _minimum;
    unsigned _maximum;
    unsigned _recordNum;
    int *_data;
    unsigned *_subsets;

    friend class OnDiskBTree;
};
#endif // DISKBPTNODE_H
