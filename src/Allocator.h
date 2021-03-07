#pragma once

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <glob.h>
#include <set>
#include <stdexcept>
#include <ostream>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include "double_chained_list.hpp"

#if USE_TRACE
#define USE_TRY_CATCH_DEBUG 	0

#define TRACE_STREAM(s,log)  s << log
#ifdef LOG_STREAM
#define TRACE(log) TRACE_STREAM(LOG_STREAM, log)
#else
#define TRACE(log) TRACE_STREAM(std::cout, log)
#endif

#if USE_TRY_CATCH_DEBUG
#define TRY_CATCH_DEBUG(expr, msg, log) try{ \
			expr; \
		} catch(...){ \
			std::ostringstream oss; \
			oss << log << endl; \
			_dump(oss); \
			TRACE(endl << oss.str()); \
			throw ::std::runtime_error(msg); \
	}
#else
#define TRY_CATCH_DEBUG(expr, msg, log) expr;
#endif
#else
#define TRACE(log)
#define TRY_CATCH_DEBUG(expr, msg, log) expr;
#endif

#define HEX_ADDR(addr) std::hex << std::setw(8) << std::setfill('0') << addr << std::dec

#define ADD_OFFSET(ptr,s) reinterpret_cast<void*>(reinterpret_cast<u_char*>(ptr)+s)


class MEM_TYPE_t;
class FREE_MEM_t;
class ALLOC_MEM_t;


class __Allocator {
public:
	__Allocator(void* ptr, size_t size);
	~__Allocator();
	uintptr_t allocate(size_t s);
	uintptr_t allocate_block(size_t s, uint count);
	void  free(uintptr_t ref, uint index=0, uint count=1);
	void* get_address(uintptr_t ref, uint index = 0);
	void copy(void* ptr, size_t count, uintptr_t ref);
private:
	FREE_MEM_t* _find_location(size_t s);
	void _add_descriptor(ALLOC_MEM_t* mem, FREE_MEM_t* free_mem);
	ALLOC_MEM_t* _find_desc(uintptr_t uptr);
	void _remove_allocated(ALLOC_MEM_t* p);
	void _insert_before(MEM_TYPE_t* ptr, MEM_TYPE_t* after);
	void _insert_after(MEM_TYPE_t* ptr, MEM_TYPE_t* before);
	void _insert_before(FREE_MEM_t* ptr, FREE_MEM_t* after);
	void _insert_after(FREE_MEM_t* ptr, FREE_MEM_t* before);
	void _replace(MEM_TYPE_t* ptr, MEM_TYPE_t* by);
	void _replace(FREE_MEM_t* ptr, FREE_MEM_t* by);
	FREE_MEM_t* _merge_free_mem(FREE_MEM_t* free_block, size_t required_size);
	void _remove_mem(FREE_MEM_t* free_mem);
	void _remove_mem(ALLOC_MEM_t* mem);
	void _dump(std::ostream& os) const; //debug purpose only

	const size_t _mem_size;//size max of memory
	void* _ptr_mem;//start of memory
	size_t _allocated;//whole allocated size
	std::set<ALLOC_MEM_t*> _allocs;//all allocated memory descriptors
	MEM_TYPE_t* _head, *_tail; //chained list for free and allocated memory blocks
	FREE_MEM_t* _fhead, *_ftail; //chained list for free memory blocks only: quickly find free memory slot
};

#endif
