#include "ondiskbtree.h"

//Constructor to load an existing OnDiskBTree from file.
OnDiskBTree::OnDiskBTree(const char * dataFile, const char * prevRecFile,
                     unsigned maxValues, unsigned maxChars, unsigned min) : _minimum(min)
{
    _maximum = min * 2;
    _recordManager = new RecordManager(dataFile,prevRecFile, maxValues, maxChars, min);
    _root = _recordManager->getNode(0,false);
}

OnDiskBTree::~OnDiskBTree()
{
    _recordManager->writeNodeToFile(_root);
    delete _recordManager;
}

//preconditions: none
//postconditions: the tree will be printed
void OnDiskBTree::printTree( ostream& outs, int level, int index, unsigned rec)
{
    DiskBTreeNode * root = _recordManager->getNode(rec);
    if(root->_childCount > 0)
    {
        for(int i = root->_childCount-1; i >= 0; --i)
        {
            printTree(outs, level+1,i,root->_subsets[i]);
            root = _recordManager->getNode(rec);
        }
    }
    outs << setw(level*10) << index << " : ";
    printArray(root->_data,root->_dataCount);
}

//preconditions: none
//postconditions: the entry will be inserted into the BTree using looseInsert,
// which when returning, may cause an excess in the root, which will be resolved by:
// 1) creating a new node, copying all the contents of this root into it,
// 2) clearing the root node,
// 3) making the new node this root's only child (subset[0])
// 4) calling fixExcess on this only subset (subset[0])
void OnDiskBTree::insert(int entry)
{
    _root = _recordManager->getNode(0);

    bool inserted = looseInsert(_root->_recordNum, entry);
    if(inserted)
    {
        if(_root->_dataCount == _maximum + 1)
        {
            DiskBTreeNode * newNode = _recordManager->getNode(0,true);
            copyArray(newNode->_data, _root->_data, newNode->_dataCount, _root->_dataCount);
            copyArray(newNode->_subsets, _root->_subsets, newNode->_childCount, _root->_childCount);
            _root->_dataCount = 0;
            _root->_childCount = 1;
            _root->_subsets[0] = newNode->_recordNum;
            fixExcess(0,_root);
        }
    }
}

//preconditions: none
//postcondition: looseRemove will be called to remove the target from the tree,
// the tree will be valid when returning, except that the root may have no data
// and only a single subset - This will be resolved by:
//  1) create a temporary pointer (shrink_ptr) and point it to this root's only subset
//  2) copy all the data and subsets of this subset into the root (through shrink_ptr)
//  3) now, the root contains all the data and poiners of it's old child.
//  4) simply delete shrink_ptr (blank out child), and the tree has shrunk by one level.
// Note, the root node of the tree will always be the same, it's the child node we delete
void OnDiskBTree::remove(int entry)
{
    _root = _recordManager->getNode(0);

    looseRemove(entry, _root->_recordNum);

    if(_root->_dataCount == 0 && _root->_childCount > 0)
    {
        DiskBTreeNode * firstSubset = _recordManager->getNode(_root->_subsets[0]);
        copyArray(_root->_data, firstSubset->_data, _root->_dataCount, firstSubset->_dataCount);
        copyArray(_root->_subsets, firstSubset->_subsets, _root->_childCount, firstSubset->_childCount);
        firstSubset->_childCount = 0;
        _recordManager->insertDeletedRecordNum(firstSubset->_recordNum);
    }
}

//preconditions: none
//postconditions: returns true if the entry exists in the tree, otherwise false.
bool OnDiskBTree::containsInternal(unsigned recNum, int entry)
{
    DiskBTreeNode * root = _recordManager->getNode(recNum);

    int index = firstGE(root->_data, root->_dataCount, entry);
    if(index < root->_dataCount && !(entry < root->_data[index]))
        return true;
    else if(root->_childCount == 0)
        return  false;
    else
    {
        bool result = containsInternal(root->_subsets[index], entry);
        return result;
    }
}

bool OnDiskBTree::contains(int entry)
{
    return containsInternal(0, entry);
}
//preconditions: none
//postconditions: the entry will be inserted into the tree,
// if it does not already exist, or if duplicates are allowed.
// The item will be inserted as follows:
//  1) find the index of the first item in data[] of this node that is not less than the entry.
//     if no such entry exists, index will be set to dataCount.
//  2) check if the entry exists in the tree
//  3) if the entry was found,
//     a) if dupsOk, insert it here.
//     b) else, return false.
//  4) if this node is a leaf, insert the entry here,
//     otherwise, call looseInsert on subset[i]
bool OnDiskBTree::looseInsert(unsigned recNum, int entry)
{
    DiskBTreeNode * root = _recordManager->getNode(recNum);
    int i = firstGE(root->_data, root->_dataCount, entry);
    bool itemInserted = false;
    bool found = (i < root->_dataCount && entry == root->_data[i]);

    if(found)
        itemInserted = false;
    else
    {
        if(root->isLeaf())
        {
            orderedInsert(root->_data, root->_dataCount, entry);
            itemInserted = true;
        }
        else
        {
            itemInserted = looseInsert(root->_subsets[i], entry);
        }

        if(itemInserted)
        {
            root = _recordManager->getNode(recNum);
            fixExcess(i,root);
        }
    }
    root = _recordManager->getNode(recNum);
    return itemInserted;
}

