
#include "Allocator.h"

#include <algorithm>
#include <bitset>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <assert.h>


using namespace std;


class MEM_TYPE_t {
public:
	MEM_TYPE_t(bool b): _free(b), _next(nullptr), _prev(nullptr) {}
	virtual ~MEM_TYPE_t(){}
	void remove_self(MEM_TYPE_t** head, MEM_TYPE_t** tail){
		if (_prev){
			_prev->_next = _next;
		}
		else
			*head = _next;

		if (_next){
			_next->_prev = _prev;
		}
		else
			*tail = _prev;
	}
	bool _free;
	MEM_TYPE_t *_next, *_prev;
};

/*
 *
 * Class for single memory allocation descriptor (i.e. new operator)
 */
class ALLOC_MEM_t: public MEM_TYPE_t {
public:
	ALLOC_MEM_t(void* ptr, size_t size): MEM_TYPE_t(false), _ptr(ptr), _size(size), _unit_size(size) {}
	void* _ptr;
	size_t _size; //total size
	size_t _unit_size; //for array allocations: size of one element

};

/*
 *
 * Class for array memory allocation descriptor (i.e. new[] operator)
 */
class ALLOC_ARRAY_MEM_t: public ALLOC_MEM_t {
	static const u_char _mod_bits[7];
public:
	ALLOC_ARRAY_MEM_t(void* ptr, size_t size, uint count): ALLOC_MEM_t(ptr, size*count), _count(count) {
		_unit_size = size;

		//set all element bits to 1 (not deleted). Position in bitfield is pos in array (<_count)
		uint modulo = _count & 0x07; // modulo div by 8
		_nb_octets = (_count+0x07) >> 3; // divide / 8 (+ 1 if modulo > 0)
		_remain.reset(new u_char[_nb_octets]);

		uint nboct = _count >> 3; //without modulo: parts with all bits set to 1
		memset(_remain.get(),0xff,nboct); // set all bits to 1
		if (modulo){ // set the "modulo" part => nb elements bits set to 1 < 8bits
			_remain[nboct] = _mod_bits[modulo-1];
		}
	}
	//all elements in array have been deleted
	bool all_deleted (){
		return std::all_of(_remain.get(), _remain.get()+_nb_octets, [](const u_char c){ return c == 0x00; });
	}

	void set_element_deleted(uint index){
		if (index < _count){
			uint index_octet = index >> 3;
			uint mod = index & 0x07;
			_remain[index_octet] = _remain[index_octet] & ~(1<<mod);
		}
	}
	uint _count; // nb of elements allocated in array
	std::unique_ptr<u_char[]> _remain; //bit field to keep which array element is not yet deleted
	uint _nb_octets; // nb octets needed to have a bitfield with nb bits >= _count

};

const u_char ALLOC_ARRAY_MEM_t::_mod_bits[7] = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f };

/*
 *
 * Class For free memory descriptor
 */
class FREE_MEM_t: public MEM_TYPE_t {
public:
	FREE_MEM_t(void* start, size_t s):
		MEM_TYPE_t(true), _start(start), _size(s), _fprev(nullptr), _fnext(nullptr)
	{
		set_size(start,s);
	}
	void set_size(void* start, size_t s){
		_start = start;
		_size = s;
	}
	void* end() {
		return reinterpret_cast<void*>( reinterpret_cast<u_char*>(_start) + _size );
	}
	void remove_self(FREE_MEM_t** fhead, FREE_MEM_t** ftail, MEM_TYPE_t** head, MEM_TYPE_t** tail){
		if (_fprev){
			_fprev->_fnext = _fnext;
		}else
			*fhead = _fnext;
		if(_fnext){
			_fnext->_fprev = _fprev;
		}
		else
			*ftail = _fprev;
		MEM_TYPE_t::remove_self(head, tail);
	}
	void* _start;
	size_t _size;
	FREE_MEM_t *_fprev, *_fnext;
};

static size_t NB_ALLOCS;

