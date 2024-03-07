#include <iostream>
#include <fstream>
#include "pin.H"
#include "Cache.cpp"
#include <cstdint>
#include <inttypes.h>
using std::cerr;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

// #define MEMORY_READ 0
// #define MEMORY_WRITE 1
// #define MEMORY_OP_READ 2
// #define MEMORY_OP_WRITE 3

// initalize output stream to write to output file in future
// FILE* outputFile;

Cache cache(9, 3, DATA_CACHE);

VOID cacheDisbatch(VOID* insAddr, VOID* memAddr, UINT32 memOp, VOID* voidCache, BOOL read){
    // Cache* cache = (Cache*)voidCache;
    cache.insert(memAddr, insAddr, read); // 16
    // if(operation){
    //     fprintf(outputFile, "meer\n");
    // }
    // else{
    //     fprintf(outputFile, "sadge\n");
    // }
    
}

// VOID instructionHandler(VOID* address, VOID* data, VOID* voidCache){
//     Cache* cache = (Cache*)voidCache;
//     cache->insert(address, data); // 5
// }

// VOID memOpWriteHander(VOID* insAddr, VOID* memAddr, UINT32 memOp, VOID* voidCache){
//     //Cache* cache = (Cache*)voidCache;
//     // if(cache){}
//    // cache->insert(memAddr, insAddr); // 3
//     // fprintf(outputFile, "%p: OR %p\n", insAddr, memAddr);
// }

// VOID memOpReadHander(VOID* insAddr, VOID* memAddr, UINT32 memOp, VOID* voidCache, UINT32 operation){
//     //Cache* cache = (Cache*)voidCache;
//     // if(cache){}
//    // cache->insert(memAddr, insAddr); // 5
//     // fprintf(outputFile, "%p: OR %p\n", insAddr, memAddr);
// }

// VOID memWriteHandler(VOID* insAddr, VOID* memAddr, VOID* voidCache){
//     //Cache* cache = (Cache*)voidCache;
//     // if(cache){}
//     // cache->insert(memAddr, insAddr); // 5
//     // fprintf(outputFile, "%p: OR %p\n", insAddr, memAddr);
// }

// VOID memReadHandler(VOID* insAddr, VOID* memAddr, VOID* voidCache){
//     Cache* cache = (Cache*)voidCache;
//     if(cache){}
//     //cache->insert(memAddr, insAddr); // 7
//     // fprintf(outputFile, "%p: OR %p\n", insAddr, memAddr);
// }

/*
    Instruction is a pin callback function so it must have parameters,
    INS_INSTRUMENT_CALLBACK: function
    VOID*: value
*/
VOID Instruction(INS ins, VOID* voidCache){

    // std::string instructionString = INS_Disassemble(ins);

    // fprintf(outputFile, "%s\n", instructionString.c_str());
    // Cache* cache = (Cache*)voidCache;

    UINT32 memOperands = INS_MemoryOperandCount(ins);

    for (UINT32 memOp = 0; memOp < memOperands; memOp++){
        if (INS_MemoryOperandIsRead(ins, memOp)){
            // fprintf(outputFile, "%d\n", (UINT32)MEMORY_OP_READ);
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cacheDisbatch, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp, IARG_PTR, voidCache, IARG_BOOL, true, IARG_END);
        }
        if(INS_MemoryOperandIsWritten(ins, memOp)){
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cacheDisbatch, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp, IARG_PTR, voidCache, IARG_BOOL, false, IARG_END);
        }
    }
    // if(INS_IsMemoryWrite(ins)){
    //     INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cacheDisbatch, IARG_INST_PTR, IARG_MEMORYWRITE_EA, memOperands, IARG_PTR, voidCache, IARG_BOOL, false, IARG_END);
    // }
    // if(INS_IsMemoryRead(ins)){
    //     INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)cacheDisbatch, IARG_INST_PTR, IARG_MEMORYREAD_EA, memOperands, IARG_PTR, voidCache, IARG_BOOL, true, IARG_END);
    // }
    
    // INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)instructionHandler, IARG_INST_PTR, IARG_PTR, (VOID*)(new std::string(instructionString)), IARG_PTR, voidCache, IARG_END);

    /*
    INS_InsertCall = inset call function with parameters
    INS: instruction = Instruction to instrument 
    IPOINT: action = specifies before or after
    AFUNPTR: funptr = list of arguments to pass to funptr must end with IARG_END
    */
}

/*
    Finish function is a FINI_CALLBACK function so it takes paremeters:
    INT32: code = O/S specific termination code 
    VOID*: value = The tool's call-back value
*/
VOID FinishFunction(INT32 code, VOID* v){

    // fprintf(outputFile, "#eof\n");
    // fclose(outputFile);
}

// simple helper function to describe what the tool does
INT32 Usage(){
    PIN_ERROR("This Pintool prints the total size of memory that the program writes to\n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}


int main(int argc, char* argv[]){


    // Cache* cache = new Cache(9, 3, DATA_CACHE);

    // initalize pin which is what we use to be able to add/view instructions
    if (PIN_Init(argc, argv)) return Usage();

    // open a file that the output will be written to
    // outputFile = fopen("thomasTest.out", "w");
    
    /*
    add an instriment function to be called with parameters:
    INS_INSTRUMENT_CALLBACK: function = Instrumentation function for instructions
    VOID*: value = second value (ignored in this case)
    */
    INS_AddInstrumentFunction(Instruction, 0);
    // INS_AddInstrumentFunction(Instruction, (VOID*)cache);

    /*
    set the FinishFunction to be called on application exit, has parameters:
    FINI_CALLBACK: function = Call back function is passed exit code and val
    VOID*: value = second value (ignored in this case)
    */
    PIN_AddFiniFunction(FinishFunction, 0);

    // start the pin program, it has no return 
    PIN_StartProgram();

    // delete cache;

    // exit main
    return 0;
}