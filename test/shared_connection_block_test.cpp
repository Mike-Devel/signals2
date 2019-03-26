// Boost.Signals2 library

// Copyright Douglas Gregor 2001-2003.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/core/lightweight_test.hpp>
#include <array>
#include <boost/signals2/shared_connection_block.hpp>
#include <boost/signals2/signal.hpp>
#include <iostream>
#include <sstream>
#include <string>

static std::array<boost::signals2::connection, 4> connections;

static std::ostringstream test_output;

struct test_slot {
  explicit test_slot(int v = 0) : value(v)
  {}

  void operator()() const {
    test_output << value;
  }

  int value;
};

int main(int, char* [])
{
  boost::signals2::signal<void ()> s0;

  for(unsigned i = 0; i < connections.size(); ++i)
  {
    connections.at(i) = s0.connect(test_slot(i));
  }


  {
    // Blocking 2
    boost::signals2::shared_connection_block block(connections.at(2));
    BOOST_TEST(block.blocking());
    test_output.str("");
    s0();
    BOOST_TEST(test_output.str() == "013");
  }

  // Unblocking 2
  test_output.str("");
  s0();
  BOOST_TEST(test_output.str() == "0123");

  {
    // Blocking 1 through const connection
    const boost::signals2::connection conn = connections.at(1);
    boost::signals2::shared_connection_block block(conn);
    test_output.str("");
    s0();
    std::cout << test_output.str() << std::endl;
    BOOST_TEST(test_output.str() == "023");
    // Unblocking 1
    block.unblock();
    BOOST_TEST(block.blocking() == false);
    test_output.str("");
    s0();
    BOOST_TEST(test_output.str() == "0123");
  }

  {
    // initially unblocked
    boost::signals2::shared_connection_block block(connections.at(3), false);
    BOOST_TEST(block.blocking() == false);
    test_output.str("");
    s0();
    BOOST_TEST(test_output.str() == "0123");
    // block
    block.block();
    test_output.str("");
    s0();
    BOOST_TEST(test_output.str() == "012");
  }

  {
    // test default constructed block
    boost::signals2::shared_connection_block block;
    BOOST_TEST(block.blocking() == true);
    block.unblock();
    BOOST_TEST(block.blocking() == false);
    block.block();
    BOOST_TEST(block.blocking() == true);

    // test assignment
    {
      block.unblock();
      boost::signals2::shared_connection_block block2(connections.at(0));
      BOOST_TEST(block.connection() != block2.connection());
      BOOST_TEST(block.blocking() != block2.blocking());
      block = block2;
      BOOST_TEST(block.connection() == block2.connection());
      BOOST_TEST(block.blocking() == block2.blocking());
    }
    test_output.str("");
    s0();
    BOOST_TEST(test_output.str() == "123");
  }
  return boost::report_errors();;
}
