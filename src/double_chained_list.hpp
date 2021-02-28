/*
 * double_chained_list.hpp
 *
 *  Created on: 25 oct. 2020
 *      Author: aldu
 */

#ifndef DOUBLE_CHAINED_LIST_HPP_
#define DOUBLE_CHAINED_LIST_HPP_

#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <functional>

namespace sstl {
/**
 *
 * 	Double chained list
 */

	template<class Type_t>
	class double_chained_list {

	public:

		typedef Type_t value_type;

		double_chained_list(): _head(&_end), _tail(&_end){}
		double_chained_list(const double_chained_list<Type_t>& lst): double_chained_list(){
			iterator it = end();
			std::copy(lst.begin(), lst.end(),
					std::inserter(*this, it));
		}
		double_chained_list(double_chained_list&& lst): _head(lst._head), _tail(lst._tail) {
			if(_tail->_prev){
				_tail->_prev->_next = &_end;
			}
			_end._prev = _tail->_prev;
			lst._head = lst._tail = &lst._end;
		}
		virtual ~double_chained_list() {
			clear();
		}
	private:
		typedef double_chained_list<Type_t> MyType;
		struct __Node {
			friend MyType;
			__Node(): _type(), _prev(nullptr), _next(nullptr){}
			__Node(Type_t& type):  _type(type), _prev(nullptr), _next(nullptr){}
			__Node(Type_t&& type): _type(type), _prev(nullptr), _next(nullptr){}
		private:
			Type_t _type;
			__Node* _next;
			__Node* _prev;
		};
		typedef struct __Node _N;

		template <class ExposedType_t>
		struct __iterator_templ{
			friend MyType;
			typedef struct std::bidirectional_iterator_tag iterator_category;
			typedef ExposedType_t value_type;
			typedef ExposedType_t* pointer;
			typedef ExposedType_t& reference;
			typedef ssize_t	difference_type;

			typedef __iterator_templ<ExposedType_t> MyItType;

			__iterator_templ(): _n(nullptr){}
			__iterator_templ(const __iterator_templ& it): _n(it._n){}
			__iterator_templ(__iterator_templ&& it): _n(it._n){ it._n = nullptr; }
			__iterator_templ& operator = (const __iterator_templ& it){ if (this!=&it) _n = it._n ; return *this; }
			__iterator_templ& operator = (__iterator_templ&& it){ _n = it._n; it._n = nullptr; return *this; }
			ExposedType_t& operator *() { return _n->_type; }
			ExposedType_t* operator ->() { return &_n->_type; }
			__iterator_templ operator ++ () 	{ __check_next(); _n = _n->_next; return static_cast<__iterator_templ&>(*this); }
			__iterator_templ operator ++ (int) { __check_next(); __iterator_templ it(*this);  _n = _n->_next; return  std::move(it); }
			__iterator_templ operator -- () 	{ __check_prev(); _n = _n->_prev; return static_cast<__iterator_templ&>(*this); }
			__iterator_templ operator -- (int) { __check_prev(); __iterator_templ it(*this);  _n = _n->_prev; return std::move(it); }
			bool operator == (const __iterator_templ& i){ return _n == i._n; }
			bool operator == (const __iterator_templ& i) const { return _n == i._n; }
			bool operator != (const __iterator_templ& i){ return _n != i._n; }
			bool operator != (const __iterator_templ& i)const { return _n != i._n; }
//			operator __iterator_templ<const Type_t> (){
//				return  std::enable_if<true, __iterator_templ<const Type_t>>::type ( _n );
//			}
		private:
			__iterator_templ<ExposedType_t>(__Node* n): _n(n){}
			void __check_next() const { if (_n->_next==nullptr) throw std::runtime_error("End of sequence reached"); }
			void __check_prev() const { if (_n->_prev==nullptr) throw std::runtime_error("Begin of sequence reached"); }
			struct __Node* _n;
		};

		__Node 	_end;
		struct __Node *_head;
		struct __Node *_tail;

