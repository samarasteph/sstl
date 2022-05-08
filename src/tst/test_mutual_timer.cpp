#include <gtest/gtest.h>
//#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <array>
#include <iostream>
#include <string>
#include <iomanip>
#include "../timer_wrapper.h"

typedef MutualTimer::tm_period period_t;

template<class TimeUnit>
void thread_sleep(const TimeUnit& elapse) {
	std::this_thread::sleep_for(elapse);
}

template<size_t Size>
constexpr std::array<int,Size> range(int min){

	std::array<int,Size> a;

	for(int i=0; i<Size ; ++i)
		a[i] = i+min;

	return std::move(a);
}

class NotifiedMock: public MutualTimer::INotified {
public:
	NotifiedMock(): m_called(0){}
	NotifiedMock(const std::string& name): m_called(0), _name(name){}
	bool notify () override {
		//std::cout << _name << " notified " << m_called << std::endl;
		m_called += 1;
		return true;
	}
	uint32_t called() const { return m_called; }
	void set_name(const std::string& name){ _name = name; }
	uint32_t m_called;
	std::string _name;
};

class NotifiedMockClock: public NotifiedMock{
public:
	NotifiedMockClock(): NotifiedMock(), _nslate(0) {
		get_time(&_tsp);
	}
	NotifiedMockClock(const std::string& name): NotifiedMock(name), _nslate(0){
		get_time(&_tsp);
	}
	//MOCK_METHOD1(bool,notify);
	bool notify () override { 
		m_called += 1;
		struct timespec tsp;
		get_time(&tsp);
		time_t sec = tsp.tv_sec - _tsp.tv_sec;
		long nsec = tsp.tv_nsec - _tsp.tv_nsec;
		if  (nsec<0){
			sec -= 1;
			nsec = 1000000000 - nsec; 
		}
		if (_log){
			std::cout << _name << " called " << m_called << " elapsed=" << sec << "." << std::setw(9) << std::setfill('0') << nsec  << std::endl;
		}
		_tsp = tsp; 
		return true;
	}
	void get_time(struct timespec* ptsp){
		clock_gettime(CLOCK_MONOTONIC, ptsp);
	}
	static void log(bool b){ _log = b; }
	struct timespec _tsp;
	long _nslate;
	static bool _log;
};

bool NotifiedMockClock::_log = false;

TEST(MutualTimer, timer_2_seconds){
	MutualTimer mt;
	NotifiedMock mock;
	period_t period;
	period.unit = period_t::SECONDS;
	period.elapse = 2;

	mt.start();

	mt.registerNotified(static_cast<MutualTimer::INotified*>(&mock), period);
	EXPECT_EQ(mock.called(), 0);
	thread_sleep(std::chrono::milliseconds(2100));
	EXPECT_EQ(mock.called(), 1);
	//EXPECT_CALL(mock,notify()).Times(0);
	mt.stop();
	
	ASSERT_TRUE(true);
}

TEST(MutualTimer, start_2_timers){
	MutualTimer mt;
	NotifiedMock mock1,mock2;
	period_t period;

	mt.start();

	period.unit = period_t::SECONDS;
	period.elapse = 2;
	mt.registerNotified(static_cast<MutualTimer::INotified*>(&mock1), period);

	period.unit = period_t::MILLISECONDS;
	period.elapse = 500;
	mt.registerNotified(static_cast<MutualTimer::INotified*>(&mock2), period);


	EXPECT_EQ(mock1.called(), 0);
	EXPECT_EQ(mock2.called(), 0);
	thread_sleep(std::chrono::milliseconds(520));
	
	EXPECT_EQ(mock1.called(), 0);
	EXPECT_EQ(mock2.called(), 1);
	thread_sleep(std::chrono::milliseconds(1500));
	
	EXPECT_EQ(mock1.called(), 1);
	EXPECT_EQ(mock2.called(), 4);
	//EXPECT_CALL(mock,notify()).Times(0);
	mt.stop();
}

TEST(MutualTimer, unregister){
	MutualTimer mt;
	NotifiedMock mock;
	period_t period;

	period.unit = period_t::MILLISECONDS;
	period.elapse = 500;

	mt.start();

	mt.registerNotified(static_cast<MutualTimer::INotified*>(&mock), period);
	thread_sleep(std::chrono::milliseconds(1510));
	mt.unregisterNotified(static_cast<MutualTimer::INotified*>(&mock));

	thread_sleep(std::chrono::milliseconds(510));

	ASSERT_EQ(mock.called(),3);
}

TEST(MutualTimer, multiple_seconds){
	MutualTimer mt;
	constexpr uint NB = 10;
	constexpr uint MOD = 3;

	NotifiedMockClock mock[NB];
	period_t period;

	period.unit = period_t::SECONDS;

	auto elapsed = [](int i) { return (i % MOD) + 1; };

	for (int i: range<NB>(0)){
		period.elapse = elapsed(i);
		mock[i].set_name(std::to_string(i+1));
		mt.registerNotified(static_cast<MutualTimer::INotified*>(&mock[i]), period);
	}

	mt.start();

	thread_sleep(std::chrono::seconds(MOD+2));

	for(int i : range<NB>(0)){
		mt.unregisterNotified(static_cast<MutualTimer::INotified*>(&mock[i]));
	}
	
	mt.stop();

	for(int i : range<NB>(0)){
		double expected = double(MOD) / double(elapsed(i));
		expected -= expected * 1E-1 * MOD;
		ASSERT_GE(double(mock[i].called()), expected) << mock[i]._name << " called " << mock[i].called() << " times !=" << expected;
		//std::cout << mock[i]._name << " called " << mock[i].called() << " times" << std::endl;
	}
	ASSERT_TRUE(true);
}

TEST(MutualTimer, multiple_milliseconds){
	MutualTimer mt;
	constexpr uint NB = 100;
	constexpr uint MOD = 30;

	NotifiedMockClock mock[NB];
	period_t period;

	period.unit = period_t::MILLISECONDS;

	auto elapsed = [](int i) { return (i % MOD) + 1; };

	for (int i: range<NB>(0)){
		period.elapse = elapsed(i);
		mock[i].set_name(std::to_string(i+1));
		mt.registerNotified(static_cast<MutualTimer::INotified*>(&mock[i]), period);
	}

	mt.start();

	thread_sleep(std::chrono::seconds(MOD+1));

	for(int i : range<NB>(0)){
		mt.unregisterNotified(static_cast<MutualTimer::INotified*>(&mock[i]));
	}

	mt.stop();
	for(int i : range<NB>(0)){
		double expected = double(MOD) / double(elapsed(i));
		expected -= expected * 1E-1 * MOD;
		ASSERT_GE(double(mock[i].called()), expected) << mock[i]._name << " called " << mock[i].called() << " times !=" << expected;
		//std::cout << mock[i]._name << " called " << mock[i].called() << " times" << std::endl;
	}
	ASSERT_TRUE(true);

}