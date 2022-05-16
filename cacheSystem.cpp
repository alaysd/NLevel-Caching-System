// Problem statement: Machine coding for N-level caching system.

#include<bits/stdc++.h>

using namespace std;

// Provides metadata related to each level in our cache system.
struct LevelDescription {
    int capacity;
    int readTime, writeTime;
    string evictionPolicy;
    LevelDescription(int capacity, int readTime, int writeTime, string evictionPolicy) {
        this->capacity = capacity;
        this->readTime = readTime;
        this->writeTime = writeTime;
        this->evictionPolicy = evictionPolicy;
    }
};

struct ReadResponse {
    string value;
    int time;
    ReadResponse(string value, int time) {
        this->value = value;
        this->time = time;
    }
};

struct WriteResponse {
    int time;
    WriteResponse(int time) {
        this->time = time;
    }
};

// Interface which provides read and write operations and the implemenation is based on the object what the Level pointer gets during initialization. Can be LRU / LFU or any other we have implemented. 
class Level {
public:
    map<string, string> data;
    int capacity;
    int readTime, writeTime;
    // Above data should have protected access modifier but for helperToPrintData() we have kept it here.

    Level(LevelDescription ld) {
        this->capacity = ld.capacity;
        this->readTime = ld.readTime;
        this->writeTime = ld.writeTime;
    }

    virtual ReadResponse read(string key) = 0;
    virtual WriteResponse write(string key, string value) = 0;

};

// LRU implementation of cache level.
class LRU: public Level {
private:
    map<string, list<string>::iterator> dataLocation;
    list<string> dataTrack;
public:
    
    LRU(LevelDescription ld) : Level(ld) {
        
    };

    // Kept the response time of read and write at Level at that level only. Aggreagation of different times logic is kept at CACHE System level.

    ReadResponse read(string key) {
        if(data.find(key) == data.end()) return ReadResponse("", readTime);

        auto locationOfData = dataLocation[key];

        dataTrack.erase(locationOfData);
        dataTrack.push_back(key);
        dataLocation[key] = --dataTrack.end();

        return ReadResponse(data[key], readTime);
    }

    WriteResponse write(string key, string value) {
        WriteResponse wr(0);
        
        if(data.find(key) == data.end()) {

            if(data.size() == capacity) {
                auto it = dataTrack.front();
                data.erase(it);
                dataLocation.erase(it);
                dataTrack.pop_front();
            } 

            data[key] = value;
            dataTrack.push_back(key);
            dataLocation[key] = --dataTrack.end();
            wr.time += writeTime;
            
            return wr;

        } else {
            
            ReadResponse rr = read(key);

            if(rr.value == value) {
                // Returns 0 if already present in the current level cache;
                return wr;
            }

            data[key] = value;
            wr.time += writeTime;
            
            return wr;

        }
    }
};


// Contains information about all the caching mechanism available in our system. Upon implementation of new cache mechanism we can also add it to the list
class cacheTypes {
private: 
    map<string, string> descritionOfCache;
    map<string, int> code;
public:

    cacheTypes() {

    }

    void insertNewType(string type, string description) {
        descritionOfCache[type] = description;
        code[type] = code.size()+1; 
    }

    Level* getCache(LevelDescription ld) {
        int codeOfCache = code[ld.evictionPolicy];
        switch (codeOfCache) {
        case 1: return new LRU(ld);
        // case 2: return new LFU();
        default: return nullptr;
        }
        return nullptr;
    }

};

cacheTypes* cacheTypesAvailable;
     

// Main system.
class CACHE {
private:
    int totalLevels;
    vector<Level*> levels;
    // Return time needed to write on all the previous levels of cahce.
    int writeToPreviousLevels(string key, string val, stack<Level*> prevLevels) {
        int timeTakenToWrite = 0;
        while(prevLevels.size()) {
            WriteResponse wr = prevLevels.top()->write(key, val);
            prevLevels.pop();
            timeTakenToWrite += wr.time;
        }
        return timeTakenToWrite;
    }
public:
    CACHE(int totalLevels, vector<LevelDescription> levelsDescription) {
        this->totalLevels = totalLevels;
        levels = vector<Level*>(totalLevels);
        for(int i = 0 ; i < levelsDescription.size() ; i++) {
            levels[i] = cacheTypesAvailable->getCache(levelsDescription[i]);
        }
    }

