#pragma once

#ifndef __OBSERVABLE_FIELD__
#define __OBSERVABLE_FIELD__

template<class Type_t>
class Copyable {
public:
   Copyable(Type_t& t): _t(t){}
   void operator = (const Type_t& t) { if (&t != &_t) _t = t; }
private:
   Type_t& _t;
};

template<class Type_t,size_t SIZE>
class Copyable<Type_t[SIZE]> {
public:
   Copyable(Type_t t[SIZE]): _t(t){}
   void operator = (const Type_t (&t)[SIZE]) { if (t != _t)
       memcpy(_t ,t, sizeof(Type_t) * SIZE);
   }
private:
   Type_t* _t;
};

template<class Type_t>
class Comparable {
public:
   Comparable(Type_t& t): _t(t){}
   bool operator == (const Type_t& t) const { return t == _t; }
   bool operator != (const Type_t& t) const { return t != _t; }
private:
   Type_t& _t;
};

template<class Type_t, size_t SIZE>
class Comparable<Type_t[SIZE]> {
public:
   Comparable(Type_t (&t)[SIZE]): _t(t){}
   bool operator == (const Type_t (&t)[SIZE]) const { return memcmp(t,_t, sizeof(t)) == 0; }
   bool operator != (const Type_t (&t)[SIZE]) const { return memcmp(t,_t, sizeof(t)) != 0; }
private:
   Type_t* _t;
};

template <class Type_t>
class ObservableField {
public:
   ObservableField(){}
   ObservableField(const Type_t& t): _t(t){}
   const Type_t& operator = (const Type_t& t){
       Comparable<Type_t> comparable(_t);
       if (comparable != t){
           Copyable<Type_t> copyable(_t);
           copyable = t;
           if (_obs){
               _obs( reinterpret_cast<uintptr_t>(this) );
           }
       }
       return _t;
   }
   const Type_t& value() const { return _t; }
   operator Type_t& () const { return _t; }

   void setObserver(std::function<void(uintptr_t)>& obs){
       _obs = obs;
   }

private:
   Type_t _t;
   std::function<void(uintptr_t)> _obs;
};

#endif