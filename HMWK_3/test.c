#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define NUM_THREADS 3

int shared_iter;
// pthread_cond_t cond1;
// pthread_cond_t cond2;
// pthread_cond_t cond3;
pthread_mutex_t lock;
// bool done = false;

/* create thread argument struct for thr_func() */
typedef struct _thread_data_t
{
	int thread_id;
	int iter;
} thread_data_t;

/* thread function */
void *thread_function(void *arg)
{
	thread_data_t *data = (thread_data_t *) arg;
	pthread_mutex_lock(&lock);
	for (shared_iter = 1; shared_iter <= 5; shared_iter++)
	{
		for (data->iter = 1; data->iter <= 3; data->iter++)
		{
			data->thread_id = data->iter;
			printf("Thread %d - iteation no. %d\n\n", data->thread_id, shared_iter);
		}
	}
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	pthread_t thread[NUM_THREADS];
	int i, rc;

	/* create a thread_data_t argument array */
	thread_data_t thread_data[NUM_THREADS];

	shared_iter = 0;

	pthread_mutex_init(&lock, NULL);

	/* create threads */
	for (i = 0; i < NUM_THREADS; i++)
	{
		thread_data[i].thread_id = i + 1;
		if ((rc = pthread_create(&thread[i], NULL, thread_function, &thread_data[i])))
		{
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			return EXIT_FAILURE;
		}
		break;
	}

	/* block until all threads complete */
	for (i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(thread[i], NULL);
		break;
	}
	
	return EXIT_SUCCESS;
}