#include <gtest/gtest.h>
#include <rwlock.h>
#include <thread>
#include <functional>
#include <mutex>
#include <future>

typedef unsigned int uint;

constexpr uint MAX_COUNT = 100;
constexpr uint WAIT_WRITER = 100;
constexpr uint WAIT_READER = 10;

class ThreadProc {
public:
    ThreadProc(): count(0){}

    uint writer_proc(void){
        sstl::mutex_wr mtxw(mtx);
        uint write = 0;
        while(count<MAX_COUNT){
            write += 1;
            increment(mtxw);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_WRITER));
        }
        return write;
    }

    uint reader_proc(void){
        sstl::mutex_rd mtxr(mtx);
        uint read = 0;
        while (count < MAX_COUNT){
            read += 1;
            lock_read(mtxr);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_READER));
        }
        return read;
    }
private:
    void increment(sstl::mutex_wr& m){
        std::lock_guard<sstl::mutex_wr> lock(m);
        count += 1;
    }
    void lock_read(sstl::mutex_rd& m){
        std::lock_guard<sstl::mutex_rd> lock(m);
    }
    sstl::base_mutex mtx;
    uint count = 0;
};

TEST(RWLockSuite,Read) {

    ThreadProc tp;
    std::future<uint> f1, f2, f3, f4, f5;

    f1 = std::async(std::launch::async, std::bind(&ThreadProc::reader_proc,&tp));
    f2 = std::async(std::launch::async, std::bind(&ThreadProc::reader_proc,&tp));
    f3 = std::async(std::launch::async, std::bind(&ThreadProc::writer_proc,&tp));
    f4 = std::async(std::launch::async, std::bind(&ThreadProc::writer_proc,&tp));
    f5 = std::async(std::launch::async, std::bind(&ThreadProc::reader_proc,&tp));

    ASSERT_TRUE(true);

    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();
    f5.wait();

    double r1, r2, r3, w1, w2;
    r1 = f1.get();
    r2 = f2.get();
    r3 = f5.get();
    w1 = f3.get();
    w2 = f4.get();

    std::cout << "r1=" << r1 << " r2=" << r2 << " r3=" << r3 << " w1=" << w1  << " w2=" << w2 << std::endl;

    const double ratio = double(WAIT_WRITER) / double(WAIT_READER);
    ASSERT_GT(r1*ratio,w1);
    ASSERT_GT(r1*ratio,w2);
    ASSERT_GT(r1*ratio,w1);
    ASSERT_GT(r2*ratio,w2);
}