    // Aggregates the needed read and write times needed for READ opeartion.
    ReadResponse read(string key) {
        
        stack<Level*> prevs;
        int returnTime = 0;
        string answerValue = "";

        for(auto l : levels) {
            auto currRead = l->read(key);
            returnTime += l->readTime;
            if(currRead.value == "") {
                // not found;
                prevs.push(l);
            } else {
                // found
                answerValue = currRead.value;
                returnTime += writeToPreviousLevels(key, answerValue, prevs);
                ReadResponse answer(answerValue, returnTime);
                return answer;
            }
        }

        // On KEY NOT FOUND, returns the read times on all the cache levels till end.
        ReadResponse answer("KEY NOT FOUND", returnTime);

        return answer;
    }
    
    // Aggregates Write time along with read.
    WriteResponse write(string key, string value) {
        WriteResponse wr(0);
        for(auto level : levels) {

            auto rr = level->read(key);
            wr.time += rr.time;
            if(rr.value == value) {
                return wr;
            }
            auto currWR = level->write(key, value);
            wr.time += currWR.time;
        }

        return wr;

    }

    void helperToPrintData() {
        cout << "************************" << endl;
        for(auto l : levels) {
            for(auto d : l->data) {
                cout << d.first << " ";
            }
            cout << endl;
        }
        cout << "************************" << endl;
    }


};



   
int main() {

    // System initialization.
    vector<LevelDescription> allLevels { LevelDescription(2, 1, 10, "LRU"), LevelDescription(4, 2, 20, "LRU"), LevelDescription(8, 3, 30, "LRU") };
    cacheTypesAvailable = new cacheTypes();
    cacheTypesAvailable->insertNewType("LRU", "Least recently used");
    cacheTypesAvailable->insertNewType("LFU", "Least frequently used");
    

    CACHE* nLevel = new CACHE(3, allLevels); 

    
    // Sample testing.

    // Initializing data
    nLevel->write("1", "1");
    nLevel->write("2", "2");
    nLevel->write("3", "3");
    nLevel->write("4", "4");
    nLevel->write("5", "5");
    nLevel->write("6", "6");
    nLevel->write("7", "7");
    nLevel->write("8", "8");

    nLevel->helperToPrintData();

    // Fetching 1. EXPECTED VALUE: (1+10) + (2+20) + (3+0)
    auto fetch = nLevel->read("1");
    cout << "Fetch :: 1 :: EXPECTED TIME: (1+10) + (2+20) + (3+0) = 36 :: ACTUAL TIME :: " << fetch.time << " || OUTPUT VALUE = " << fetch.value << endl;
    nLevel->helperToPrintData();

    // Fetching 7. 
    fetch = nLevel->read("7");
    cout << "Fetch :: 7 :: EXPECTED TIME: (1+10) + (2+0) = 13 :: ACTUAL TIME :: " << fetch.time << " || OUTPUT VALUE :: "<< fetch.value  << endl;
    nLevel->helperToPrintData();

    // Fetching 11
    fetch = nLevel->read("11");
    cout << "Fetch :: 11 :: EXPECTED TIME :: (1+0) + (2+0) + (3+0) = 6 :: ACTUAL TIME = " << fetch.time << " :: || OUTPUT VALUE :: " << fetch.value << endl;
    nLevel->helperToPrintData();

    // Write 20
    auto putting = nLevel->write("20", "20");
    cout << "Putting value 20 :: EXPECTED TIME :: (1+10) + (2+20) + (3+30) = 66 :: ACTUAL TIME = " << putting.time << endl;
    nLevel->helperToPrintData();

    // Fetching 6. 
    fetch = nLevel->read("6");
    cout << "Fetch :: 6 :: EXPECTED TIME :: (1+10) + (2+20) + (3+0) = 36 :: ACTUAL TIME = " << fetch.time << " :: || OUTPUT VALUE :: " << fetch.value << endl;
    nLevel->helperToPrintData();

    // Write 7
    putting = nLevel->write("7", "7");
    cout << "Putting value 7 :: EXPECTED TIME :: (1+10) + (2+0) + (0+0) = 13 :: ACTUAL TIME = " << putting.time << endl;
    nLevel->helperToPrintData();

    // Write 20 with different value.
    putting = nLevel->write("20", "7");
    cout << "Putting value 20 with value 7 :: EXPECTED TIME :: (1+10) + (2+20) + (3+30) = 66 :: ACTUAL TIME = " << putting.time << endl;
    nLevel->helperToPrintData();



}