/**
	@file
	block_ptr_test3.cpp

	@note
	Copyright (c) 2008 Steven Watanabe <watanabesj@gmail.com>

	Distributed under the Boost Software License, Version 1.0.

	See accompanying file LICENSE_1_0.txt or copy at
	http://www.boost.org/LICENSE_1_0.txt

	See http://www.boost.org/libs/smart_ptr/doc/index.html for documentation.
*/



#include <boost/smart_ptr/block_ptr.hpp>

#include <vector>
#include <iostream>

#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/array.hpp>
#include <boost/container/vector.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

static int count;

using namespace boost;

struct node {
    node() {
        ++count;
    }
    ~node() {
        --count;
    }
    node(const node&) {
        ++count;
    }
    block_ptr<node> prior;
    block_ptr<node> next;
};

struct list {
public:
    list() {}
    void clear() {
        root.reset();
    }
    void insert() {
        if(root.get() == 0) {
            root = block_ptr<node>(new block<node>());
        } else {
            root->next = block_ptr<node>(root.proxy(), new block<node>());
            root->next->prior = root;
            root = root->next;
        }
    }
    ~list()
    {
    }
private:
    block_ptr<node> root;
};


struct vector {
    vector() { ++count; }
    ~vector() { --count; }
    vector(const vector& other) : elements(other.elements) { ++count; }
    boost::container::vector<block_ptr<vector> > elements;
};

struct create_type {
    template<class T>
    void operator()(T) const {
        make_block<boost::array<char, T::value> >();
    }
};

BOOST_AUTO_TEST_CASE(test_block_ptr) {

    count = 0;
    {
        block_ptr<vector> v = make_block<vector>();
        v->elements.push_back(v);
    }
    BOOST_CHECK_EQUAL(count, 0);

    count = 0;
    {
        list l;
        for(int j = 0; j < 2; ++j) {
            for(int i = 0; i < 2; ++i) {
                l.insert();
            }
            l.clear();
        }
    }
    BOOST_CHECK_EQUAL(count, 0);

    count = 0;
    {
        block_ptr<int> test = make_block<int>(5);
        test = test;
        
        BOOST_CHECK_NE(test.get(), static_cast<int*>(0));
        BOOST_CHECK_EQUAL(*test, 5);
    }
    
    for(int i = 0; i < 500; ++i) {
        boost::mpl::for_each<boost::mpl::range_c<int, 1, 100> >(create_type());
    }

    count = 0;
    {
        block_ptr<vector> v = make_block<vector>();
        v->elements.push_back(v);
    }
    BOOST_CHECK_EQUAL(count, 0);

    {
        vector v;
        v.elements.push_back(make_block<vector>());
    }
    BOOST_CHECK_EQUAL(count, 0);

}