		template<class Iterator_Type_t>
		struct __Node *__get_Node(Iterator_Type_t& it){
			return it._n;
		}

		template<class Iterator_Type_t, class ValueType_t = typename Iterator_Type_t::reference>
		Iterator_Type_t __insert(Iterator_Type_t it,  ValueType_t val){

			struct __Node* pn = __get_Node(it);
			struct __Node* insert = new __Node(val);
			if(pn->_prev){
				pn->_prev->_next = insert;
			}
			else {
				_head = insert;
			}
			insert->_prev = pn->_prev;
			insert->_next = pn;
			pn->_prev = insert;
			return iterator(insert);
		}

		template <class TypeQ1, class TypeQ2>
		struct __HasCompatibleQualifiers{
			 constexpr static bool value = std::is_const<TypeQ1>::value == std::is_const<TypeQ2>::value
					 or std::is_const<TypeQ2>::value;
		};

		template<bool compatibleQualifiers, typename ConvertFrom_t, typename ConverTo_t>
		struct __Convert {
			__Convert(ConvertFrom_t from): _from(from) {}
			ConvertFrom_t _from;
		};

		template<typename ConvertFrom_t, typename ConverTo_t>
		struct __Convert<true, ConvertFrom_t, ConverTo_t> {
			__Convert(ConvertFrom_t from): _from(from) {}
			ConverTo_t operator ()(){
				return static_cast<ConverTo_t>(_from);
			}
			ConvertFrom_t _from;
		};

		template<typename ConvertFrom_t, typename ConverTo_t>
		struct __Convert<false, ConvertFrom_t, ConverTo_t> {
			__Convert(ConvertFrom_t from): _from(from) {}
			ConverTo_t operator ()(){
				return static_cast<ConverTo_t>(_from);
			}
			ConvertFrom_t _from;
		};

	public:
		typedef __iterator_templ<Type_t> iterator;
		typedef __iterator_templ<const Type_t> const_iterator;

		iterator begin(){
			return iterator(_head);
		}
		iterator end(){
			return iterator(_tail);
		}
		const_iterator begin() const{
			return const_iterator(_head);
		}
		const_iterator end() const{
			return const_iterator(_tail);
		}
		const_iterator cbegin() {
			return const_iterator(_head);
		}
		const_iterator cend() {
			return const_iterator(_tail);
		}

		template<typename QualifiedType_t>
		iterator insert(iterator it, QualifiedType_t t){
			return __insert(it, t);
		}

		template <class Iterator_t>
		iterator insert(iterator where, Iterator_t from, Iterator_t to){
			while(from != to)
				where = insert(where,*from++);
			return where;
		}

		iterator replace(iterator it, Type_t& t){
			struct __Node * pn = __get_Node(it);
			if(pn != &_end){
				it = erase(it);
				insert(it,t);
			}
			return it;
		}
		iterator replace(iterator it, Type_t&& t){
			return replace(it, t);
		}
		iterator erase(iterator it){
			struct __Node * pn = __get_Node(it);
			if(pn != &_end){
				iterator it_next(pn->_next);
				if(pn->_prev){
					pn->_prev->_next = pn->_next;
				}
				else {
					_head = pn->_next;
				}

				pn->_next->_prev = pn->_prev;
				delete pn;
				return it_next;
			}
			return it;
		}
		iterator erase(iterator it_from, iterator it_to){
			struct __Node * pn = __get_Node(it_from);
			if(pn != &_end){

				if(pn->_prev){
					pn->_prev->_next = __get_Node(it_to);
				}
				else {
					_head = __get_Node(it_to);
				}
				__get_Node(it_to)->_prev = pn->_prev;

				do{
					__Node* tmp = pn->_next;
					delete pn;
					pn = tmp;
				}while(pn != it_to._n);

				return iterator(it_to);
			}
			return it_from;
		}
		bool empty() const{
			return _head == &_end;
		}
		void clear(){
			erase(begin(), end());
		}
	};

}
#endif /* DOUBLE_CHAINED_LIST_HPP_ */
