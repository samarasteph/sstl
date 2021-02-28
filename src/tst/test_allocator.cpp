#include <fstream>
#include <ctime>
#include <sstream>
#include <gtest/gtest.h>

std::ostringstream oss;
#define LOG_STREAM oss
#define USE_TRACE		1

#include "Allocator.h"

typedef struct{
	int tab[150];
	unsigned char blob[256];
} Payload_t;

TEST( Allocator, SimpleCases ){

	Payload_t mem[256];
	Payload_t buffer[256];
	u_char* ptr;
	uint count;
	u_char signature;
	size_t len;
	__Allocator alloc(reinterpret_cast<void*>(mem), sizeof(mem));

//	unsigned int allocations[] = { 100, 50, 23, 78 };
//	unsigned int deallocations[] = { };

	count = 20; signature=0xaa;
	uintptr_t ref1 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref1);

	ptr = (u_char*)alloc.get_address(ref1);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 37; signature=0xbb;
	uintptr_t ref2 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref2);

	ptr = (u_char*)alloc.get_address(ref2);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 75; signature=0xcc;
	uintptr_t ref3 = alloc.allocate_block(sizeof(Payload_t), count);

	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref3);

	ptr = (u_char*)alloc.get_address(ref3);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 20; signature=0xaa;
	ptr = (u_char*)alloc.get_address(ref1);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 37; signature=0xbb;
	ptr = (u_char*)alloc.get_address(ref2);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	alloc.free(ref2, 0, 37);
	alloc.free(ref1, 0, 20);
	alloc.free(ref3, 0, 75);


	count = 220; signature=0xaa;
	ref1 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref1);

	ptr = (u_char*)alloc.get_address(ref1);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 36; signature=0xbb;
	ref2 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref2);

	ptr = (u_char*)alloc.get_address(ref2);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	alloc.free(ref1, 0, 220);
	alloc.free(ref2, 0, 36);

	count = 220; signature=0xaa;
	ref1 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref1);

	ptr = (u_char*)alloc.get_address(ref1);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 36; signature=0xbb;
	ref2 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref2);

	ptr = (u_char*)alloc.get_address(ref2);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	alloc.free(ref2, 0, 36);
	alloc.free(ref1, 0, 220);


	count = 256; signature=0xaa;
	ref1 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref1);

	ptr = (u_char*)alloc.get_address(ref1);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );
	alloc.free(ref1, 0, 220);
	alloc.free(ref1, 220, 36);

	count = 20; signature=0xaa;
	ref1 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref1);

	ptr = (u_char*)alloc.get_address(ref1);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 37; signature=0xbb;
	ref2 = alloc.allocate_block(sizeof(Payload_t), count);
	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref2);

	ptr = (u_char*)alloc.get_address(ref2);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 75; signature=0xcc;
	ref3 = alloc.allocate_block(sizeof(Payload_t), count);

	len = sizeof(Payload_t)*count;
	memset(buffer, signature, len);
	alloc.copy(buffer, len, ref3);

	ptr = (u_char*)alloc.get_address(ref3);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 20; signature=0xaa;
	ptr = (u_char*)alloc.get_address(ref1);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	count = 37; signature=0xbb;
	ptr = (u_char*)alloc.get_address(ref2);
	ASSERT_TRUE( std::all_of(ptr,ptr+count*sizeof(Payload_t), [signature](const u_char c){ return c == signature; }) );

	alloc.free(ref1, 0, 20);
	alloc.free(ref2, 0, 37);
	alloc.free(ref3, 0, 75);


}

constexpr uint Action_Alloc = 0;
constexpr uint Action_Dealloc = 1;
//	constexpr uint Action_Read = 2;
//	constexpr uint Action_Copy = 3;
constexpr uint Action_None = 4;

typedef struct record {
	explicit record(): action(Action_None), size(0), byte_size(0), ref(0), success(false) {}
	record(const record& r): action(r.action), size(r.size), byte_size(r.byte_size), ref(r.ref), success(r.success){}
	record(record&&) = delete;
	uint action;
	size_t size;
	size_t byte_size;
	uintptr_t ref;
	bool success;
}record_t;

std::ostream& operator << (std::ostream& ost, const record_t& r){

	ost << "record action=" << r.action << " ref=" << HEX_ADDR(r.ref) << " size=" << std::dec << r.size << " byte_size="
			<< r.byte_size << " success=" << r.success << std::endl;
	return ost;
}

void clean_file(){
	std::ofstream f("result.log", std::ios_base::trunc);
	if(f.is_open()){
		f.close();
	}
}
void flush_tofile(){
	std::ofstream f("result.log", std::ios_base::out|std::ios_base::app);
	if(f.is_open()){
		f << oss.str();
		f.close();
	}
	oss.seekp(0,std::ios_base::beg);
}

void segfault_handler(int){
	oss << std::endl << "!!Segmentation fault!!" << std::endl;
	flush_tofile();
	abort();
}

void set_sighandler(){
	struct sigaction act;
	act.sa_handler = segfault_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGSEGV, &act, 0);
}

