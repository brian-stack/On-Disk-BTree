#ifndef ARRAYUTIL_H
#define ARRAYUTIL_H
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
using namespace std;

//return the larger of the two items
template <typename T>
T maximal(const T& a, const T& b);

//swap the two items
template <typename T>
void swap(T& a, T& b);

//return index of the largest item in data
template <typename T>
int indexOfMaximal(T data[ ], int n);

//insert entry into the sorted array data with length n.
template <typename T>
void orderedInsert(T data[ ], unsigned& n, T entry);

//return the first element in data that is /not less than entry
template <typename T>
int firstGE(const T data[ ], int n, const T& entry);

//append entry to the right of data
template <typename T>
void attachItem(T data[ ], unsigned& n, const T& entry);

//insert entry at index i in data
template <typename T>
void insertItem(T data[ ], int i, unsigned& n, T entry);

//remove the last element and return it.
template <typename T>
T detachItem(T data[ ], unsigned& n);

//delete item at index i return it.
template <typename T>
T deleteItem(T data[ ], int i, unsigned& n);

//append data2 to the back of data1
template <typename T>
void mergeArrays(T dest[], unsigned& destSize, T source[], unsigned& sourceSize);

//insert data2 at the front of data1
template <typename T>
void mergeFront(T dest[ ], unsigned& destSize, T source[ ], unsigned& sourceSize);

//move n/2 elements from the right of data1 and move to data2
template <typename T>
void split(T data1[ ], unsigned& n1, T data2[ ], unsigned& n2, bool includeMid = false);

//copy src[] into dest[]
template <typename T>
void copyArray(T dest[], const T src[], int& dest_size, int src_size);

//print array data
template <typename T>
void printArray(const T data[], int n);

//the range: [start, end] of a will be printed.
template <typename T>
void printArraySegment(T *a, int start, int end);

//item > all data[i]
template <typename T>
bool isGt(const T data[], int n, const T& item);

//item <= all data[i]
template <typename T>
bool isLe(const T data[], int n, const T& item);

//shuffle the contents of the array
template <typename T>
void shuffleArray(T * a, int size);

//verify that the array is sorted.
template<typename T>
bool arrayIsSorted(const T [], size_t size);

template <typename T>
ostream& operator <<(ostream& outs, const vector<T>& list); //print vector list

template <typename T, typename U>
vector<T>& operator +=(vector<T>& list, const U& addme); //list.push_back

//preconditions: a is allocated to have at least size elements
//postconditions: returns true if, a[i] <= a[i+1] for all i in: [0,size-1), otherwise false.
template<typename T>
bool arrayIsSorted(const T a[], size_t size)
{
    bool inOrder = true;
    if(size > 1)
    {
        for(size_t i = 0; i < size-1 && inOrder; i++)
        {
            if(a[i] > a[i+1])
                inOrder = false;
        }
    }

    return inOrder;
}

