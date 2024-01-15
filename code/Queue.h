#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX 6

int intArray[MAX];
int itemCount = 0;

int peek(){
   return intArray[itemCount - 1];
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
			
        // insert the data 
         intArray[itemCount] = data;
         itemCount++;
        }  
			

      }
}


int removeData(){
   return intArray[--itemCount]; 
}