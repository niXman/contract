#define CONTRACT_DISABLE_INVARIANTS
#include <contract/contract.hpp>

#include "contract_error.hpp"

#include <boost/test/unit_test.hpp>

namespace
{

void test_disable_invariants()
{
    CONTRACT(fun) { INVARIANT(false); };
}

void test_disable_invariants_precondition()
{
    CONTRACT(fun) { PRECONDITION(false); };
}

void test_disable_invariants_postcondition()
{
    CONTRACT(fun) { POSTCONDITION(false); };
}

}

BOOST_AUTO_TEST_CASE(macro_disable_invariants)
{
    test::contract_handler_frame cframe;

    // expect disabled invariant
    BOOST_CHECK_NO_THROW(test_disable_invariants());

    // expect precondition to fail
    BOOST_CHECK_THROW(test_disable_invariants_precondition(),
                      test::contract_error);

    // expect postcondition to fail
    BOOST_CHECK_THROW(test_disable_invariants_postcondition(),
                      test::contract_error);
}

// Copyright Alexei Zakharov, 2013.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