__Allocator::__Allocator(void* ptr, size_t size):
		_mem_size(size), _ptr_mem(ptr), _allocated(0) {
	_head = _tail = _ftail = _fhead = new FREE_MEM_t(_ptr_mem, _mem_size);
	TRACE("allocator init base ptr: " << HEX_ADDR(_ptr_mem) << " size=" << _mem_size <<
			" Free Mem descriptor " << HEX_ADDR(_fhead) << endl);
}
__Allocator::~__Allocator() {
	std::for_each(_allocs.begin(), _allocs.end(), [](ALLOC_MEM_t* d){ delete d; });
}

//single element allocation
uintptr_t __Allocator::allocate(size_t s){
	NB_ALLOCS += 1;
	FREE_MEM_t* free_mem = _find_location(s);
	ALLOC_MEM_t* mem = new ALLOC_MEM_t(free_mem->_start, s);
	_add_descriptor(mem,free_mem);
	return reinterpret_cast<uintptr_t>(mem);
}
//array of n elements allocation
uintptr_t __Allocator::allocate_block(size_t s, uint count){
	NB_ALLOCS += 1;
	FREE_MEM_t* free_mem = nullptr;
	TRY_CATCH_DEBUG(free_mem = _find_location(s*count), "error finding location", "free_mem size=" << (s*count));
	ALLOC_ARRAY_MEM_t *mem = new ALLOC_ARRAY_MEM_t(free_mem->_start, s, count);
	TRY_CATCH_DEBUG(_add_descriptor(mem,free_mem), "error adding block descriptor", "free_mem descr address=" << HEX_ADDR(free_mem)
			<< " start ptr=" << HEX_ADDR(free_mem->_start) << " size=" << free_mem->_size
			 << " alloc mem address=" << HEX_ADDR(mem) << " memory ptr=" << HEX_ADDR(mem->_ptr) << " size=" << mem->_size)
	TRACE("allocate memory location descriptor " << HEX_ADDR(mem) << " mem ptr: " << HEX_ADDR(mem->_ptr) << endl);

	return reinterpret_cast<uintptr_t>(mem);
}

inline
FREE_MEM_t*  __Allocator::_find_location(size_t s){

	if (_mem_size < _allocated+s){
		TRACE("Cannot allocate size= " << s << " total exceed " << _mem_size << "(" << _allocated+s << ")");
		throw std::bad_alloc();
	}

	FREE_MEM_t* p = _fhead;
	while(p and p->_size < s){
		p=p->_fnext;
	}

	if (p==nullptr){
		//memory is fragmented
		double fragmented_percent = static_cast<double>(_mem_size-_allocated) / static_cast<double>(_mem_size)*100;
		TRACE("Cannot allocate memory because too fragmented " <<  fragmented_percent << "% (allocated=" << _allocated << " total memory=" << _mem_size << ", total allocs=" << NB_ALLOCS << ")");
		std::ostringstream oss_dump;
		_dump(oss_dump);
#ifndef USE_TRACE
		TRACE(oss_dump.str());
#endif
		p = _merge_free_mem(_fhead, s);
	}
	return p;
}

inline
ALLOC_MEM_t* __Allocator::_find_desc(uintptr_t ref){
	auto it = _allocs.find(reinterpret_cast<ALLOC_MEM_t*>(ref));
	if(it != _allocs.end()){
		return *it;
	}
	return nullptr;
}

inline
void __Allocator::_insert_before(MEM_TYPE_t* ptr, MEM_TYPE_t* after){
	if (after->_prev){
		after->_prev->_next = ptr;
	}
	else{
		_head = ptr;
	}
	ptr->_prev = after->_prev;
	after->_prev = ptr;
	ptr->_next = after;
}

inline void __Allocator::_insert_after(MEM_TYPE_t *ptr, MEM_TYPE_t *before) {
	if(before->_next){
		before->_next->_prev = ptr;
	}else{
		_tail = ptr;
	}
	ptr->_next = before->_next;
	before->_next = ptr;
	ptr->_prev = before;
}

void __Allocator::_insert_before(FREE_MEM_t* ptr, FREE_MEM_t* after){
	if (after->_fprev){
		after->_fprev->_fnext = ptr;
	}
	else{
		_fhead = ptr;
	}
	ptr->_fprev = after->_fprev;
	after->_fprev = ptr;
	ptr->_fnext = after;
}

