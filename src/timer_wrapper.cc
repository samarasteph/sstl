#include "timer_wrapper.h"
#include <time.h>
#include <memory>
#include <set>
//#define __TRACE__ 1
#ifdef __TRACE__
#include <iostream>
#define TRACE(trace) std::cout << trace << std::endl
#else
#define TRACE(trace)
#endif

std::mutex TimerWrapper::m_mutex;
std::map<uint64_t,TimerWrapper*> TimerWrapper::instances;
boost::asio::io_service TimerWrapper::m_private_io;
TimerWrapper::TimerPrivateService TimerWrapper::tm_io_service;


TimerWrapper::TimerPrivateService::TimerPrivateService(): m_work(TimerWrapper::m_private_io){}

uint64_t Common_getSysclock() {
	struct timespec tsp;
	clock_gettime(CLOCK_MONOTONIC, &tsp);
	return tsp.tv_sec * 1000000000 + tsp.tv_nsec;
}

void TimerWrapper::expires(std::function<void()> func, std::chrono::milliseconds delay, uint64_t id){
	if(m_timer==nullptr){
		m_timer = new boost::asio::steady_timer(m_io, std::chrono::milliseconds(0));
		m_timer->expires_from_now(delay);
		_trigger_timer(func, id);
	}
}
void TimerWrapper::expires(std::function<void()> func, std::chrono::microseconds delay, uint64_t id){
	if(m_timer==nullptr){
		m_timer = new boost::asio::steady_timer(m_io, std::chrono::microseconds(0));
		m_timer->expires_from_now(delay);
		_trigger_timer(func, id);
	}
}

void TimerWrapper::expires(std::function<void()> func, std::chrono::nanoseconds delay, uint64_t id){
	if(m_timer==nullptr){
		m_timer = new boost::asio::steady_timer(m_io, std::chrono::nanoseconds(0));
		m_timer->expires_from_now(delay);
		_trigger_timer(func, id);
	}
}

void TimerWrapper::_trigger_timer(std::function<void()>& func, uint64_t id){
	m_timer->async_wait( [this, func, id] (const boost::system::error_code& error){
		if (error != boost::asio::error::operation_aborted) {
			func();
		}
		removeInstance(id);
	});
	TimerWrapper* ptm = new TimerWrapper(*this);
	std::unique_lock<std::mutex> lock(m_mutex);
	instances.insert(std::make_pair(id,ptm));
}

void TimerWrapper::abort() {
	if (m_timer) {
		try{
			m_timer->cancel();
		}catch(boost::system::system_error& e) {
			perror("TimerWrapper catch exception on abort");
		}
	}
}

//static
TimerWrapper* TimerWrapper::getInstance(uint64_t id) {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (instances.find(id) != std::end(instances)){
		return instances[id];
	}
	return nullptr;
}

void TimerWrapper::removeInstance(uint64_t id) {
	std::unique_lock<std::mutex> lock(m_mutex);
	auto it = instances.find(id);
	if (it != std::end(instances)){
		delete it->second;
		instances.erase(it);
	}
}

MutualTimer::MutualTimer(): m_ploop(nullptr){}

MutualTimer::~MutualTimer(){
	stop();
}

bool MutualTimer::start(){
	if(m_ploop == nullptr){
		m_running = true;
		m_exit_signal = std::promise<void>();
		m_ploop = new std::thread( [this] () {
			this->_private_running_loop();
		} );
		m_condv.notify_one();
		return true;
	}
	return false;
}

bool MutualTimer::stop() {
	if(m_ploop){
		m_running = false;
		m_exit_signal.set_value();
		m_condv.notify_one();
		m_ploop->join();

		delete m_ploop;
		m_ploop = nullptr;
		return true;
	}
	return false;
}

constexpr uint64_t KILO=1000;
constexpr uint64_t MEGA=KILO*KILO;
constexpr uint64_t GIGA=KILO*MEGA;