TEST( Allocator, RandomTest ){

	constexpr size_t NB_ITEMS = 256;
	constexpr size_t	SIZE_ITEM = sizeof(Payload_t);

	Payload_t mem[NB_ITEMS];

	std::vector<uintptr_t> allocs;

	const std::vector<size_t> alloc_sizes = { 7, 12, 19, 25, 57 };
	const std::vector<uint> actions = {Action_Alloc, Action_Dealloc};

	std::time_t t,end;

	std::time(&t);
	std::srand(t);
	//std::srand(1600970261);

	end = t+35;

	__Allocator alloc(reinterpret_cast<void*>(mem), sizeof(mem));

	size_t allocated = 0;
	uint count = 20;
	allocs.reserve(NB_ITEMS);

	clean_file();
	oss.seekp(0,std::ios_base::beg);
	set_sighandler();

	TRACE("---------- Random Seed=" << t << std::endl);
	flush_tofile();
	do{
		size_t alloc_size = alloc_sizes[ static_cast<size_t>(std::rand()) % alloc_sizes.size() ];
		if(allocated+alloc_size <= NB_ITEMS){
			try{
				TRACE("Attempt alloc size=" << alloc_size << std::endl);
				uintptr_t ref = alloc.allocate_block(SIZE_ITEM, alloc_size);
				if (ref == 0){
					throw std::invalid_argument("Ref returned is zero");
				}
				record_t r;
				r.action = Action_Alloc;
				r.ref = ref;
				r.size = alloc_size;
				r.byte_size = alloc_size * SIZE_ITEM;
				r.success = true;
				TRACE("\t**" << r);
				allocs.push_back( ref );
				size_t* ps = reinterpret_cast<size_t*>(alloc.get_address(ref));

				if(ps==nullptr){
					std::ostringstream oss;
					TRACE( "Memory pointer returned for ref=" << HEX_ADDR(ref) << " is null" << std::endl);
					allocs.pop_back();
					throw std::invalid_argument(oss.str());
				}

				*ps = alloc_size;
				allocated+=alloc_size;

			}catch(std::bad_alloc& ba){
				TRACE( "error allocating size=" << alloc_size << std::endl );
				FAIL();
			}catch(std::runtime_error& r){
				TRACE( "runtime error line " << __LINE__ << ": " << r.what() << std::endl );
				FAIL();
			}
			catch(std::invalid_argument& ia){
				TRACE( "Invalid argument error: " << ia.what() << std::endl );
				FAIL();
			}
		}else {
			count += 1;
		}
	}while (count < 25 && allocated <= NB_ITEMS);

	TRACE( "<<<<<<<<<<<<<<<<<<<<<<< End of pre alloc (" << std::dec << allocs.size() <<  " items allocated) >>>>>>>>>>>>>>>>>>>>>>>" << std::endl );

	flush_tofile();

	record_t r;
	while(t<end){

		uint action = std::rand() % actions.size();

		r.action = action;
		r.size = 0;
		r.ref = 0;

		switch ( r.action ){
		case Action_Alloc:
			r.size = alloc_sizes[std::rand() % alloc_sizes.size()];
			r.byte_size = r.size * SIZE_ITEM;
			if(r.size+allocated <= NB_ITEMS){
				try {
					r.ref = alloc.allocate_block(r.size,SIZE_ITEM);
					r.success = true;
					TRACE("\t**" << r << std::endl);

					size_t* ps = reinterpret_cast<size_t*>(alloc.get_address(r.ref,0));
					if(ps==nullptr){
						std::ostringstream oss;
						TRACE( "Cannot read memory at " << HEX_ADDR(r.ref) << ". Null pointer returned" << std::endl);
						throw std::runtime_error(oss.str());
					}

					allocated += r.size;
				}catch(std::bad_alloc&){
					TRACE( "Allocating nb items=" << r.size << " nb_bytes="<< r.byte_size << " failed (allocated="<< allocated << ")" << std::endl );
					r.success = false;
					TRACE("\t**" << r << std::endl);
					goto ____FAIL____;
				}
			}
//			else{
//				std::cout << "Allocation of " << r.size << " exceeds max items " << dec << allocated << std::endl;
//			}
			break;
		case Action_Dealloc:
			try{
				if(not allocs.empty()){
					uint pos = static_cast<size_t>(std::rand()) % allocs.size();
					r.ref = allocs[pos];
					size_t* ps = reinterpret_cast<size_t*>(alloc.get_address(r.ref,0));
					if(ps){
						alloc.free(r.ref,0,*ps);
						allocs.erase(allocs.begin() + pos);
						allocated -= *ps;
					}else{
						std::ostringstream ostr;
						std::string s("cannot read allocated memory at ");
						TRACE( HEX_ADDR(r.ref) << std::endl);
						s += ostr.str() + " for pos=" + std::to_string(pos);
						throw std::invalid_argument(s);
					}
					r.success = true;
					TRACE("\t** " << r << std::endl);
				}
//				else{
//					std::cout << "No items allocated" << std::endl;
//				}
			}catch(std::runtime_error& re){
				r.success = false;
				TRACE("runtime error line " << __LINE__ << ": " << re.what() << std::endl);
				TRACE(r);
				goto ____FAIL____;
			}catch(std::invalid_argument& ia){
				std::cout << "Invalid argument error: " << ia.what() << std::endl;
				r.success = false;
				TRACE(r << std::endl);
				goto ____FAIL____;
			}
		}
		std::time(&t);
	}
	return;
____FAIL____:

	flush_tofile();
	FAIL();
}