void __Allocator::_insert_after(FREE_MEM_t* ptr, FREE_MEM_t* before){
	if(before->_fnext){
		before->_fnext->_fprev = ptr;
	}else{
		_ftail = ptr;
	}
	ptr->_fnext = before->_fnext;
	before->_fnext = ptr;
	ptr->_fprev = before;
}

void __Allocator::_replace(MEM_TYPE_t* ptr, MEM_TYPE_t* by){
	if (ptr->_next){
		ptr->_next->_prev = by;
	}else{
		_tail = by;
	}
	if (ptr->_prev){
		ptr->_prev->_next = ptr;
	}else{
		_head = by;
	}
	by->_prev = ptr->_prev;
	by->_next = ptr->_next;
}
void __Allocator::_replace(FREE_MEM_t* ptr, FREE_MEM_t* by){
	if (ptr->_fnext){
		ptr->_fnext->_fprev = by;
	}else{
		_ftail = by;
	}
	if (ptr->_fprev){
		ptr->_fprev->_fnext = ptr;
	}else{
		_fhead = by;
	}
	by->_fprev = ptr->_fprev;
	by->_fnext = ptr->_fnext;
}

void __Allocator::_remove_mem(FREE_MEM_t* free_mem){
	free_mem->remove_self(&_fhead, &_ftail, &_head, &_tail);
}

void __Allocator::_remove_mem(ALLOC_MEM_t* mem){
	mem->remove_self(&_head, &_tail);
}

inline
void __Allocator::_add_descriptor(ALLOC_MEM_t *mem, FREE_MEM_t *free_mem) {
	//alloc from left edge of free mem

	size_t s = mem->_size;
	//increase alloc size
	_allocated+=s;

	TRACE("New allocation at free memory block descr=" << HEX_ADDR(free_mem) << " mem ptr=" << HEX_ADDR(free_mem->_start) << " size=" << s << " (allocated=" << _allocated << " total=" << _mem_size << ")");
	_insert_before(mem,free_mem);
	//check if free memory has room left
	if (free_mem->_size>s){

		//shrink free_mem by allocated size
		free_mem->set_size(ADD_OFFSET(free_mem->_start,s) , free_mem->_size-s);
	}
	else {	// free mem is full => delete it

		free_mem->remove_self(&_fhead,&_ftail,&_head,&_tail);
		TRACE(" !! Free memory block is full !! delete block descriptor of free_mem="  << HEX_ADDR(free_mem)
				<< " start ptr=" << HEX_ADDR(free_mem->_start) << " size=" << free_mem->_size
				<< "_fhead=" << HEX_ADDR(_fhead) << " _ftail=" << HEX_ADDR(_ftail) << endl);
		delete free_mem;
	}

	_allocs.insert(mem);

	TRACE(" Allocated memory block descr=" << HEX_ADDR(mem) << " mem ptr=" << HEX_ADDR(mem->_ptr) << " Prev=" << HEX_ADDR(mem->_prev) << " Next="
			<< HEX_ADDR(mem->_next) << " free=" << boolalpha << mem->_free << " allocated="<< _allocated << " total=" << _mem_size << endl );
}