typedef struct __TIMER_ITEM__ {
	typedef struct __TIMER_ITEM__ Type;
	typedef struct __TIMER_ITEM__* PointerType;

	__TIMER_ITEM__(MutualTimer::INotified* p, const MutualTimer::tm_period& tmp): pno(p) {

		tu = tmp.unit;
		memset(&splited, 0, sizeof(splited));
		uint32_t sec, millisec, microsec, nano;

		switch (tu){
		case MutualTimer::tm_period::TIME_UNIT::SECONDS:
		{
			nanos = tmp.elapse * GIGA;
			splited[MutualTimer::tm_period::TIME_UNIT::SECONDS] = tmp.elapse;
			break;
		}
		case MutualTimer::tm_period::TIME_UNIT::MILLISECONDS:
		{
			nanos =  tmp.elapse * MEGA;
			splited[MutualTimer::tm_period::TIME_UNIT::SECONDS] = tmp.elapse / KILO;
			splited[MutualTimer::tm_period::TIME_UNIT::MILLISECONDS] = tmp.elapse % KILO;
			break;
		}
		case MutualTimer::tm_period::TIME_UNIT::MICROSECONDS:
		{
			nanos = tmp.elapse * KILO;
			sec = tmp.elapse / MEGA;
			millisec = (tmp.elapse % MEGA) / KILO;
			splited[MutualTimer::tm_period::TIME_UNIT::SECONDS] = sec;
			splited[MutualTimer::tm_period::TIME_UNIT::MILLISECONDS] = millisec;
			splited[MutualTimer::tm_period::TIME_UNIT::MICROSECONDS] = millisec % KILO;
			break;
		}
		case MutualTimer::tm_period::TIME_UNIT::NANOSECONDS:
		{
			nanos = tmp.elapse;
			sec = tmp.elapse / GIGA;
			millisec = (tmp.elapse % GIGA) / MEGA;
			microsec = (tmp.elapse % MEGA) / KILO;
			nano = (tmp.elapse % KILO);
			splited[MutualTimer::tm_period::TIME_UNIT::SECONDS] = sec;
			splited[MutualTimer::tm_period::TIME_UNIT::MILLISECONDS] = millisec;
			splited[MutualTimer::tm_period::TIME_UNIT::MICROSECONDS] = microsec;
			splited[MutualTimer::tm_period::TIME_UNIT::NANOSECONDS] = nano;
			break;
		}
		default:
			nanos = 0;
		}
		if (splited[MutualTimer::tm_period::TIME_UNIT::SECONDS]>0){
			tu = MutualTimer::tm_period::TIME_UNIT::SECONDS;
		}
		else if(splited[MutualTimer::tm_period::TIME_UNIT::MILLISECONDS]>0){
			tu = MutualTimer::tm_period::TIME_UNIT::MILLISECONDS;
		}
		else if(splited[MutualTimer::tm_period::TIME_UNIT::MICROSECONDS]>0){
			tu = MutualTimer::tm_period::TIME_UNIT::MICROSECONDS;
		}
		else {
			tu = MutualTimer::tm_period::TIME_UNIT::NANOSECONDS;
		}
	}

	MutualTimer::INotified* pno;
	uint64_t nanos;
	uint32_t splited[4];
	MutualTimer::tm_period::TIME_UNIT tu;

} Timer_Item_t, *PTimer_Item_t;

typedef Timer_Item_t* _PTR_SP_;

bool MutualTimer::registerNotified(INotified* pnotified, const tm_period& period) {

	if ( pnotified == nullptr or period.elapse == 0 ) {
		return false;
	}
	std::unique_ptr<Timer_Item_t> p(new Timer_Item_t(pnotified, period));
	if ( p->nanos > 0 ){
		std::unique_lock<std::mutex> lck(m_mtx_new);
		m_newcomers.push_back( reinterpret_cast<uintptr_t> (p.release()) );
		m_condv.notify_one();
		return true;
	}
	return false;
}

void MutualTimer::unregisterNotified(INotified* pnotified) {
	std::unique_lock<std::mutex> lck(m_mtx_unreg);
	m_unregistered.push_back(reinterpret_cast<uintptr_t>(pnotified));
}

bool MutualTimer::is_running() const {
	return m_running;
}

template<size_t Size>
void init_scale(uint64_t scales[Size], const uint64_t ratio){
	uint64_t step = 1000 / Size;
	uint64_t accum = step;

	for(size_t i = 0; i < Size; i+=1 ){
		scales[i] = accum * ratio;
		accum += step;
	}
}


