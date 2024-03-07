#include <string>
#include <cstdint>
#include <inttypes.h>
#include <iostream>
#include "pin.H"

// 
#define INS_CACHE 0
// 
#define DATA_CACHE 1

// #define MEMORY_READ 0
// #define MEMORY_WRITE 1
// #define MEMORY_OP_READ 2
// #define MEMORY_OP_WRITE 3

class Entry{

    private:
        // the data to be stored in the cache, in this case its instructions
        VOID* data;
        // the tag which is used to identify which memory address coresponds to a set
        VOID* tag;
        // used to track what Set was least recently loosed if all sets in line are full
        uint64_t lru;
        // used to mark if a set is valid or can be removed from the cache
        bool validBit;

    public:
        Entry(){
            data = NULL;
            tag = NULL;
            lru = 0;
            validBit = false;
        }

        VOID* getData(){
            return data;
        }

        VOID* getTag(){
            return tag;
        }

        uint64_t getLRU(){
            return lru;
        }

        bool getValid(){
            return validBit;
        }

        VOID setValid(bool valid){
            validBit = valid;
        }

        VOID setLRU(uint64_t lru){
            this->lru = lru;
        }

        VOID setTag(VOID* tag){
            this->tag = tag;
        }

        VOID setData(VOID* data){
            this->data = data;
        }

};

class Cache{

    private:

        // type of the cache, instruction or data
        int type;

        // lines and acossiativity of cache
        int lines;
        int acossiativity;

        // internal counter that minics clock, used for lru
        uint64_t clock;

        // file used for debug statments
        FILE* outputFile;

        // 2D array of Entry struct which is the cache
        Entry** cache;

        VOID insertInstruction(VOID* address, VOID* data){

            //get the 4-12 bits of 64 bit address which is the line index
            uint64_t lineIndex = ((uint64_t)address >> acossiativity) & ((1 << lines) - 1);

            //variables to keep track of canidates for insertion if set is full
            int invalidBlockIndex = -1;
            int lruBlockIndex = -1;

            // set the smallest lru to lru of first block in set
            uint64_t smallestLRU = cache[lineIndex][0].getLRU();
            if(lineIndex && invalidBlockIndex && lruBlockIndex && smallestLRU){}
            //iterate over all blocks in set
            for(int setIndex = 0; setIndex < (1 << acossiativity); setIndex++){

                // if there is a cache hit replace the data
                if(cache[lineIndex][setIndex].getTag() == address){

                    // print that there is a cache hit and you are replacing the block
                    fprintf(outputFile, "Hit! Replacing ");
                    // insert the data
                    insertHelper(lineIndex, setIndex, data, address);
                    return;
                }

                // // if there is an empty entry insert into that entry
                if(!cache[lineIndex][setIndex].getData()){

                    // print that it is fresh insertion
                    fprintf(outputFile, "Inserting ");
                    // insert data
                    insertHelper(lineIndex, setIndex, data, address);
                    return;
                }

                // if invalid entry mark it as canidate
                if(!cache[lineIndex][setIndex].getValid()){
                    invalidBlockIndex = setIndex;
                }

                // if least recently used mark as canidate
                if(cache[lineIndex][setIndex].getLRU() < smallestLRU){
                    smallestLRU = cache[lineIndex][setIndex].getLRU();
                    lruBlockIndex = setIndex;
                }
            }

            // if there were not hits or empty slots insert into optimal canidate
            // prioritize invalid blocks to be replaced
            int canidateIndex = invalidBlockIndex != -1 ? invalidBlockIndex : lruBlockIndex;
            fprintf(outputFile, "Set full, Inserting ");
            // if(canidateIndex){}
            insertHelper(lineIndex, canidateIndex, data, address);
        }

