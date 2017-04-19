#include <windows.h>

class lw_mutex {
    CRITICAL_SECTION csHandle;
  public:
    lw_mutex() {
        InitializeCriticalSection(&csHandle);
    }

    ~lw_mutex() {
        DeleteCriticalSection(&csHandle);
    }

    void lock() {
        EnterCriticalSection(&csHandle);
    }

    void unlock() {
        LeaveCriticalSection(&csHandle);
    }
};

class lw_lock_guard {
    lw_mutex& lgMutex;
  public:
    lw_lock_guard(lw_mutex& m): lgMutex(m) {
        lgMutex.lock();
    }

    ~lw_lock_guard() {
        lgMutex.unlock();
    }
};