//preconditions: none
//postconditions: return the larger of the two items
template <typename T>
T maximal(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

//preconditions: none
//postconditions: swap the two items
template <typename T>
void swap(T& a, T& b)
{
    T temp;
    temp = a;
    a = b;
    b = temp;
}

//preconditions: none
//postconditions: return index of the largest item in data
template <typename T>
int indexOfMaximal(T data[], int n)
{
    assert(n > 0);

    int largestIndex = 0;
    T largestItem = data[0];

    for(int i = 1; i < n; ++i)
    {
        if(data[i] > largestItem)
        {
            largestIndex = i;
            largestItem = data[i];
        }
    }

    return largestIndex;
}

//preconditions: none
//postconditions: insert entry into the sorted array data with length n
template <typename T>
void orderedInsert(T data[], unsigned& n, T entry)
{
    int insertAt = firstGE(data,n,entry);
    //shift all items to the right of insertAt to the right.
    for(int i = n-1; i >= insertAt; i--)
        data[i+1] = data[i];

    data[insertAt] = entry;
    ++n;
}

//preconditions: none.
//postconditions: return the index of the first element in data that is not less than entry
// if no such element exists, returns n.
template <typename T>
int firstGE(const T data[], int n, const T& entry)
{
    int indexOfGE = 0;
    bool found = false;
    for(int i = 0; i < n && !found; i++)
    {
        if(data[i] >= entry)
        {
            indexOfGE = i;
            found = true;
        }
    }
    //in this case, the entry is larger than all other items in the array.
    if(!found)
        indexOfGE = n;

    return indexOfGE;
}

//preconditions: none
//postconditions: append entry to the right of data
template <typename T>
void attachItem(T data[], unsigned& n, const T& entry)
{
    data[n] = entry;
    ++n;
}

//preconditons: data[] has a capacity of at least n+1
//postconditions: the entry is inserted to data at data[i]
// and everything right and including data[i] is shifted right.
template <typename T>
void insertItem(T data[], int i, unsigned& n, T entry)
{
    for(int j = n; j > i; --j)
        data[j] = data[j-1];

    data[i] = entry;
    ++n;
}

//preconditions: none
//postconditon: returns the last element in data, decrementing n.
template <typename T>
T detachItem(T data[], unsigned& n)
{
    T item = data[n-1];
    n--;
    return item;
}

//preconditions: i < n
//postconditions: delete data[i], shifting everything to the right of i left.
// then decrement n and return the item.
template <typename T>
T deleteItem(T data[], int i, unsigned& n)
{
    T item = data[i];
    //shift
    for(int j = i; j < n-1; j++)
        data[j] = data[j+1];
    --n;
    return item;
}

//data1 = dest, data2 = source, n1 = destSize, n2 = sourceSize
//preconditions: data1[] has at least a capacity of n1 + n2.
//postconditions: append data2 to the right of data1.
// then, add n2 to n1, and set n2 to 0.
template <typename T>
void mergeArrays(T dest[], unsigned& destSize, T source[], unsigned& sourceSize)
{
    for(int i = destSize; i < destSize + sourceSize; ++i)
        dest[i] = source[i-destSize];

    destSize += sourceSize;
    sourceSize = 0;
}

//data1 = dest, data2 = source, n1 = destSize, n2 = sourceSize
//preconditions: data1[] has at least a capacity of n1 + n2.
//postconditions: shift all items in data1[] to the right by n2.
// then insert data2 at the front of data1, add n2 to n1,
// and set n2 to 0.
template <typename T>
void mergeFront(T dest[ ], unsigned& destSize, T source[ ], unsigned& sourceSize)
{
    //shift all items in data1 over by n2.
    for(int i = destSize-1; i >= 0; --i)
        dest[i+sourceSize] = dest[i];

    destSize += sourceSize;

    for(int i = 0; i < sourceSize; i++)
        dest[i] = source[i];

    sourceSize = 0;
}

//preconditions: none
//postconditins: move mid elements from the right of data1 to data2,
// where mid is defined as: (n1+1)/2, setting n2 = n1 - mid, and n1 = mid.
template <typename T>
void split(T data1[], unsigned& n1, T data2[], unsigned& n2, bool includeMid)
{
    if(includeMid)
    {
        //the index of the first item in data1 to be copied to data2.
        int mid = (n1+1)/2;

        for(int i = mid-1; i < n1; i++)
            data2[i-mid+1] = data1[i];

        n2 = n1 - mid+1;
        n1 = mid-1;
    }
    else
    {
        //the index of the first item in data1 to be copied to data2.
        int mid = (n1+1)/2;

        for(int i = mid; i < n1; i++)
            data2[i-mid] = data1[i];

        n2 = n1 - mid;
        n1 = mid;
    }
}

//preconditions: none
//postconditions: copy src[] into dest[], setting dest_size to src_size.
template <typename T>
void copyArray(T dest[], const T src[], unsigned& dest_size, unsigned src_size)
{
    for(int i = 0; i < src_size; i++)
        dest[i] = src[i];

    dest_size  = src_size;
}

//preconditions: pos < n.
//postconditions: prints data[], if pos is provided, prints: ^ under data[pos].
template <typename T>
void printArray(const T data[], int n)
{
    cout << "[";
    for(int i = 0; i < n; ++i)
        cout << "(" << data[i] << ") ";
    cout << "]" << endl;
}

//preconditions: end > start
//postconditions: the range: [start, end] of a will be printed.
template <class T>
void printArraySegment(T *a, int start, int end)
{
    assert(end >= start);
    cout << "[";
    for(int i = start; i < end; i++)
        cout << a[i] << ", ";
    cout << a[end] << "]" << endl;
}

//preconditions: none
//postconditions: returns true, if: item > all data[i], otherwise false.
template <typename T>
bool isGt(const T data[], int n, const T& item)
{
    bool isLargest = true;
    for(int i = 0; i < n && isLargest; ++i)
        if(item <= data[i])
            isLargest = false;

    return isLargest;
}

//preconditions: none
//postconditions: returns true, if: item <= all data[i], otherwise false.
template <typename T>
bool isLe(const T data[], int n, const T& item)
{
    bool isLessOrEqual = true;
    for(int i = 0; i < n && isLessOrEqual; ++i)
        if(item > data[i])
            isLessOrEqual = false;

    return isLessOrEqual;
}

//preconditions: none
//postconditions: the array recieved will be shuffled.
template<typename T>
void shuffleArray(T * a, int size)
{
    //for each: 0 <= i < size-1
    //swap a[i] with a[j], where i <= j < size
    for(int i = 0; i < size-1; i++)
        std::swap(a[i],a[(rand() % (size-i))+i]);
}

//-------------- Vector Extra operators: ---------------------
template <typename T>
ostream& operator <<(ostream& outs, const vector<T>& list)
{
    outs << "[";
    if(!list.empty())
    {
        for(unsigned i = 0; i < list.size()-1; i++)
            outs << list.at(i) << ", ";
        outs << list.back();
    }
    outs << "]";
    return outs;
}

template <typename T, typename U>
vector<T>& operator +=(vector<T>& list, const U& addme) //list.push_back
{
    list.push_back(addme);
    return list;
}


#endif // ARRAYUTIL_H
