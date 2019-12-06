#include <random>
#include "ondiskbtree.h"
#include <set>
using namespace std;

int main()
{
    string fName = "bTreeFile.txt";
    string prevRecords = "_prevRecords.txt";
    unsigned minimum = 100;
    unsigned numValues = 5 + (4*minimum);
    unsigned numChars = 10;

    OnDiskBTree tree(fName.c_str(),prevRecords.c_str(),numValues,numChars,minimum);
    set<int> inserted;

    //insert 10k random integers
    for(unsigned i = 0; i < 10000; ++i)
    {
        int x = rand()  % 10000000;
        tree.insert(x);
        inserted.insert(x);
    }
    set<int>::iterator it = inserted.begin();
    set<int> removed;

    //remove multiples of 13
    while(it != inserted.end())
    {
        if(*it % 13 == 0)
        {
            tree.remove(*it);
            removed.insert(*it);
        }
        ++it;
    }

    //verify that all values not deleted that were inserted are still present
    it = inserted.begin();
    while(it != inserted.end())
    {
        if(removed.find(*it) == removed.end() && !tree.contains(*it))
        {
            cout << "[error] Could not find: " << *it << endl;
            exit(EXIT_FAILURE);
        }
        ++it;
    }

    it = removed.begin();

    //verify that all deleted values do not still exist in the tree
    while(it != removed.end())
    {
        if(tree.contains(*it))
        {
            cout << "[error] Found: " << *it << endl;
            exit(EXIT_FAILURE);
        }
        ++it;
    }
    return 0;
}
