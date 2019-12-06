# On-Disk-BTree

This implementation is not optimized, I will most likely not spend much more time on this project. Instead, I plan to implement a more useful on-disk b+tree.

For the most part, the BTree code was not substantially changed from my in-memory version.
The OnDiskBTree class itself is not aware that the nodes are stored on disk, 
whenever it needs to access a node it must make a request from a record manager class,
which uses a naive LRU (least recently used) cache implementation to reduce disk i/o.

The record manager also maintains a seperate file of pages that were previously deleted that will be reused before appending a new page to the file. This measure reduces the amount of wasted space in the data file.
