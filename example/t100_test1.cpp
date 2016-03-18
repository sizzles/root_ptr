/**
	@file
	t100_test1.cpp

	@note
	Copyright (c) 2008 Phil Bouchard <phil@fornux.com>.

	Distributed under the Boost Software License, Version 1.0.

	See accompanying file LICENSE_1_0.txt or copy at
	http://www.boost.org/LICENSE_1_0.txt

	See http://www.boost.org/libs/smart_ptr/doc/index.html for documentation.
*/

#include <pstreams/pstream.h>
#include <set>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <boost/regex.hpp>
#include <boost/smart_ptr/root_ptr.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>

#include "t100.h"

using namespace std;
using namespace redi;
using namespace boost;
using boost::detail::sh::neuron_sight;

void foo()
{
    pstream page(string("lynx --dump http://www.unifiedfieldtheoryfinite.com/files/experiment.pdf | pdftotext - -"), pstreams::pstdout);
    stringstream formatted;
    
    for (string line; getline(page.out(), line, '.');)
    {
        boost::algorithm::trim_all(line);
        boost::algorithm::to_lower(line);
        boost::algorithm::replace_all(line, "\n", " ");

        formatted << line << '.' << endl;
    }
    
    list<string> sentence;
    for (string line; getline(formatted, line, '.');)
    {
        sentence.push_back(line);
    }
    
    set<string> mind;
    for (list<string>::iterator i = sentence.begin(); i != sentence.end(); ++ i)
    {
        for (list<string>::iterator j = sentence.begin(); j != sentence.end(); ++ j)
        {
            if (i == j)
                continue;
            
            pstream proc(string("bash -c \"wdiff <(echo '") + *i + string("' ) <(echo '") + *j + string("')\""), pstreams::pstdout);
            
            for (string line; getline(proc.out(), line);)
                mind.insert(line);
        }
        cout << distance(sentence.begin(), i) * 100 / distance(sentence.begin(), sentence.end()) << "%..." << endl;
    }
    
    for (set<string>::iterator i = mind.begin(); i != mind.end(); ++ i)
        cout << *i << endl;
    
    return 0;
}


int main(int argv, char * argc[])
{
    root_ptr<neuron_sight> t100;
    t100 = new node<neuron_sight>(t100, "I eat ([a-z]+) then drink ([a-z]+)");
    t100->sub_[0].second = new node<neuron_sight>(t100, "beef|chicken");
    t100->sub_[1].second = new node<neuron_sight>(t100, "vodka|water");

    cout << (* t100)("I eat beef then drink vodka") << endl;
    cout << (* t100)("I eat beef then drink wine") << endl;
    cout << (* t100)("I eat fish then drink wine") << endl;
    cout << (* t100)("I eat fish then drink beer") << endl;
}
