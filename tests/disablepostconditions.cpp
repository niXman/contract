
// Copyright Alexei Zakharov, 2013.
// Copyright niXman (i dot nixman dog gmail dot com) 2016.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define CONTRACT_DISABLE_POSTCONDITIONS
#include <contract/contract.hpp>

#include "contract_error.hpp"

#include <boost/test/unit_test.hpp>

namespace
{

void test_disable_postconditions()
{
    CONTRACT(fun) { POSTCONDITION(false); };
}

void test_disable_postconditions_precondition()
{
    CONTRACT(fun) { PRECONDITION(false); };
}

void test_disable_postconditions_invariant()
{
    CONTRACT(fun) { INVARIANT(false); };
}

}

BOOST_AUTO_TEST_CASE(macro_disable_postconditions)
{
    test::contract_handler_frame cframe;

    // expect disabled postcondition
    BOOST_CHECK_NO_THROW(test_disable_postconditions());

    // expect precondition to fail
    BOOST_CHECK_THROW(test_disable_postconditions_precondition(),
                      test::contract_error);

    // expect invariant to fail
    BOOST_CHECK_THROW(test_disable_postconditions_invariant(),
                      test::contract_error);
}
