#include "String.h"
#include "vector.h"
#include "threading.h"
#include "PersistentHashMap.h"
#include "Pair.h"
#include "StringView.h"
#include "PostingList.h"
#include "Postings.h"
#include "PriorityQueue.h"
#include "Parser.hpp"
#include "hash_table.hpp"
#include <deque>


class Index{
public:
    Index(std::deque<Doc_object>* docQueue, threading::Mutex* queueLock, threading::ConditionVariable* CV);
    void reader();
    void writerDriver();
    
private:
    //FUNCTIONS
    
    //returns blocks that contains word's posting list
    //if posting list does not exist creates it, immediately updates blocks word index to hold this word
    //VARIABLES
    int fd;
    PriorityQueue queue;
    //queue of doc objects from parser
    std::deque<Doc_object>* documentQueue;
    unsigned long long totalDocLength;
    struct urlMetadata{
        int length;
        int urlLength;
        int urlSlashes;
        int inLinks;
        int outLinks;
        char domain;
        unsigned importance;
        urlMetadata()
        :length(0), urlLength(0), urlSlashes(0), inLinks(0), outLinks(0), domain('x'), importance(745000){}
        urlMetadata(int length, int urlLength, int urlSlashes, int inLinks, int outLinks, char domain, unsigned importance)
        :length(length), urlLength(urlLength), urlSlashes(urlSlashes), inLinks(inLinks), outLinks(outLinks), domain(domain), importance(importance){}
        urlMetadata(const urlMetadata& u)
        :length(u.length), urlLength(u.urlLength), urlSlashes(u.urlSlashes), inLinks(u.inLinks), outLinks(u.outLinks), domain(u.domain), importance(u.importance){}
    };
    //map will prob be moved to scheduler
    //FIGURE OUT HOW TO STORE URLS
    PersistentHashMap<unsigned long long, FixedLengthURL> urlMap;
    PersistentHashMap<FixedLengthURL, urlMetadata> metaMap;
    //current absolute location on the web
    unsigned long long currentLocation;
    //true if priority queue is empty
    bool emptyQueue;
    int currentDocId;
    //reader() waits until its docId = currentWriteDocId to push to the pQueue
    int currentWriteDocId;
    
    //THREADING
    Vector<pthread_t> readThreads;
    Vector<pthread_t> writeThreads;
    threading::ConditionVariable* dequeCV;
    threading::ConditionVariable queueReadCV;
    threading::ConditionVariable queueWriteCV;
    threading::Mutex pQueueLock;
    threading::Mutex currentLocationMutex;
    threading::Mutex currentWriteDocIdMutex;
    threading::Mutex* documentQueueLock;
};
