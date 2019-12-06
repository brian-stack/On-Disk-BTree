#ifndef ONDISKBTREE_H
#define ONDISKBTREE_H
#include <iostream>
#include <fstream>
#include <stack>
#include "recordmanager.h"
using namespace std;

class OnDiskBTree
{
    friend ostream& operator<<(ostream& outs, OnDiskBTree& tree)
    {
        tree.printTree(outs, 0, 0, tree._root->_recordNum);
        return outs;
    }

public:
    OnDiskBTree(const char * dataFile, const char * prevRecFile,
                         unsigned maxValues, unsigned maxChars, unsigned minimum);
    ~OnDiskBTree();

    void insert(int entry);                //insert entry into the tree
    void remove(int entry);                //remove entry from the tree
    bool contains(int entry);              //true if entry can be found in the array

private:
    unsigned _minimum;              //the minimum number of data items that a node may contain
    unsigned _maximum;
    RecordManager * _recordManager;
    DiskBTreeNode * _root;

    void printTree( ostream& outs, int level, int index, unsigned rec);

    bool containsInternal(unsigned recNum, int entry);

    //insert element functions
    bool looseInsert(unsigned recNum, int entry);                   //allows MAXIMUM+1 data elements in the root
    void fixExcess(int i, DiskBTreeNode * root);      //fix excess of data elements in child i

    //remove element functions:
    void looseRemove(int entry, unsigned recNum);                   //allows MINIMUM-1 data elements in the root
    void fixShortage(int i, DiskBTreeNode * root);                       //fix shortage of data elements in child i
    void removeBiggest(int & entry, unsigned recNum);               //remove the biggest child of this tree->entry

    //balancing functions:
    void rotateLeft(int i,DiskBTreeNode * root);                        //transfer one element LEFT from child i
    void rotateRight(int i,DiskBTreeNode * root);                       //transfer one element RIGHT from child i
    void mergeWithNextSubset(int i,DiskBTreeNode * root);               //merge subset i with subset i+1
    void mergeWithPreviousSubset(int i,DiskBTreeNode * root);
};

#endif // ONDISKBTREE_H
