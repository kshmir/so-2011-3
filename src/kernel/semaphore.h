// This whole section (Semaphore) is no longer used in the kernel.

#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

// Starts a semaphore.
int sem_create(int key);

// Gets the value of a semaphore.
int sem_value(int sem);

// Sends a semaphore up.
void sem_up(int sem, int amount);

// Sends a semaphore down.
void sem_down(int sem, int amount);

// Free's a semaphore.
int sem_free(int sem, int key);

#endif