#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/list.h"

#include "semaphore.h"


typedef struct semaphore {
	int key;
	int value;
} semaphore;

static list semaphores = NULL;

static semaphore * sem_find(int key) { 
	foreach(semaphore *, sem, semaphores) {
		if(sem->key == key) {
			return sem;
		}
	}
	return NULL;
}

// Starts a semaphore.
int sem_create(int key){
	if(semaphores == NULL)	{
		semaphores = list_init();
	}
	
	semaphore * sem = NULL;
	if((sem = sem_find(key)) == NULL)
	{
		sem = (semaphore *) malloc(sizeof(semaphore));
		sem->value = 0;
		sem->key = key;
		list_add(semaphores, sem);
	}
	return (int)sem;
}

// Gets the value of a semaphore.
int sem_value(int sem){
	 return ((semaphore *)sem)->value;
}

// Sends a semaphore up.
void sem_up(int sem, int amount){
	semaphore * _s = (semaphore *) sem;
	int i = amount;
	while(i > 0) {
		_s->value++;
		i--;
	}
}

// Sends a semaphore down.
void sem_down(int sem, int amount){
	semaphore * _s = (semaphore *) sem;
	int i = amount;
	while(i > 0) {
		while(_s->value == 0) {
			yield();
		}
		_s->value--;
		i--;
	}
}

// Free's a semaphore.
int sem_free(int sem, int key){
	// nothing good to do haha
	return 1;
}