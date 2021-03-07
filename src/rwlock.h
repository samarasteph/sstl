#include <memory>

namespace sstl {
    class base_mutex;

    class mutex_rd {
    public:
        mutex_rd() = delete;
        mutex_rd(base_mutex&);
        void lock();
        void unlock();
    private:
        base_mutex& m_mut;
    };

    class mutex_wr {
    public:
        mutex_wr() = delete;
        mutex_wr(base_mutex&);
        void lock();
        void unlock();
    private:
        base_mutex& m_mut;
    };

    class base_mutex {
    public:
        base_mutex();
        ~base_mutex();
        base_mutex(const base_mutex&) = delete;
        base_mutex& operator = (const base_mutex&) = delete;
    private:
        friend mutex_rd;
        friend mutex_wr;

        void lock_rd();
        void lock_wr();
        void unlock();
        void* m_impl;
    };

}
