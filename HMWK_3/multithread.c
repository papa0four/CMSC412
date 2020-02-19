#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

/*
 * @function usage
 * @brief Prints out the usage statement
 * @param [in] stream - I/O stream to print to
 * @return void
 */
static void
usage(FILE *stream);

/*
 * @function toLong
 * @brief Converts a string to a long integer
 * @param [in] str - a character array to convert
 * @return long
 */
static long
toLong(char *str);

/*
 * @function threadPrint
 * @brief Allows a thread to print something
 * @param [in] arg - identifies the thread number
 * @return void
 */
static void *
threadPrint(void *arg);

/*
 * @function signalDone
 * @brief Sends the signal that a task has completed
 * @return void
 */
static void
signalDone();

// These arrays are used to signal each of the threads
pthread_cond_t *condArray = NULL;
pthread_mutex_t *mutexArray = NULL;

// Protects updating of the iteration count
pthread_mutex_t mutexIter = PTHREAD_MUTEX_INITIALIZER;
long iterCnt;

// Protects keepLooping flag
pthread_mutex_t mutexLoop = PTHREAD_MUTEX_INITIALIZER;
bool keepLooping = true;

// Signals completion
pthread_cond_t condDone;
pthread_mutex_t mutexDone = PTHREAD_MUTEX_INITIALIZER;

int
main(int argc, char **argv) {

    // Lightly validate CLI args
    if (3 != argc) {
        usage(stderr);
        exit(EXIT_FAILURE);
    }
   
    long numThreads = toLong(argv[1]);
    if (-1 == numThreads) {
        usage(stderr);
        exit(EXIT_FAILURE);
    }
    
    long numIterations = toLong(argv[2]);
    if (-1 == numIterations) {
        usage(stderr);
        exit(EXIT_FAILURE);
    }
    
    int status;
    int *threadIds = NULL;
    
    // Allocate for the pthread array
    pthread_t *threadArray = NULL;
    threadArray = calloc(numThreads, sizeof(*threadArray));
    if (NULL == threadArray) {
        perror("Unable to allocate threadArray");
        exit(EXIT_FAILURE);
    }
    
    // Allocate for the thread condition variables
    condArray = malloc(numThreads * sizeof(*condArray));
    if (NULL == condArray) {
        perror("Unable to allocate condArray");
        goto ExitMain;
    }
    
    // Intialize thread condition variables
    pthread_cond_init(&condDone, NULL);
    for (int i = 0; i < numThreads; i++) {
        pthread_cond_init(&condArray[i], NULL);
    }

    // Allocate for the thread mutexes
    mutexArray = malloc(numThreads * sizeof(*mutexArray));
    if (NULL == mutexArray) {
        perror("Unable to allocate mutexArray");
        goto ExitMain;
    }

    // Initialize thread mutexes
    for (int i = 0; i < numThreads; i++) {
        pthread_mutex_init(&mutexArray[i], NULL);
    }

    // Allocate for threadIds Array
    threadIds = malloc (numThreads * sizeof(*threadIds));
    for (int i = 0; i < numThreads; i++) {
        threadIds[i] = i;
        status = pthread_create(&threadArray[i], NULL, &threadPrint, &threadIds[i]);
    }

    // ensure each thread has enough time to lock mutex and begin waiting
    sleep(5);

    for (iterCnt = 1; iterCnt <= numIterations; )
    {
        // Signal each thread one by one and listen for completion
        for (int i = 0; i < numThreads; i++) {
            // Wake up thread
            pthread_mutex_lock(&mutexArray[i]);
            pthread_cond_signal(&condArray[i]);
            pthread_mutex_unlock(&mutexArray[i]);

            // Wait for done message
            pthread_mutex_lock(&mutexDone);
            pthread_cond_wait(&condDone, &mutexDone);
            pthread_mutex_unlock(&mutexDone);
        }

        // update increment counter
        status = pthread_mutex_lock(&mutexIter);
        if (0 != status) {
            break;
        }
        iterCnt++;
        pthread_mutex_unlock(&mutexIter);
    } 

    // Stop looping
    pthread_mutex_lock(&mutexLoop);
    keepLooping = false;
    pthread_mutex_unlock(&mutexLoop);

    // Signal each thread one more time so they can exit
    for (int i = 0; i < numThreads; i++) {
        // Wake up thread
        pthread_mutex_lock(&mutexArray[i]);
        pthread_cond_signal(&condArray[i]);
        pthread_mutex_unlock(&mutexArray[i]);
        
        // Wait for Done message
        pthread_mutex_lock(&mutexDone);
        pthread_cond_wait(&condDone, &mutexDone);
        pthread_mutex_unlock(&mutexDone);
    }

    // Join each thread to exit
    for (int i = 0; i < numThreads; i++) {
       status = pthread_join(threadArray[i], NULL); 
       if (0 != status) {
            perror("Unable to join thread.");
            goto ExitMain;
       }
    }

ExitMain:
    
    // Cleanup
    if (NULL != threadIds) {
        free(threadIds);
    }

    if (NULL != condArray) {
        free(condArray);
    }

    if (NULL != mutexArray) {
        free(mutexArray);
    }

    if (NULL != threadArray) {
        free(threadArray);
    }

    return 0;
}

