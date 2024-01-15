#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX 6


struct PriorityQueue
{
   int* intArray;
   unsigned capacity;
   int itemCount;
};

struct PriorityQueue* createPriorityQueue(unsigned capacity)
{
     struct PriorityQueue* P_Queue = (struct PriorityQueue*)malloc(sizeof(struct PriorityQueue));
     P_Queue->capacity = capacity;
     P_Queue->itemCount = 0;
     P_Queue->intArray = (int*)malloc(P_Queue->capacity * sizeof(int));
     return P_Queue;
}

int peek(struct PriorityQueue* intArray){
   return &intArray[itemCount - 1];
}

bool isEmpty(){
   return itemCount == 0;
}

bool isFull(){
   return itemCount == MAX;
}

int size(){
   return itemCount;
}  

void insert(int data){
   int i = 0;

   if(!isFull()){
      // if queue is empty, insert the data 
      if(itemCount == 0){
         intArray[itemCount++] = data;        
      }else{
         // start from the right end of the queue 
			
         for(i = itemCount - 1; i >= 0; i-- ){
            // if data is larger, shift existing item to right end 
            if(data < intArray[i]){
               intArray[i+1] = intArray[i];
            }else{
               break;
            }            
         }  
			
         // insert the data 
         intArray[i+1] = data;
         itemCount++;
      }
   }
}

int removeData(){
   return intArray[--itemCount]; 
}

