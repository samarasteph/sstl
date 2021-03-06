#pragma once

#ifndef __TIMER__WRAPPER__
#define __TIMER__WRAPPER__

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <stdint.h>
#include <map>
#include <mutex>
#include <thread>
#include <list>
#include <condition_variable>
#include <future>

class TimerWrapper {
public:
	TimerWrapper(boost::asio::io_service& io_obj): m_io(io_obj), m_timer(nullptr) {}
	TimerWrapper(): m_io(m_private_io) , m_timer(nullptr) {}
	void expires(std::function<void()> func, std::chrono::milliseconds delay, uint64_t id);
	void expires(std::function<void()> func, std::chrono::microseconds delay, uint64_t id);
	void expires(std::function<void()> func, std::chrono::nanoseconds delay, uint64_t id);
	void abort();
	static TimerWrapper* getInstance(uint64_t id);
	static void removeInstance(uint64_t id);
private:
	TimerWrapper(const TimerWrapper& tm): m_io(tm.m_io), m_timer(tm.m_timer) {}
	TimerWrapper(TimerWrapper&& tm): m_io(tm.m_io), m_timer(tm.m_timer) { tm.m_timer = nullptr; }
	TimerWrapper& operator = (const TimerWrapper& tm) = delete;
	TimerWrapper&& operator = (TimerWrapper&& tm) = delete;

	void _trigger_timer(std::function<void()>& f, uint64_t id);

	boost::asio::io_service& m_io;
	boost::asio::steady_timer* m_timer;

	class TimerPrivateService {
	public:
		TimerPrivateService();
	private:
		boost::asio::io_service::work m_work;
	};

	friend TimerPrivateService;

	static std::map<uint64_t,TimerWrapper*> instances;
	static std::mutex m_mutex;
	static boost::asio::io_service m_private_io;
	static TimerPrivateService	tm_io_service;
};

class MutualTimer {
public:
	class INotified {
	public:
		virtual ~INotified(){}
		virtual bool notify() = 0;
	};
	typedef struct __tm_struct__ {
		__tm_struct__(): unit(TIME_UNIT::SECONDS), elapse(0){}
		__tm_struct__(const __tm_struct__& t): unit(t.unit), elapse(t.elapse){}
		typedef enum : uint8_t { SECONDS = 3, MILLISECONDS = 2, MICROSECONDS = 1, NANOSECONDS = 0 } TIME_UNIT;
		TIME_UNIT unit;
		uint64_t elapse;
	} tm_period;

	MutualTimer();
	~MutualTimer();
	bool start();
	bool stop();
	bool registerNotified(INotified* pnotified, const tm_period& period);
	void unregisterNotified(INotified* pnotified);
	bool is_running() const;

private:

	std::mutex	m_mtx_new, m_mtx_unreg;
	void _private_running_loop();

	std::thread* m_ploop;
	volatile bool m_running;

	std::list<uintptr_t>	m_newcomers;
	std::list<uintptr_t>	m_unregistered;
	std::condition_variable m_condv;
	std::promise<void> m_exit_signal;
};

#endif



