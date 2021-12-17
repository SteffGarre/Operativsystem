#define _XOPEN_SOURCE
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"
#include <signal.h>
#include <sys/time.h>

#define FALSE 0
#define TRUE 1

#define PERIOD 100

#define STACK_SIZE 4096

static sigset_t block;

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;

static green_t *rdy_queue = NULL;

static void init() __attribute__((constructor));

void timer_handler(int);

/*We initialize the block
to hold the mask of the SIGVTALRM, set the handler to our timer_handler()
function and associate the signal to the action handler. We then set the timer
interval and delay (value) to our PERIOD and start the timer.
*/
void init()
{
    getcontext(&main_cntx);

    sigemptyset(&block);
    sigaddset(&block, SIGVTALRM);

    struct sigaction act = {0};
    struct timeval interval;
    struct itimerval period;

    act.sa_handler = timer_handler;
    assert(sigaction(SIGVTALRM, &act, NULL) == 0);
    interval.tv_sec = 0;
    interval.tv_usec = PERIOD;
    period.it_interval = interval;
    period.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &period, NULL);
}

void enqueue(green_t **queue, green_t *new_thread)
{

    green_t *current = *queue;
    if (current == NULL)
    {
        *queue = new_thread;
    }
    else
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_thread;
    }
}

green_t *dequeue(green_t **queue)
{

    green_t *current = *queue;
    if (current == NULL)
    {
        return NULL;
    }
    else
    {
        *queue = (*queue)->next;
        current->next = NULL;
        return current;
    }
}

/*This function will do two things: start the execution of the real function and, when after returning from the call, terminate the thread. */
void green_thread()
{

    green_t *this = running;

    void *result = (*this->fun)(this->arg);

    if (this->join != NULL)
    {
        // place waiing thread in rdy queue
        enqueue(&rdy_queue, this->join);
    }

    // save result of execution
    this->retval = result;
    // Turn to zombie
    this->zombie = TRUE;
    // Find the next thread to run
    green_t *next = dequeue(&rdy_queue);

    running = next;
    setcontext(next->context);
}

/* A new green thread is created in a two stage process. First the user will call
green_create() and provide: an uninitialized green_t structure, the function
the thread should execute and pointer to its arguments. We will create new
context, attach it to the thread structure and add this thread to the ready
queue.
*/
int green_create(green_t *new, void *(*fun)(void *), void *arg)
{

    ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(cntx);

    void *stack = malloc(STACK_SIZE);

    cntx->uc_stack.ss_sp = stack; // explicity change the stack to our own
    cntx->uc_stack.ss_size = STACK_SIZE;
    makecontext(cntx, green_thread, 0);

    new->context = cntx;
    new->fun = fun;
    new->arg = arg;
    new->next = NULL;
    new->join = NULL;
    new->retval = NULL;
    new->zombie = FALSE;

    /* maging what could happen if we are in the middle of a yield
    operation and change the run queue. We need to prevent these interrupts
    when we change the state. */
    // add new to rdy queue

    sigprocmask(SIG_BLOCK, &block, NULL);
    enqueue(&rdy_queue, new);
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    return 0;
}

/*  This function will simply put the
running thread last in the ready queue and then select the first thread from
the queue as the next thread to run.
*/
int green_yield()
{

    green_t *susp = running;
    enqueue(&rdy_queue, susp);

    // Find the next thread to run
    green_t *next = dequeue(&rdy_queue);

    running = next;
    swapcontext(susp->context, next->context);
    return 0;
}

/*The join operation will wait for a thread to terminate. We therefore add the
thread to the join field and select another thread for execution*/
int green_join(green_t *thread, void **res)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    if (!thread->zombie)
    {
        green_t *susp = running;

        // add as joining thread
        thread->join = susp;

        // Find the next thread to run
        green_t *next = dequeue(&rdy_queue);
        running = next;
        swapcontext(susp->context, next->context);
    }

    if (thread->retval != NULL && res != NULL)
    {
        *res = thread->retval;
    }

    free(thread->context);
    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

// initialize a green condition variable
void green_cond_init(green_cond_t *cond)
{
    sigprocmask(SIG_BLOCK, &block, NULL);
    cond->queue = NULL;
    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

// suspend the current thread on the condition
void green_cond_wait(green_cond_t *cond, green_mutex_t *mutex)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    green_t *current = running;
    assert(current != NULL);

    enqueue(&cond->queue, current);

    if (mutex != NULL)
    {
        mutex->taken = FALSE;

        // move suspened thread to rdy queue
        green_t *susp = dequeue(&mutex->susp);
        enqueue(&rdy_queue, susp);
        mutex->susp = NULL;
    }

    green_t *next = dequeue(&rdy_queue);
    assert(next != NULL);
    running = next;
    swapcontext(current->context, next->context);

    if (mutex != NULL)
    {

        // try take the lock
        if (mutex->taken)
        {
            //bad luck suspend again
            green_t *susp = running;
            enqueue(&mutex->susp, susp);

            green_t *next = dequeue(&rdy_queue);
            running = next;
            swapcontext(susp->context, next->context);
        }
        else
        {
            // take the lock
            mutex->taken = TRUE;
        }
    }

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return;
}

// move the first suspended thread to the rdy queue
void green_cond_signal(green_cond_t *cond)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (cond->queue == NULL)
        return;
    else
    {
        green_t *thread = dequeue(&cond->queue);
        enqueue(&rdy_queue, thread);
        sigprocmask(SIG_UNBLOCK, &block, NULL);
    }
}

void timer_handler(int sig)
{

    green_t *susp = running;

    // add the running to the rdy queue
    enqueue(&rdy_queue, susp);

    // Find the next thread to run
    green_t *next = dequeue(&rdy_queue);
    running = next;
    swapcontext(susp->context, next->context);
}

/*Since a thread now can be interrupted at any point in the execution
we will have a problem when we update shared data structures. We need a way to synchronize our threads
and a mutex construct would do the trick. */

int green_mutex_init(green_mutex_t *mutex)
{
    sigprocmask(SIG_BLOCK, &block, NULL);

    mutex->taken = FALSE;
    mutex->susp = NULL;

    sigprocmask(SIG_UNBLOCK, &block, NULL);
}

int green_mutex_lock(green_mutex_t *mutex)
{

    //block timer interrupts
    sigprocmask(SIG_BLOCK, &block, NULL);

    green_t *susp = running;
    if (mutex->taken)
    {
        // suspend the runnign thread
        enqueue(&mutex->susp, susp);
        green_t *next = dequeue(&rdy_queue);

        assert(next != NULL);
        running = next;
        swapcontext(susp->context, next->context);
    }
    else
    {
        // take the lock
        mutex->taken = TRUE;
    }

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}

int green_mutex_unlock(green_mutex_t *mutex)
{

    //block timer interrupts
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (mutex->susp != NULL)
    {
        green_t *next = dequeue(&mutex->susp);
        enqueue(&rdy_queue, next);
    }
    else
    {
        // release lock
        mutex->taken = FALSE;
        mutex->susp = NULL;
    }

    sigprocmask(SIG_UNBLOCK, &block, NULL);
    return 0;
}
