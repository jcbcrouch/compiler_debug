#include "index.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Utf8Uint.h"
#include <deque>


void* readerWrapper(void* index){
    static_cast<Index*>(index)->reader();
}

void* writerWrapper(void* index){
    static_cast<Index*>(index)->writerDriver();
}


Index::Index(std::deque<Doc_object>* docQueue, threading::Mutex* queueLock, threading::ConditionVariable* CV)
:currentLocation(0), totalDocLength(0), currentDocId(0), urlMap(String("urlTable")), metaMap(String("metaTable")), currentWriteDocId(0), readThreads(10), writeThreads(10), emptyQueue(false) {
    
    fd = open("averageDocLength.bin", O_RDWR | O_CREAT, S_IRWXU);
    ftruncate(fd, sizeof(unsigned long long));
    documentQueue = docQueue;
    documentQueueLock = queueLock;
    dequeCV = CV;
    //read in
    for(unsigned i = 0; i < 10; i++){
        pthread_create(&readThreads[i], NULL, &readerWrapper, this);
    }
    for(unsigned i = 0; i < 10; i++){
        pthread_create(&writeThreads[i], NULL, &writerWrapper, this);
    }
    //reader(docQueue);
}

void Index::writerDriver(){
    //put them in a priority queue that holds wordLocations, sorted by numWords
    //pop value from priority queue to addWord
    wordLocations* locations;
    while(true){
        pQueueLock.lock();
        while(queue.size() == 0){
            emptyQueue = true;
            queueReadCV.wait(pQueueLock);
        }
        locations = queue.top();
        //now that locaitons is out of queue we remove the entry
        queue.pop();
        pQueueLock.unlock();
        //pass fixed length word and location vector to AddPostings
        Postings* postings = Postings::GetPostings();
        postings->AddPostings(FixedLengthString(locations->word.CString()), &locations->locations);
        delete locations;
    }
    
}


void Index::reader(){
    //could this be threaded?
    //    would have to make sure thread driver only happens when all document older than the newest currently being read are completely read.
    //    reader must add to queue in correct order, has to wait for readers of old docs to finish if necessary
    while(true){
        documentQueueLock->lock();
        while(documentQueue->empty()){
            documentQueueLock->unlock();
            documentQueueLock->lock();
            
        }
        //think about dynamicness
        Doc_object doc = documentQueue->front();
        documentQueue->pop_front();
        unsigned long long startLocation = currentLocation;
        int docSize = doc.Words.size();
        //every doc end is itarconv: No such file or directory own location, 1 for regular doc + 1 for each anchor text
        docSize += doc.anchor_words.size() + 1;
        for(unsigned i = 0; i < doc.anchor_words.size(); i++){
            docSize += doc.anchor_words[i].size();
        }
        currentLocation += docSize;
        int docId = currentDocId;
        currentDocId++;
        //can't read in next doc until current location and currentDocId are updated
        documentQueueLock->unlock();
        hash_table<Vector<unsigned long long> > localMap;
        //pass urls and doc ends to newDoc somehow, probably a queue of url, docEnd pairs
        //parse into word, vector<ull>location pairs
        for(unsigned i = 0; i < doc.url.size(); i++){
            (*(localMap[String("$") + doc.url[i]])).push_back(startLocation);
            startLocation++;
        }
        for(unsigned i = 0; i < doc.Words.size(); i++){
            //proabbly going to need a map here
            //if statement for where in the doc this occured
            //@-anchor
            //#-title
            //$-url
            //*-body
            if(doc.Words[i].type == 't'){
                (*(localMap[String("#") + doc.Words[i].word])).push_back(startLocation);
                startLocation++;
            }
            else{
                (*(localMap[String("*") + doc.Words[i].word])).push_back(startLocation);
                startLocation++;
            }
        }
        //docEnd
        (*(localMap[String("")])).push_back(startLocation);
        urlMap[startLocation] = FixedLengthURL(doc.doc_url.CString());
        startLocation++;
        //parse anchor texts
        for(unsigned i = 0; i < doc.anchor_words.size(); i++){
            for(unsigned j = 0; j < doc.anchor_words[i].size(); j++){
                //probably gonna need to use the same map here
                (*(localMap[String("@") + doc.anchor_words[i][j].word])).push_back(startLocation);
                startLocation++;
                
            }
            //docEnd map here
            (*(localMap[String("")])).push_back(startLocation);
            urlMap[startLocation] = FixedLengthURL(doc.Links[i].CString());
            startLocation++;
        }
        urlMetadata metadata(doc.Words.size(), doc.doc_url.Size(), doc.num_slash_in_url, metaMap[FixedLengthURL(doc.doc_url.CString())].inLinks, doc.anchor_words.size(),doc.domain_type, doc.domain_rank);
        currentWriteDocIdMutex.lock();
        while(currentWriteDocId != docId){
            queueWriteCV.wait(currentWriteDocIdMutex);
        }
        currentWriteDocIdMutex.unlock();
        pQueueLock.lock();
        //url -> metadata
        metaMap[FixedLengthURL(doc.doc_url.CString())] = metadata;
        for(unsigned i = 0; i < doc.anchor_words.size(); i++){
            metaMap[FixedLengthURL(doc.Links[i].CString())].inLinks++;
        }
        totalDocLength += doc.Words.size();
        unsigned averageDocLength = totalDocLength / (docId + 1);
        lseek(fd, 0, SEEK_SET);
        write(fd, &averageDocLength, sizeof(unsigned));
        //iterate through map and insert into pQueue
        for(unsigned i = 0; i < localMap.numBuckets; i++){
            auto it = localMap.array[i].begin();
            while(it != localMap.array[i].end()){
                //add word vector<ull>location pair to priority queue
                queue.insert((*it).key, (*it).offset);
                it++;
            }
        }
        
        pQueueLock.unlock();
        if(emptyQueue){
            emptyQueue = false;
            queueReadCV.broadcast();
        }
        queueWriteCV.broadcast();
        dequeCV->signal();
        currentWriteDocIdMutex.lock();
        currentWriteDocId++;
        currentWriteDocIdMutex.unlock();
    }
}