#define SCALE_LENGTH size_t(100)

typedef struct __TIME_DISPATCHER__ {
	typedef MutualTimer::tm_period::TIME_UNIT TU;

	static uint64_t scales[NB_UNITS][SCALE_LENGTH];
	static const uint64_t ratios[NB_UNITS];

	typedef struct __TD_Node__ {
		__TD_Node__(): p_next(nullptr), p_prev(nullptr), time(0) {}
		__TD_Node__* p_next;
		__TD_Node__* p_prev;
		std::unique_ptr<Timer_Item_t> item;
		uint64_t time; //time to trigger timer in ns
	} TDNode, *PTDNode ;

	__TIME_DISPATCHER__() {
		memset(NODES, 0, sizeof(NODES));
	}
 
	uint32_t get_node_slot(_PTR_SP_ p){
		uint32_t slot = p->tu * SCALE_LENGTH;
		slot += std::min<uint32_t>(SCALE_LENGTH-1, p->splited[p->tu] / (KILO / SCALE_LENGTH));
		TRACE("insert Unit: " <<  int(p->tu) << " slot " << slot );
		return slot;
	}

	PTDNode insert_new(_PTR_SP_ p, uint64_t now){

		PTDNode n = new TDNode;
		n->time = p->nanos + now;
		n->item.reset(p);
		const uint32_t index = get_node_slot(p);
		if (NODES[index] == nullptr){
			slots.insert(index);
		}
		insert(n, &NODES[index]);
		return n;
	}

	PTDNode insert(PTDNode n, PTDNode* ppslot){

		PTDNode* pp_next = ppslot;
		PTDNode prev = nullptr;
		while (*pp_next and (*pp_next)->time < n->time){
			prev = *pp_next;
			pp_next = &(*pp_next)->p_next ;
		}
		n->p_prev = prev;
		n->p_next = *pp_next;
		if (*pp_next){
			(*pp_next)->p_prev = n;
		}
		*pp_next = n;
		return n;
	}

	PTDNode remove(PTDNode pn){

		if (pn->p_prev){
			pn->p_prev->p_next = pn->p_next;
		}
		else {
			uint32_t index = get_node_slot(pn->item.get());
			NODES[index] = pn->p_next;
			if (NODES[index] == nullptr){
				slots.erase(index);
			}
		}
		if (pn->p_next){
			pn->p_next->p_prev = pn->p_prev;
		}
		return pn->p_next;
	}

	PTDNode remove(PTDNode pn, uint32_t index_node){

		if (pn->p_prev){
			pn->p_prev->p_next = pn->p_next;
		}
		else {
			NODES[index_node] = pn->p_next;
			if (NODES[index_node] == nullptr){
				slots.erase(index_node);
			}
		}
		if (pn->p_next){
			pn->p_next->p_prev = pn->p_prev;
		}
		return pn->p_next;
	}
	void reschedule(PTDNode last, uint32_t index_node){
		while(NODES[index_node]!=last) {
			PTDNode pn = NODES[index_node];
			NODES[index_node] = pn->p_next;
			pn->p_next->p_prev = nullptr;

			PTDNode it = last, prev = it ? it->p_prev : nullptr;
			while (it and it->time < pn->time) {
				prev = it;
				it = it->p_next;
			}
			pn->p_next = it;
			pn->p_prev = prev;
			if (it) {
				it->p_prev = pn;
			}
			if(prev){
				prev->p_next = pn;
			}
		}
	}
	uint64_t trigger_timers(uint64_t now){
		uint64_t sleep_ns = 0xFFFFFFFFFFFFFFFF;
		std::set<uint> scpy = slots; 
		uint count = 0;

		for ( uint index: scpy ){

			TRACE("Trigger timer slot: " << index);
			PTDNode pn = NODES[index];
			while(pn and pn->time <= now){
				pn->item->pno->notify();
				pn->time = now + pn->item->nanos;
				pn = pn->p_next;
				count += 1;
			}
			reschedule(pn,index);
			sleep_ns = std::min<uint64_t>(sleep_ns, NODES[index]->time);
		}
		return sleep_ns - now;
	}

	//100 for each TIME_UNIT: SECONDS, MILLISECONDS, MICROSECONDS, NANOSECONDS
	PTDNode NODES[NB_UNITS*SCALE_LENGTH];
	std::set<uint> slots;

	class Init {
		friend __TIME_DISPATCHER__;
		Init() {
			init_scale<SCALE_LENGTH>(__TIME_DISPATCHER__::scales[TU::NANOSECONDS], ratios[TU::NANOSECONDS]);
			init_scale<SCALE_LENGTH>(__TIME_DISPATCHER__::scales[TU::MICROSECONDS], ratios[TU::MICROSECONDS]);
			init_scale<SCALE_LENGTH>(__TIME_DISPATCHER__::scales[TU::MILLISECONDS], ratios[TU::MILLISECONDS]);
			init_scale<SCALE_LENGTH>(__TIME_DISPATCHER__::scales[TU::SECONDS], ratios[TU::SECONDS]);
		}
	};
	static Init init;

} TIME_DISPATCHER;