static void *
threadPrint(void *arg) {
    if (NULL == arg) {
        errno = EINVAL;
        return (void *)1;
    }
    
    // Get thread's ID
    int threadId = *(int *)arg;
    int status;
    
    while (1) {
        
        // Lock this thread's mutex
        status = pthread_mutex_lock(&mutexArray[threadId]);
        if (status != 0) {
            break;
        }

        // Lock keepLooping mutex before looking
        pthread_mutex_lock(&mutexLoop);
        if (true == keepLooping) {
            // Unlock keepLooping mutex and listen for thread's signal
            pthread_mutex_unlock(&mutexLoop);
            pthread_cond_wait(&condArray[threadId], &mutexArray[threadId]);
        }
        else {
            // Exiting so unlock keepLooping mutex and thread's mutex
            pthread_mutex_unlock(&mutexLoop);
            pthread_mutex_unlock(&mutexArray[threadId]);
            break;
        }

        // Lock keepLooping mutex before looking
        pthread_mutex_lock(&mutexLoop);
        if (true == keepLooping) {
            // Lock iterCnt mutex before printing
            status = pthread_mutex_lock(&mutexIter);
            if (0 == status) {
                // Print and unlock iterCnt mutex
                printf("Thread %d  - iteration no. %ld\n", threadId + 1, iterCnt);
                pthread_mutex_unlock(&mutexIter);
            }
        }

        // Unlock keepLooping mutex and thread's mutex
        pthread_mutex_unlock(&mutexLoop);
        pthread_mutex_unlock(&mutexArray[threadId]);
        
        // Signal completion
        signalDone(); 
    }
    
    // Signal completion
    signalDone(); 

    return (void *)0;
}

static void
signalDone() {

    // Lock done mutex, signal completion, and unlock done mutex
    pthread_mutex_lock(&mutexDone);
    pthread_cond_signal(&condDone);
    pthread_mutex_unlock(&mutexDone);
}

static void
usage(FILE *stream) {
    fprintf(stream, "Usage: ./multithread NUM_THREADS NUM_ITERATIONS\n");
}

static long
toLong(char *str) {
    errno = 0;
    char *endptr;

    long num = strtol(str, &endptr, 10);
    if ((ERANGE == errno && (num == LONG_MAX || num == LONG_MIN))
            || (errno != 0 && num == 0)) {
        return -1;
    }
    if (endptr == str || 0 != *endptr) {
        return -1;
    }

    if (0 >= num) {
        return -1;
    }

    return num;
}