inline
void __Allocator::_remove_allocated(ALLOC_MEM_t* p){
	FREE_MEM_t* fp;
	_allocated -= p->_size;
	TRACE("Remove descriptor " << HEX_ADDR(p) << " Prev=" << HEX_ADDR(p->_prev)
			<< " Next=" << HEX_ADDR(p->_next) << " => " );
	if (p->_prev == nullptr and p->_next == nullptr){ //mem is head and tail
		TRACE("1) remove head-tail block" << endl);
		fp = new FREE_MEM_t(_ptr_mem, _mem_size);
		_head = _tail = fp;
		_fhead = _ftail = fp;
		TRACE("Free memory block " << HEX_ADDR(fp) << " fnext=" << HEX_ADDR(fp->_fnext) << " fprev=" << HEX_ADDR(fp->_fprev) );
		return;
	}
	if (p->_prev == nullptr){ //mem is head

		if (p->_next->_free){
			fp = static_cast<FREE_MEM_t*>(p->_next);
			TRACE("2) remove head block--extend right free mem descr=" << HEX_ADDR(fp) << " size " << fp->_size + p->_size <<endl);
			fp->_prev = p->_prev;

			assert(fp->_fprev == nullptr);
			fp->set_size(p->_ptr, fp->_size + p->_size);
			assert( _fhead == fp );
		}
		else{

			fp = new FREE_MEM_t(p->_ptr,p->_size);
			TRACE("3) remove head block--add new free mem descr=" << HEX_ADDR(fp) << " size " << p->_size <<endl);

			fp->_next = p->_next;
			p->_next->_prev = fp;
			_head = fp;

			if(_fhead){
				_fhead->_fprev = fp;
			}
			else{
				_ftail = fp;
			}
			fp->_fnext = _fhead;
			_fhead = fp;
		}
		TRACE("Free memory block " << HEX_ADDR(fp) << " fnext=" << HEX_ADDR(fp->_fnext) << " fprev=" << HEX_ADDR(fp->_fprev) );
		return;
	}
	if (p->_next == nullptr){ //mem is tail
		if (p->_prev->_free){
			fp = static_cast<FREE_MEM_t*>(p->_prev);
			TRACE("4) remove tail block--extend left free mem descr=" << HEX_ADDR(fp) << " size " << fp->_size + p->_size <<endl);
			fp->_next = p->_next;

			assert( fp->_fnext == nullptr );
			fp->set_size(fp->_start, fp->_size+p->_size);

			assert( _ftail == fp );
		}
		else{
			fp = new FREE_MEM_t(p->_ptr,p->_size);
			TRACE("5) remove tail block--add new free mem descr=" << HEX_ADDR(fp) << " size " << p->_size <<endl);

			fp->_prev = p->_prev;
			p->_prev->_next = fp;
			_tail = fp;

			if(_ftail){
				_ftail->_next = fp;
			}else{
				_fhead = fp;
			}
			fp->_fprev = _ftail;
			_ftail = fp;

			if(_fhead == nullptr) {
				_fhead = fp;
			}
		}
		TRACE("Free memory block " << HEX_ADDR(fp) << " fnext=" << HEX_ADDR(fp->_fnext) << " fprev=" << HEX_ADDR(fp->_fprev) );
		return;
	}
	if (p->_prev->_free and p->_next->_free){
		//extend left free mem towards right and delete free mem at right
		FREE_MEM_t* del = static_cast<FREE_MEM_t*>(p->_next);
		fp = static_cast<FREE_MEM_t*>(p->_prev);

		TRACE("6) remove block--extend left free memory descr=" << HEX_ADDR(fp) << " to "
				<< fp->_size+p->_size+del->_size << " -- delete right free mem " << HEX_ADDR(del) << endl);

		fp->_next = del->_next;
		if (fp->_next) fp->_next->_prev = fp;
		else _tail = fp;

		fp->_fnext = del->_fnext;
		if(fp->_fnext) fp->_fnext->_fprev = fp;
		else _ftail = fp;
		fp->set_size(fp->_start, fp->_size+p->_size+del->_size);
		delete del;
		TRACE("Free memory block " << HEX_ADDR(fp) << " fnext=" << HEX_ADDR(fp->_fnext) << " fprev=" << HEX_ADDR(fp->_fprev) );
		return;
	}
	if (p->_prev->_free) {
		fp = static_cast<FREE_MEM_t*>(p->_prev);
		TRACE("7) remove block--extend left free memory descr=" << HEX_ADDR(fp) << " to " << fp->_size+p->_size << endl);
		fp->_next = p->_next;
		fp->_next->_prev = fp;

		fp->set_size(fp->_start, fp->_size+p->_size);
		TRACE("Free memory block " << HEX_ADDR(fp) << " fnext=" << HEX_ADDR(fp->_fnext) << " fprev=" << HEX_ADDR(fp->_fprev) );
		return;
	}
	if(p->_next->_free){
		fp = static_cast<FREE_MEM_t*>(p->_next);
		TRACE("8) remove block--extend right free memory descr=" << HEX_ADDR(fp) << " to " << fp->_size+p->_size << endl);
		fp->_prev = p->_prev;
		fp->_prev->_next = fp;

		fp->set_size(p->_ptr, p->_size+fp->_size);
		TRACE("Free memory block " << HEX_ADDR(fp) << " fnext=" << HEX_ADDR(fp->_fnext) << " fprev=" << HEX_ADDR(fp->_fprev) );
		return;
	}
	fp = new FREE_MEM_t(p->_ptr,p->_size);
	TRACE("9) remove block size="<< dec << p->_size << ". Create new free memory descr=" << HEX_ADDR(fp) << " of size " << p->_size << endl);
	fp->_next = p->_next;
	fp->_prev = p->_prev;
	fp->_next->_prev = fp;
	fp->_prev->_next = fp;

	MEM_TYPE_t* neighbour;
	//search next left free memory
	neighbour = fp->_prev;
	while(neighbour and not neighbour->_free){
		neighbour = neighbour->_prev;
	}
	fp->_fprev = static_cast<FREE_MEM_t*>(neighbour);
	if (neighbour == nullptr){
		_fhead = fp;
		TRACE("\t free mem desc is HEAD " << HEX_ADDR(fp) << endl);
	}else{
		static_cast<FREE_MEM_t*>(neighbour)->_fnext = fp;
	}
	//search next right free memory
	neighbour = fp->_next;
	while(neighbour and not neighbour->_free){
		neighbour = neighbour->_next;
	}
	fp->_fnext = static_cast<FREE_MEM_t*>(neighbour);
	if(neighbour==nullptr){
		_ftail = fp;
		TRACE("\t free mem desc is TAIL " << HEX_ADDR(fp) << endl);
	}else{
		static_cast<FREE_MEM_t*>(neighbour)->_fprev = fp;
	}
	TRACE("Free memory block " << HEX_ADDR(fp) << " fnext=" << HEX_ADDR(fp->_fnext) << " fprev=" << HEX_ADDR(fp->_fprev) );
}