//ratios in NanoSec
/*
const uint64_t TIME_DISPATCHER::ratios[TIME_DISPATCHER::TU::NANOSECONDS]  = 1;
const uint64_t TIME_DISPATCHER::ratios[TIME_DISPATCHER::TU::MICROSECONDS] = 1000;
const uint64_t TIME_DISPATCHER::ratios[TIME_DISPATCHER::TU::MILLISECONDS] = 1000000;
const uint64_t TIME_DISPATCHER::ratios[TIME_DISPATCHER::TU::SECONDS] 	  = 1000000000;
*/
const uint64_t TIME_DISPATCHER::ratios[NB_UNITS] = { 1, KILO, MEGA, GIGA };
uint64_t TIME_DISPATCHER::scales[NB_UNITS][SCALE_LENGTH];
TIME_DISPATCHER::Init TIME_DISPATCHER::init;

void MutualTimer::_private_running_loop() {
	typedef TIME_DISPATCHER::PTDNode PTDNode;
	typedef TIME_DISPATCHER::TDNode  TDNode;

	std::multimap<MutualTimer::INotified*,std::unique_ptr<TDNode>> registered_items;

	TIME_DISPATCHER dispatcher;

	std::list<uintptr_t> newcomers_exch;
	std::list<uintptr_t> unregistered_exch;
	std::future<void> future = m_exit_signal.get_future();
	std::mutex cond_mx;

	while(m_running){

		newcomers_exch.clear();
		unregistered_exch.clear();

		{
			std::unique_lock<std::mutex> lck(m_mtx_new);
			newcomers_exch = std::move(m_newcomers);
		}

		{
			std::unique_lock<std::mutex> lck(m_mtx_unreg);
			unregistered_exch = std::move(m_unregistered);
		}

		uint64_t now = Common_getSysclock();
		for (auto it = std::begin(newcomers_exch);  it != std::end(newcomers_exch); ++it ) {
			_PTR_SP_ p = reinterpret_cast<_PTR_SP_>(*it);
			PTDNode pn = dispatcher.insert_new(p, now);
			registered_items.insert( std::make_pair(p->pno, pn) );
		}
		for (auto it = std::begin(unregistered_exch); it != std::end(unregistered_exch); ++it ) {
			MutualTimer::INotified* pn = reinterpret_cast<MutualTimer::INotified*>(*it);
			auto itpn = registered_items.find(pn);
			if (itpn != registered_items.end()){
				dispatcher.remove(itpn->second.get());
				registered_items.erase(itpn);
			}
		}
		if (!registered_items.empty()) {
			//triggers elapsed timers
			uint64_t sleep = dispatcher.trigger_timers(Common_getSysclock());
			TRACE("Timers trigerred: next in " << sleep << " nanoseconds");
			future.wait_for(std::chrono::nanoseconds(sleep));
		}else {
			TRACE("No timer registered: wait");
			std::unique_lock<std::mutex> lock(cond_mx);
			m_condv.wait(lock, [&registered_items,this](){ return not registered_items.empty() or not m_running; });
			TRACE("Wake up, continue loop");
		}
	}
	TRACE("STOP TIMER loop");
}
