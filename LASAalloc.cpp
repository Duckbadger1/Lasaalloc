/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 		LASAalloc.h
 * 		LASAalloc class declaration.
 *
 * 		Do not change this file other than to add local varaibles and
 * Functions. Make any changes only in the provided block.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "LASAalloc.h"
#include <iostream>
#include <stdlib.h>

// Defines for LASAalloc buffer simulation.
// Keep it simple, no changes to program break
#define INITIAL_MALLOC_SIZE 100000
#define MAX_MALLOC_SIZE 100000

using namespace std;

// typedef unsigned char BYTE_t;

LASAalloc::LASAalloc() {
  brk(INITIAL_MALLOC_SIZE);

  // Point to where first node will be located.
  block *firstBlock = (block *)bufferBase;
  freeList = firstBlock;

  // Configure first node on freeList
  firstBlock->size = (int)(bufferSize);
  firstBlock->prev_block = nullptr;
  firstBlock->next_block = nullptr;
  firstBlock->this_block_data =
      (void *)((long long int)bufferBase + (long long int)sizeof(block));
  firstBlock->freeFlag = true;

  // Show initial statistics
  cout << "buffer Allocation: " << bufferBase << " - " << brk(0) << endl;
  cout << "freeList: " << freeList << " - " << brk(0) << endl;
  cout << "Block header size " << sizeof(block) << endl;
  cout << "integer size " << sizeof(int) << endl;

  display_node(freeList);
}

LASAalloc::~LASAalloc() {}

void LASAalloc::display_node(struct block *p) {
  cout << "Prev: " << p->prev_block;
  cout << "\tNext: " << p->next_block;
  cout << "\tFree: " << p->freeFlag;
  cout << "\tSize: " << p->size;
  cout << "\tThis: " << p->this_block_data << endl;
  cout << endl;
}

void LASAalloc::display() {
  struct block *p;
  if (freeList == NULL) {
    cout << "List is empty\n";
    return;
  }
  p = freeList;
  cout << "List is :\n";
  while (p != NULL) {
    display_node(p);
    p = p->next_block;
  }
  cout << "\n";
}

void LASAalloc::display(struct block *begin) {
  struct block *p;
  if (begin == NULL) {
    cout << "List is empty\n";
    return;
  }
  p = begin;
  cout << "List is :\n";
  while (p != NULL) {
    display_node(p);
    p = p->next_block;
  }
  cout << "\n";
}

void *LASAalloc::lalloc(int size) {
  block *temp = freeList;
  block *node;
  while (temp != NULL) {
    if (temp->size >= size + 32) { // just a normal malloc
      node = (block*)((long long int)(temp)+(long long int)(size + 32));
      if (temp == freeList) {   // if we are at the start
        freeList = node;
        node->next_block = temp->next_block;
        node -> prev_block = NULL;
        if (temp->next_block != NULL) {
          temp->next_block->prev_block = node;
        }
      } else {  // if we are in the middle of freelist
        node -> next_block = temp -> next_block;
        node -> prev_block = temp -> prev_block;
        temp->prev_block->next_block = node;
        if (temp->next_block != NULL) {
          temp->next_block->prev_block = node;
        }
      }
      node->size = temp->size - size - 32;
      temp -> this_block_data = (void*)((long long int) temp + 32);
      node->this_block_data =
          (void *)((long long int)temp->this_block_data + size + 32);
      node -> freeFlag = true;
      freeList = node;
      temp->freeFlag = false;
      temp->size = size;
      return temp->this_block_data;
    } else if (temp->size >= size) { // hijack
      temp->freeFlag = false;
      if (temp->prev_block == NULL) { // if the hijacked is freelist
        freeList = temp->next_block;
        temp->next_block->prev_block = NULL;
      } else if (temp->next_block == NULL) { // if the hijacked is the end
        temp->prev_block->next_block = NULL;
      } else if (temp->next_block != NULL && temp->prev_block != NULL) { // if in middle
        temp->next_block->prev_block = temp->prev_block;
        temp->prev_block->next_block = temp->next_block;
      }
      temp->prev_block = NULL;
      temp->next_block = NULL;
      return temp->this_block_data;
    } else {
      temp = temp->next_block;   // Check next
    }
  }
  return NULL;  
}

void *LASAalloc::lfree(void *userBlock) {
  block *inBlock = (block *)((long long int)(userBlock) - (long long int)(32));
  block *temp = freeList;
  while (temp != NULL) {
    if (temp > inBlock) {
      if (temp == freeList) { // case if it comes before
        inBlock->next_block = temp;
        temp->prev_block = inBlock;
        freeList = inBlock;
      } else { // case if it is in the middle
        inBlock->next_block = temp;
        inBlock->prev_block = temp->prev_block;
        temp->prev_block = inBlock;
        inBlock->prev_block->next_block = inBlock;
      }
      inBlock -> freeFlag = true;
      break;
    } else if (temp->next_block == NULL &&
               temp < inBlock) { // case if it as the end
      temp->next_block = inBlock;
      inBlock->prev_block = temp;
      inBlock -> freeFlag = true;
      break;
    } else {
      temp = temp->next_block;
    }
  } // Start of coalesense;
  if (inBlock->next_block) {
    block *nextBlockCheck =
        (block *)((long long int)(inBlock) + (long long int) (inBlock-> size + 32));
    if (nextBlockCheck == inBlock -> next_block) { // case if the next block can coalese
      inBlock->size += inBlock->next_block->size + 32;
      if (inBlock->next_block->next_block != NULL){
        inBlock->next_block = inBlock->next_block->next_block;
        inBlock -> next_block -> prev_block = inBlock;
      }
      else {
        inBlock -> next_block = NULL;
      }
    }
  }
  if (inBlock->prev_block) {
    block *prevBlockCheck =
        (block *)((long long int)(inBlock -> prev_block) + (long long int)(inBlock->prev_block->size + 32));
    if (prevBlockCheck == inBlock) { // case if the previous block can coalese
      inBlock->prev_block->size += 32 + inBlock->size;
      if (inBlock -> next_block != NULL){
        inBlock->prev_block->next_block = inBlock->next_block;
        inBlock->next_block->prev_block = inBlock->prev_block;
      }
      else {
        inBlock -> prev_block -> next_block = NULL;
      }
    }
    return inBlock->prev_block;
  }
  return inBlock;
}

void *LASAalloc::findFit(int size) {}

void *LASAalloc::split(block *target, int size) {}

/*
 *   >>>>>>  DO NOT CHANGE THIS SECTION  <<<<<<<
 *
 * brk()
 * Function to simulate the libc brk() function to allocate memory for a buffer
 *
 */

void *LASAalloc::brk(int size) {

  if (size != 0) {
    if (bufferBase == 0) {
      bufferBase = malloc(size);
      bufferSize = size;
    } else {
      cout << "buffer already locked/n";
      return 0;
    }
  }
  return (void *)(bufferSize + (long long int)(bufferBase));
}