inline FREE_MEM_t * __Allocator::_merge_free_mem(FREE_MEM_t *free_block, size_t required_size) {

	while(free_block->_size < required_size){

		MEM_TYPE_t* block = free_block->_next;
		void* ptr_reloc = free_block->_start;

		size_t alloc_size = 0;
		while(block and not block->_free){
			ALLOC_MEM_t* alloc_ptr = static_cast<ALLOC_MEM_t*>(block);
			alloc_size += alloc_ptr->_size;
			alloc_ptr->_ptr = ptr_reloc;
			ptr_reloc = reinterpret_cast<u_char*>(ptr_reloc) + alloc_ptr->_size;
			block = block->_next;
		}

		if(alloc_size>0){

			u_char* start = reinterpret_cast<u_char*>(free_block->_start);
			u_char* end = reinterpret_cast<u_char*>(free_block->_start)+alloc_size;

			while(start < end){
				size_t free_block_size = std::min<size_t>(alloc_size, free_block->_size);
				memcpy(start, start+free_block_size, free_block_size);
				alloc_size -= free_block_size;
				start += free_block_size;
			}
		}
		if (block){ //merge free blocks
			FREE_MEM_t* next_free = static_cast<FREE_MEM_t*>(block);
			next_free->set_size(ptr_reloc, next_free->_size+free_block->_size);
			_remove_mem(free_block);
			delete free_block;
			free_block = next_free;
		}
		else{
			break;
		}
	}
	return free_block;
}

