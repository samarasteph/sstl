#include <cstring>
#include "Allocator.h"

namespace sstl {

	constexpr size_t _1Ko 				= 1024;
	constexpr size_t _2Ko 				= _1Ko << 1;
	constexpr size_t _4Ko 				= _2Ko << 1;
	constexpr size_t _8Ko				= _4Ko << 1;
	constexpr size_t 	_16Ko 			= _8Ko << 1;
	constexpr size_t 	_32Ko			= _16Ko << 1;
	constexpr size_t 	_64Ko 			= _32Ko << 1;
	constexpr size_t 	_128Ko 			= _64Ko << 1;
	constexpr size_t 	_256Ko 			= _128Ko << 1;
	constexpr size_t 	_512Ko 			= _256Ko << 1;
	constexpr size_t 	_1Mo 			= _512Ko << 1;

	template <class Type_t>
	class MemoryPtr {
		typedef Type_t pointer;

		MemoryPtr(__Allocator* a, uintptr_t descriptor): _a(a), _descriptor(descriptor){}
		MemoryPtr(MemoryPtr&& mptr): MemoryPtr(mptr._a), _descriptor(mptr._descriptor){
			mptr._a = mptr._descriptor = nullptr;
		}
		~MemoryPtr() {
			if (_a) {
				_a->free(_descriptor);
			}
		}
		Type_t& operator * () {
			return *_a->get_address(_descriptor);
		}
		operator pointer () {
			return _a->get_address(_descriptor);
		}

	private:
		uintptr_t _descriptor;
		__Allocator* _a;
	};

	template<class Type_t, size_t NbElemMin_t = _4Ko, size_t NbElemMax_t = NbElemMin_t>
	class allocator {
		typedef Type_t* pointer;
		typedef const Type_t* const_pointer;
		typedef Type_t& reference;
		typedef Type_t& const_reference;
		typedef Type_t 	value_type;

		typedef std::size_t     size_type;
		typedef std::ptrdiff_t  difference_type;


		pointer
		address(reference __x) const
		{ return std::__addressof(__x); }

		const_pointer
		address(const_reference __x) const
		{ return std::__addressof(__x); }

		// NB: __n is permitted to be 0.  The C++ standard says nothing
		// about what the return value is when __n == 0.
		pointer
		allocate(size_type __n, const void* = static_cast<const void*>(0))
		{


#if __cpp_aligned_new
			if (alignof(_Tp) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
			{
				std::align_val_t __al = std::align_val_t(alignof(_Tp));
				return static_cast<_Tp*>(::operator new(__n * sizeof(_Tp), __al));
			}
#endif

			//return static_cast<_Tp*>(::operator new(__n * sizeof(_Tp)));
			uintptr_t ptr = _alloc.allocate_block(sizeof(Type_t) , __n);
		}

		__Allocator _alloc;
		__Allocator* ptr;
		value_type buffer[NbElemMin_t];

	};

}
