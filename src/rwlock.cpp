#include <pthread.h>
#include "rwlock.h"

namespace sstl {
	namespace detail {
		class mutex_internal {
			friend sstl::base_mutex;
			public:
			~mutex_internal(){
				pthread_rwlock_destroy(m_rwlock.get());
			}
			private:
			void lock_wr(){
				pthread_rwlock_wrlock(m_rwlock.get());
			}
			void lock_rd(){
				pthread_rwlock_rdlock(m_rwlock.get());
			}
			void unlock() {
				pthread_rwlock_unlock(m_rwlock.get());
			}
			mutex_internal(){
				m_rwlock.reset(new pthread_rwlock_t);
				pthread_rwlock_init(m_rwlock.get(), &m_attr);
			}
			std::unique_ptr<pthread_rwlock_t> m_rwlock;
			pthread_rwlockattr_t m_attr;
		};
	}
}

sstl::mutex_rd::mutex_rd(sstl::base_mutex& bm): m_mut(bm){}

void sstl::mutex_rd::lock(){
	m_mut.lock_rd();
}

void sstl::mutex_rd::unlock(){
	m_mut.unlock();
}

sstl::mutex_wr::mutex_wr(sstl::base_mutex& bm): m_mut(bm){}

void sstl::mutex_wr::lock(){
	m_mut.lock_wr();
}

void sstl::mutex_wr::unlock(){
	m_mut.unlock();
}

sstl::base_mutex::base_mutex() : m_impl(new sstl::detail::mutex_internal()) {}
sstl::base_mutex::~base_mutex() { delete reinterpret_cast<sstl::detail::mutex_internal*>(m_impl);  } 
    
void sstl::base_mutex::lock_rd(){ reinterpret_cast<sstl::detail::mutex_internal*>(m_impl)->lock_rd(); }
void sstl::base_mutex::lock_wr(){ reinterpret_cast<sstl::detail::mutex_internal*>(m_impl)->lock_wr(); }
void sstl::base_mutex::unlock(){ reinterpret_cast<sstl::detail::mutex_internal*>(m_impl)->unlock(); }