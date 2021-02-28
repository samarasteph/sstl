#include <gtest/gtest.h>
#include <iterator>
#include <algorithm>
#include <list>

#include "double_chained_list.hpp"

typedef sstl::double_chained_list<int> listint_t;
typedef sstl::double_chained_list<const int> list_const_int_t;

constexpr unsigned int NB_VALUES = 5;
int values[NB_VALUES] = {1, 2, 3, 4, 5};

TEST(DoubleChainedList, InsertEndRead){
	listint_t lst;

	for(int val: values){
		lst.insert(std::end(lst), val);
	}

	listint_t::const_iterator cit = lst.cbegin();
	ASSERT_EQ(*cit++, values[0]);
	ASSERT_EQ(*cit++, values[1]);
	ASSERT_EQ(*cit++, values[2]);
	ASSERT_EQ(*cit++, values[3]);
	ASSERT_EQ(*cit++, values[4]);
}

TEST(DoubleChainedList, InsertBeginRead){
	listint_t lst;

	for(int val: values){
		lst.insert(std::begin(lst), val);
	}

	listint_t::const_iterator cit = std::begin(const_cast<const listint_t&>(lst));
	ASSERT_EQ(*cit++, values[4]);
	ASSERT_EQ(*cit++, values[3]);
	ASSERT_EQ(*cit++, values[2]);
	ASSERT_EQ(*cit++, values[1]);
	ASSERT_EQ(*cit++, values[0]);
}

TEST(DoubleChainedList, InsertErase){
	listint_t lst;

	for(int val: values){
		lst.insert(std::end(lst), val);
	}

	listint_t::iterator it = std::begin(lst);

	ASSERT_EQ(*it,values[0]);

	it = lst.erase(it);

	ASSERT_EQ(it,std::begin(lst));
	ASSERT_EQ(*it,values[1]);

	++it;
	ASSERT_EQ(*it,values[2]);

	it = lst.insert(it,6);
	ASSERT_EQ(*it,6);
	ASSERT_EQ(*++it,values[2]);
}

TEST(DoubleChainedList, EraseRange){
	listint_t lst;

	for(int val: values){
		lst.insert(std::end(lst), val);
	}

	listint_t::iterator it = std::begin(lst);

	it++;
	lst.erase(it, std::end(lst));

	ASSERT_EQ(*std::begin(lst), values[0]);

	it = std::begin(lst);
	ASSERT_EQ(++it, std::end(lst));
}

TEST(DoubleChainedList, Throw){

	listint_t l;
	listint_t::iterator it = l.begin();

	EXPECT_THROW(++it, std::runtime_error);
	EXPECT_THROW(it++, std::runtime_error);

	EXPECT_THROW(--it, std::runtime_error);
	EXPECT_THROW(it--, std::runtime_error);

	listint_t::const_iterator cit = l.cbegin();

	EXPECT_THROW(++cit, std::runtime_error);
	EXPECT_THROW(cit++, std::runtime_error);

	EXPECT_THROW(--cit, std::runtime_error);
	EXPECT_THROW(cit--, std::runtime_error);

}

TEST(DoubleChainedList, CopyConstructor){
	listint_t l;
	listint_t::iterator it = l.end();
	std::copy(values, values+NB_VALUES, std::inserter(l,it));

	listint_t::const_iterator cit = l.cbegin();
	ASSERT_EQ(*cit++,values[0]);
	ASSERT_EQ(*cit++,values[1]);
	ASSERT_EQ(*cit++,values[2]);

    listint_t l2(l);
	cit = l2.cbegin();
	ASSERT_EQ(*cit++,values[0]);
	ASSERT_EQ(*cit++,values[1]);
	ASSERT_EQ(*cit++,values[2]);
}

TEST(DoubleChainedList, ConstTypeTemplate){

	list_const_int_t cl;

	std::list<int> l = {5, 6, 7, 8, 9, 10};
	std::copy(std::begin(l), std::end(l), std::inserter(cl, std::begin(cl)));

	list_const_int_t::const_iterator cit = cl.cbegin();

	EXPECT_EQ(*cit++, 5);
	EXPECT_EQ(*cit++, 6);
	EXPECT_EQ(*cit++, 7);
	EXPECT_EQ(*cit++, 8);
	EXPECT_EQ(*cit++, 9);
	EXPECT_EQ(*cit++, 10);

	EXPECT_EQ(cit, cl.cend());

	const list_const_int_t& ccl = cl;
	cit = cl.begin();

	EXPECT_EQ(*cit++, 5);
	EXPECT_EQ(*cit++, 6);
	EXPECT_EQ(*cit++, 7);
	EXPECT_EQ(*cit++, 8);
	EXPECT_EQ(*cit++, 9);
	EXPECT_EQ(*cit++, 10);
}

TEST(DoubleChainedList, Replace){

	std::list<int> l = {5, 6, 7, 8, 9, 10};
	listint_t li;

	std::copy(std::begin(l), std::end(l), std::inserter(li, std::begin(li)));

	listint_t::iterator it = std::begin(li);
	EXPECT_EQ(*it,5);
	li.replace(it, 10);

	it = std::begin(li);
	EXPECT_EQ(*it++,10);
	EXPECT_EQ(*it,6);
	li.replace(it, 11);

	EXPECT_EQ(std::find(std::begin(li), std::end(li), 5), std::end(li));
	EXPECT_EQ(std::find(std::begin(li), std::end(li), 6), std::end(li));
	EXPECT_NE(std::find(std::begin(li), std::end(li), 7), std::end(li));
}

TEST(DoubleChainedList, IteratorBackForth){

	std::list<int> l = {5, 6, 7, 8, 9, 10};
	listint_t li;

	std::copy(std::begin(l), std::end(l), std::inserter(li, std::begin(li)));

	listint_t::iterator it = std::begin(li);
	EXPECT_EQ(*it++,5);
	EXPECT_EQ(*it++,6);
	EXPECT_EQ(*it++,7);
	EXPECT_EQ(*it++,8);
	EXPECT_EQ(*it++,9);
	EXPECT_EQ(*it++,10);
	EXPECT_EQ(it,std::end(li));
	EXPECT_EQ(*--it,10);
	EXPECT_EQ(*--it,9);
	EXPECT_EQ(*--it,8);
	EXPECT_EQ(*--it,7);
	EXPECT_EQ(*--it,6);
	EXPECT_EQ(*--it,5);
	EXPECT_EQ(*++it,6);
	EXPECT_EQ(*++it,7);
	EXPECT_EQ(*++it,8);
	EXPECT_EQ(*++it,9);
	EXPECT_EQ(*++it,10);
	EXPECT_EQ(*it--,10);
	EXPECT_EQ(*it--,9);
	EXPECT_EQ(*it--,8);
	EXPECT_EQ(*it--,7);
	EXPECT_EQ(*it--,6);
	EXPECT_EQ(*it,5);
}