//preconditions: subset[i] must have an excess,  index < maximum+1, childCount <= maximum+1
//postconditions: the excess in subset[i] will be resolved as follows:
//  1) add a new subset at location i+1 of this node
//  2) split subset[i] (both the subset array and the data array)
//  and move half into subset[i+1] (this is the subset we created in step 1.)
//  3) detach the last data item of subset[i] and bring it and insert it into this node's data[]
//Note that this last step may cause this node to have too many items. This is OK. This will be
//dealt with at the higher recursive level. (my parent will fix it!)
void OnDiskBTree::fixExcess(int i, DiskBTreeNode * root)
{
    assert(i <= _maximum+1 && root->_childCount <= _maximum +1);
    if(i < root->_childCount)
    {
        DiskBTreeNode * subsetI = _recordManager->getNode(root->_subsets[i]);
        if(subsetI->_dataCount > _maximum)
        {
            DiskBTreeNode * newNode = _recordManager->getNode(0, true);
            insertItem(root->_subsets,i+1,root->_childCount,newNode->_recordNum);
            split(subsetI->_data,subsetI->_dataCount,newNode->_data,newNode->_dataCount);
            split(subsetI->_subsets,subsetI->_childCount,newNode->_subsets,newNode->_childCount);
            int insertVal = detachItem(subsetI->_data,subsetI->_dataCount);
            orderedInsert(root->_data,root->_dataCount, insertVal);
        }
    }
}

//preconditions: none
//postconditions: the entry, if it exists, will be removed from the tree.
// a) leaf && not found target: there is nothing to do
// b) leaf && found target: just remove the target
// c) not leaf and not found target: recursive call to loose_remove
// d) not leaf and found: replace target with largest child of subset[i]
void OnDiskBTree::looseRemove(int entry, unsigned recNum)
{   
    DiskBTreeNode * root = _recordManager->getNode(recNum);
    DiskBTreeNode * subsetI = nullptr;

    int index = firstGE(root->_data, root->_dataCount, entry);
    bool found = (index < root->_dataCount && root->_data[index] == entry);

    if(!root->isLeaf())
    {
        if(found)
        {
            int biggest = 0;
            removeBiggest(biggest, root->_subsets[index]);
            root->_data[index] = biggest;
        }
        else
            looseRemove(entry, root->_subsets[index]);

        subsetI = _recordManager->getNode(root->_subsets[index]);
        if(subsetI->_dataCount < _minimum)
        {
            root = _recordManager->getNode(recNum);
            fixShortage(index, root);
        }
    }
    else if(found)
        deleteItem(root->_data,index, root->_dataCount);
}

//preconnditions: none
//postconditions: the largest entry in this subtree will be deleted and placed in entry.
void OnDiskBTree::removeBiggest(int & entry, unsigned recNum)
{
    // Keep looking in the last subtree (recursive) until you get to a leaf.
    // Then, detach the last (biggest) data item, after the recursive call, fix shortage.
    DiskBTreeNode * root = _recordManager->getNode(recNum);

    if(!root->isLeaf())
    {
        unsigned cc = root->_childCount;
        removeBiggest(entry, root->_subsets[cc-1]);

        DiskBTreeNode * subsetcc = _recordManager->getNode(root->_subsets[cc-1]);
        if(subsetcc->_dataCount < _minimum)
        {
            root = _recordManager->getNode(recNum);
            fixShortage(cc - 1, root);
        }
    }
    else
        entry = detachItem(root->_data,root->_dataCount);
}

//preconditions: subset[i] must have a shortage.
//postconditions: the shortaged at subset[i] will be resolved by:
// 1) if: i+1 < childCount && subset[i+1]->dataCount > MINIMUM, then rotateLeft
// 2) if: i > 0 && i < childCount && subset[i-1]->dataCount > MINIMUM, then rotateRight
// 3) if i+1 < childCount, then mergeWithNextSubset
// 4) otherwise, mergeWithPreviousSubset
void OnDiskBTree::fixShortage(int i, DiskBTreeNode * root)
{
    DiskBTreeNode * rightC = nullptr;
    DiskBTreeNode * leftC = nullptr;

    if(i+1 < root->_childCount)
        rightC = _recordManager->getNode(root->_subsets[i+1]);
    if(i > 0 && i < root->_childCount)
        leftC = _recordManager->getNode(root->_subsets[i-1]);

    if(i+1 < root->_childCount && rightC->_dataCount > _minimum)
        rotateLeft(i, root);
    else if(i > 0 && i < root->_childCount && leftC->_dataCount > _minimum)
        rotateRight(i, root);
    else if(i+1 < root->_childCount)
        mergeWithNextSubset(i, root);
    else
        mergeWithPreviousSubset(i, root);
}