        VOID insertData(VOID* address, VOID* data, bool read){
            //get the 4-12 bits of 64 bit address which is the line index
            uint64_t lineIndex = ((uint64_t)address >> acossiativity) & ((1 << lines) - 1);

            //variables to keep track of canidates for insertion if set is full
            int invalidBlockIndex = -1;
            int lruBlockIndex = -1;

            // set the smallest lru to lru of first block in set
            uint64_t smallestLRU = cache[lineIndex][0].getLRU();
            if(lineIndex && invalidBlockIndex && lruBlockIndex && smallestLRU){}
            //iterate over all blocks in set
            for(int setIndex = 0; setIndex < (1 << acossiativity); setIndex++){

                // if there is a cache hit check check if reading or writing
                if(cache[lineIndex][setIndex].getTag() == address){

                    // if cache hit on read update LRU
                    if(read){
                        fprintf(outputFile, "Hit! Read %p\n", address);
                        cache[lineIndex][setIndex].setLRU(clock);
                    }
                    // If cache hit on write change value for cache
                    else{
                        // print that there is a cache hit and you are overwriting the block
                        fprintf(outputFile, "Hit! Memory Write, Replacing ");
                        // insert the data
                        insertHelper(lineIndex, setIndex, data, address);
                        return;
                    }
                }

                // if there is an empty entry insert into that entry if it is a write command
                if(!cache[lineIndex][setIndex].getData() && !read){

                    // print that it is fresh insertion
                    fprintf(outputFile, "Miss! Memory Write, Inserting ");
                    // insert data
                    insertHelper(lineIndex, setIndex, data, address);
                    return;
                }

                // if invalid entry mark it as canidate
                if(!cache[lineIndex][setIndex].getValid()){
                    invalidBlockIndex = setIndex;
                }

                // if least recently used mark as canidate
                if(cache[lineIndex][setIndex].getLRU() < smallestLRU){
                    smallestLRU = cache[lineIndex][setIndex].getLRU();
                    lruBlockIndex = setIndex;
                }
            }

            // if there were not hits or empty slots insert into optimal canidate if cache write
            if(!read){
                // prioritize invalid blocks to be replaced
                int canidateIndex = invalidBlockIndex != -1 ? invalidBlockIndex : lruBlockIndex;
                fprintf(outputFile, "Set full, Inserting ");
                // if(canidateIndex){}
                insertHelper(lineIndex, canidateIndex, data, address);
            }
        }

        VOID insertHelper(int lineIndex, int setIndex, VOID* data, VOID* tag){
            cache[lineIndex][setIndex].setData(data);
            cache[lineIndex][setIndex].setTag(tag);
            cache[lineIndex][setIndex].setLRU(clock);
            cache[lineIndex][setIndex].setValid(true);
            if(type == INS_CACHE){
                fprintf(outputFile, "%p, %s\n", tag, (*((std::string*)data)).c_str());
            }
            else{
                fprintf(outputFile, "%p, %p\n", tag, data);
            }
        }

    public:

        Cache(int lines, int acossiativity, int type){
            this->type = type;
            this->lines = lines;
            this->acossiativity = acossiativity;

            // open output file to write cache operations
            if(type == INS_CACHE){
                outputFile = fopen("instructionCache.out", "w");
            }
            else{
                outputFile = fopen("dataCache.out", "w");
                // fprintf(outputFile, "meer");
            }

            clock = 0;
            
            cache = new Entry*[(1 << lines)];
            for(int idx = 0; idx < (1 << lines); idx++){
                cache[idx] = new Entry[(1 << acossiativity)];
            }
        }

        ~Cache(){
            fprintf(outputFile, "#eof\n");
            fclose(outputFile); 
            delete[] cache;
        }

        VOID clearCache(){

            // go through all values in cache and set to defualt values
            for(int idx = 0; idx < 1 << lines; idx++){
                for(int itor = 0; itor < 1 << acossiativity; itor++){
                    cache[idx][itor].setData(NULL);
                    cache[idx][itor].setTag(NULL);
                    cache[idx][itor].setLRU(0);
                    cache[idx][itor].setValid(false);
                }
            }

            // print cache clear occured
            fprintf(outputFile, "#######################\n###                 ###\n###  Cleared Cache  ###\n###                 ###\n#######################\n");
        }

        VOID insert(VOID* address, VOID* data, BOOL read){

            clock++;

            if(type == INS_CACHE){
                insertInstruction(address, data);
            }
            else{
                // if(read){
                //     fprintf(outputFile, "meer\n");
                // }
                // else{
                //     fprintf(outputFile, "sadge\n");
                // }
                // fprintf(outputFile, "meer\n");
                insertData(address, data, read);
            }
        }
};

