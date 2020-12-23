#ifndef MUTEX_H
#define MUTEX_H

//TODO add google-glog CHECK() Macro.

#include "noncopyable.h"
#include <assert.h>
#include <pthread.h>

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

class mutex_lock : noncopyable
{
public:
    mutex_lock()
        // : holder_(0)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

        pthread_mutex_init(&mutex_, &attr);    
    }

    ~mutex_lock() 
    {
        // assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    void lock()             // Only used for mutex_lock_guard. Forbid user code use it.
    {
        pthread_mutex_lock(&mutex_);
        // holder_ = current_thread::tid();
    }
    void unlock()           // Only used for mutex_lock_guard. Forbid user code use it.
    {
        // holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }

    /* bool islocked_by_this_thread()
    {
        return holder_ == current_thread::tid():
    } */

    /* void assert_locked()
    {
        assert(islocked_by_this_thread());
    } */

    /* pthread_mutex_t* get_pthread_mutex()     //Only used for condition. Forbid user code use it.
    {
        return mutex_;
    } */


private:
    pthread_mutex_t mutex_;
    // pid_t holder_;
};

/* 
    the usage is:

    mutex_lock mutex;
    void doit()
    {
        mutex_lock_guard lock(mutex);
        
        // This is Critical section
    }
 */
class mutex_lock_guard : noncopyable 
{
public:
    explicit mutex_lock_guard(mutex_lock &mutex) : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~mutex_lock_guard()
    {
        mutex_.unlock();
    }

private:
    mutex_lock &mutex_;
};

// Prevent misuse like:
// mutex_lock_guard(mutex_);
// A tempory object doesn't hold the lock for long!
#define mutex_lock_guard(x) static_assert(false, "missing mutex guard var name")

#endif