//preconditions: (i + 1 < childCount)
//postconditions: The shortage in subset[i] will be resolved by the following:
//                1) transfer data[i] to the front of subset[i+1] data
//                2) transfer all data and subsets from subset[i] to front of subset[i+1]
//                3) delete subset[i]
void OnDiskBTree::mergeWithNextSubset(int i, DiskBTreeNode * root)
{
    assert(root->_childCount> i+1);

    DiskBTreeNode * firstSubset = _recordManager->getNode(root->_subsets[i]);
    DiskBTreeNode * secondSubset = _recordManager->getNode(root->_subsets[i+1]);

    insertItem(secondSubset->_data,0,secondSubset->_dataCount, deleteItem(root->_data,i,root->_dataCount));
    mergeFront(secondSubset->_data, secondSubset->_dataCount, firstSubset->_data, firstSubset->_dataCount);
    mergeFront(secondSubset->_subsets, secondSubset->_childCount, firstSubset->_subsets, firstSubset->_childCount);
    unsigned deleted = deleteItem(root->_subsets,i, root->_childCount);
    _recordManager->insertDeletedRecordNum(deleted);
}

//preconditions: (i > 0)
//postconditions: The shortage in subset[i] will be resolved by the following:
//                1) transfer all data and subsets from subset[i] to end of subset[i-1]
//                2) delete subset[i]
void OnDiskBTree::mergeWithPreviousSubset(int i,DiskBTreeNode * root)
{
    assert(i > 0);

    DiskBTreeNode * firstSubset = _recordManager->getNode(root->_subsets[i-1]);
    DiskBTreeNode * secondSubset = _recordManager->getNode(root->_subsets[i]);

    firstSubset->_data[firstSubset->_dataCount] = deleteItem(root->_data, i-1, root->_dataCount);
    firstSubset->_dataCount += 1;
    mergeArrays(firstSubset->_data, firstSubset->_dataCount, secondSubset->_data, secondSubset->_dataCount);
    mergeArrays(firstSubset->_subsets, firstSubset->_childCount, secondSubset->_subsets, secondSubset->_childCount);
    unsigned deleted = deleteItem(root->_subsets, i, root->_childCount);
    _recordManager->insertDeletedRecordNum(deleted);
}

//preconditions: (dataCount > i) && (subset[i]->dataCount < MAXIMUM+1) && (subset[i+1]->dataCount > MINIMUM)
//postconditions: The shortage in subset[i] will be resolved by the following:
//                1) transfer data[i+1] to end of subset[i]->data
//                2) transfer first item from subset[i+1]->data to data[i].
//                3) If subset[i+1] has children, transfer subset[i+1]->subset[0] child to end of subset[i]->subset
void OnDiskBTree::rotateLeft(int i,DiskBTreeNode * root)
{
    DiskBTreeNode * firstSubset = _recordManager->getNode(root->_subsets[i]);
    DiskBTreeNode * secondSubset = _recordManager->getNode(root->_subsets[i+1]);

    assert((root->_dataCount > i) &&
           (firstSubset->_dataCount < _maximum+1) &&
           (secondSubset->_dataCount > _minimum));

    attachItem(firstSubset->_data, firstSubset->_dataCount, deleteItem(root->_data, i,root->_dataCount));
    insertItem(root->_data, i, root->_dataCount, deleteItem(secondSubset->_data, 0, secondSubset->_dataCount));
    if(secondSubset->_childCount> 0)
        attachItem(firstSubset->_subsets, firstSubset->_childCount,
                   deleteItem(secondSubset->_subsets, 0, secondSubset->_childCount));
}

//preconditions: (i > 0) && (subset[i]->dataCount < MAXIMUM+1) && (subset[i-1]->dataCount > MINIMUM)
//postconditions: The shortage in subset[i] will be resolved by the following:
//                1) transfer data[i-1] to front of subset[i]->data
//                2) transfer final item from subset[i-1]->data to data[i-1].
//                3) If subset[i-1] has children, transfer final child to front of subset[i]->subset
void OnDiskBTree::rotateRight(int i,DiskBTreeNode * root)
{
    DiskBTreeNode * firstSubset = _recordManager->getNode(root->_subsets[i-1]);
    DiskBTreeNode * secondSubset = _recordManager->getNode(root->_subsets[i]);

    assert((i > 0) && (secondSubset->_dataCount < _maximum+1)
           && (firstSubset->_dataCount > _minimum));

    insertItem(secondSubset->_data, 0, secondSubset->_dataCount, deleteItem(root->_data, i-1, root->_dataCount));
    insertItem(root->_data, i-1, root->_dataCount, detachItem(firstSubset->_data, firstSubset->_dataCount));
    if(firstSubset->_childCount > 0)
        insertItem(secondSubset->_subsets, 0, secondSubset->_childCount,
                   detachItem(firstSubset->_subsets, firstSubset->_childCount));
}
