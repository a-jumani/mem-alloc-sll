#ifndef __MEM_H_
#define __MEM_H_

void * Mem_Init(int sizeOfRegion);

void * Mem_Alloc(int size);

int Mem_Free(void * ptr, int coalesce);

void Mem_Dump();

#endif


