#include "semaphore.h"

typedef int semaphore;

// Starts a semaphore.
int sem_create(int key){
	// semaphore * sem = (semaphore *) malloc(sizeof(semaphore));
	// *sem = 0;
	// return sem;
}

// Gets the value of a semaphore.
int sem_value(int sem){
	// return *sem;
}

// Sends a semaphore up.
void sem_up(int sem, int amount){

}

// Sends a semaphore down.
void sem_down(int sem, int amount){
	// int i = amount;
	// while(i > 0) {
	// 	while(*sem == 0)
	// 	{
	// 		/* code */
	// 	}
	// 	i--;
	// }
}

// Free's a semaphore.
int sem_free(int sem, int key){
	// nothing good to do haha
}