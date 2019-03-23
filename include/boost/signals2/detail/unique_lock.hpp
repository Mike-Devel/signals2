/*
  Provides a basic subset of boost::unique_lock functionality.  Provided only because
  including boost/thread/locks.hpp requires linking to thread library
*/
// Copyright Frank Mori Hess 2008.
// Distributed under the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/signals2 for library home page.

#ifndef BOOST_SIGNALS2_UNIQUE_LOCK_HPP
#define BOOST_SIGNALS2_UNIQUE_LOCK_HPP

#include <mutex>
namespace boost
{
  namespace signals2
  {
    namespace detail
    {

		template<typename Mutex>
		struct unique_lock : std::unique_lock<Mutex>
		{
			using std::unique_lock<Mutex>::unique_lock;
		};

    } // namespace detail
  } // namespace signals2
} // namespace boost

#endif  // BOOST_SIGNALS2_UNIQUE_LOCK_HPP