void __Allocator::_dump(std::ostream& os) const {
	using namespace std;
	os << "--- [Memory Dump] ---" << endl;
	os << "total size="<< _mem_size << ", allocated size=" << _allocated << endl;
	os << "[Memory Blocks]" << endl;
	MEM_TYPE_t* p = _head;
	/*try*/{
		while(p){
			if(p->_free){
				FREE_MEM_t* pf = static_cast<FREE_MEM_t*>(p);
				os << "[ free=true ,ptr=" << HEX_ADDR(pf->_start) << dec << ",size=" << pf->_size << "]" << endl;
			}else{
				ALLOC_MEM_t* pa = static_cast<ALLOC_MEM_t*>(p);
				os << "[ free=false, ptr=" << HEX_ADDR(pa->_ptr) << ",size=" << pa->_size << ",unit_size="
						<< pa->_unit_size;
				if (pa->_size > pa->_unit_size){
					ALLOC_ARRAY_MEM_t* par = static_cast<ALLOC_ARRAY_MEM_t*>(pa);
					os << ",count=" << par->_count << ",remains=[" << hex << setfill('0') << setw(1);
					for(uint i=0; i < par->_nb_octets; ++i)
						os << (uint)par->_remain[i] << " ";
					os << dec << " ]";
				}
				os << " ]" << endl;
			}
			p = p->_next;
		}
		os << "[End]" << endl;
		os << "[Free Blocks]" << endl;
		FREE_MEM_t* pf = _fhead;
		while(pf){
			os << "[ ptr=" << HEX_ADDR(pf->_start) << ",size=" << pf->_size << "]" << endl;
			pf = pf->_fnext;
		}
	}
//	catch(...){
//		os << "Error while dumping memory: faulty block descr=" << HEX_ADDR(p) << endl;
//	}
	os << "[End]" << endl;
}

void  __Allocator::free(uintptr_t ref, uint index, uint count){
	ALLOC_MEM_t* p = _find_desc(ref);
	if(p){
		bool done = false;
		if (p->_size == p->_unit_size){

			_remove_allocated(p);
			TRACE("Remove all single allocation memory: descr=" << HEX_ADDR(p) << " mem ptr=" << HEX_ADDR(p->_ptr) <<endl);
			done = true;
		}else{
			ALLOC_ARRAY_MEM_t* pa = static_cast<ALLOC_ARRAY_MEM_t*>(p);
			if (index > pa->_count ){
				throw std::runtime_error("index greater than nb of array elements");
			}
			count = std::min<uint>(pa->_count-index, count);
			const uint max = index+count;
			for ( ; index < max ; index+=1){

				pa->set_element_deleted(index);
				if( pa->all_deleted() ){
					TRACE("Remove array allocation memory descr=" << HEX_ADDR(pa) << " mem ptr=" << HEX_ADDR(p->_ptr) << endl);
					TRY_CATCH_DEBUG(_remove_allocated(p), "error while removing descriptor", "error while removing descriptor address=" << HEX_ADDR(p) << " ptr mem=" << HEX_ADDR(p->_ptr) << " prev="
							<< HEX_ADDR(p->_prev) << " next=" << HEX_ADDR(p->_next) << endl)
					done = true;
					break;
				}
			}
			if (done){
				_allocs.erase(p);
				delete p;
				TRACE("\t end remove => _fhead=" << HEX_ADDR(_fhead) << " _ftail=" << HEX_ADDR(_ftail)
						<< " allocated=" << _allocated << " total=" << _mem_size <<endl);
			}
		}

	}
	else
		throw std::runtime_error("Bad memory descriptor");
}

void* __Allocator::get_address(uintptr_t ref, uint index){

	ALLOC_MEM_t* d = _find_desc(ref);
	if(d){
		if(d->_size==d->_unit_size)
			return d->_ptr;
		else {
			uint8_t* ptr = reinterpret_cast<uint8_t*>(d->_ptr);
			uint8_t* ptr_elem = ptr + (index*d->_unit_size);
			if(ptr+d->_size>ptr_elem){
				return  reinterpret_cast<void*>(ptr_elem);
			}
		}
	}
	return nullptr;
}

void __Allocator::copy(void* ptr, size_t count, uintptr_t ref){
	ALLOC_MEM_t* d = reinterpret_cast<ALLOC_MEM_t*>(ref);
	//auto it = _descr.find(d);
	//if(it!=_descr.end())
	{
		memcpy(d->_ptr, ptr, count );
	}
}
