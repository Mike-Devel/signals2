// Signals2 library
// tests for connection class

// Copyright Frank Mori Hess 2008
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/core/lightweight_test.hpp>
#include <boost/signals2.hpp>

namespace bs2 = boost::signals2;

typedef bs2::signal<void ()> sig_type;

void myslot()
{}

void swap_test()
{
  sig_type sig;

  {
    bs2::connection conn1 = sig.connect(&myslot);
    BOOST_TEST(conn1.connected());
    bs2::connection conn2;
    BOOST_TEST(conn2.connected() == false);

    conn1.swap(conn2);
    BOOST_TEST(conn2.connected());
    BOOST_TEST(conn1.connected() == false);

    swap(conn1, conn2);
    BOOST_TEST(conn1.connected());
    BOOST_TEST(conn2.connected() == false);
  }

  {
    bs2::scoped_connection conn1;
    conn1 = sig.connect(&myslot);
    BOOST_TEST(conn1.connected());
    bs2::scoped_connection conn2;
    BOOST_TEST(conn2.connected() == false);

    conn1.swap(conn2);
    BOOST_TEST(conn2.connected());
    BOOST_TEST(conn1.connected() == false);

    swap(conn1, conn2);
    BOOST_TEST(conn1.connected());
    BOOST_TEST(conn2.connected() == false);
  }
}

void release_test()
{
  sig_type sig;
  bs2::connection conn;
  {
    bs2::scoped_connection scoped(sig.connect(&myslot));
    BOOST_TEST(scoped.connected());
    conn = scoped.release();
  }
  BOOST_TEST(conn.connected());

  bs2::connection conn2;
  {
    bs2::scoped_connection scoped(conn);
    BOOST_TEST(scoped.connected());
    conn = scoped.release();
    BOOST_TEST(conn.connected());
    BOOST_TEST(scoped.connected() == false);
    conn.disconnect();

    // earlier release shouldn't affect new connection
    conn2 = sig.connect(&myslot);
    scoped = conn2;
  }
  BOOST_TEST(conn2.connected() == false);
}

void move_test()
{
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
  sig_type sig;
  bs2::connection conn;
  // test move assignment from scoped_connection to connection
  {
    bs2::scoped_connection scoped(sig.connect(&myslot));
    BOOST_TEST(scoped.connected());
    conn = std::move(scoped);
    BOOST_TEST(scoped.connected() == false);
  }
  BOOST_TEST(conn.connected());

  // test move construction from scoped to scoped
  {
    bs2::scoped_connection scoped2(conn);
    BOOST_TEST(scoped2.connected());
    bs2::scoped_connection scoped3(std::move(scoped2));
    BOOST_TEST(scoped2.connected() == false);
    BOOST_TEST(scoped3.connected() == true);
    BOOST_TEST(conn.connected() == true);
  }
  BOOST_TEST(conn.connected() == false);

  // test move assignment from scoped to scoped
  conn = sig.connect(&myslot);
  {
    bs2::scoped_connection scoped3;
    bs2::scoped_connection scoped2(conn);
    BOOST_TEST(scoped2.connected());
    scoped3 = std::move(scoped2);
    BOOST_TEST(scoped2.connected() == false);
    BOOST_TEST(scoped3.connected() == true);
    BOOST_TEST(conn.connected() == true);
  }
  BOOST_TEST(conn.connected() == false);
#endif // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
}

int main(int, char*[])
{
  release_test();
  swap_test();
  move_test();
  return boost::report_errors();
}
