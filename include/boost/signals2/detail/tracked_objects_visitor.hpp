// Boost.Signals2 library

// Copyright Frank Mori Hess 2007-2008.
// Copyright Timmo Stange 2007.
// Copyright Douglas Gregor 2001-2004. 
// Copyright Mike Dev 2019.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SIGNALS2_TRACKED_OBJECTS_VISITOR_HPP
#define BOOST_SIGNALS2_TRACKED_OBJECTS_VISITOR_HPP

#include <boost/signals2/slot_base.hpp>
#include <boost/signals2/trackable.hpp>

#include <boost/signals2/detail/signals_common.hpp>

#include <functional>
#include <type_traits>

namespace boost
{
  namespace signals2
  {
    namespace detail
    {

      // Visitor to collect tracked objects from a bound function.
      class tracked_objects_visitor
      {
      public:
        tracked_objects_visitor(slot_base *slot) : slot_(slot)
        {}
        template<typename T>
        void operator()(const T& t) const
        {
            if constexpr (std::is_pointer<T>::value) {
                m_visit_pointer(t);
            } else {
                m_visit_pointer(&t);
            }
        }
      private:
        template<typename T>
        void m_visit_pointer(T*  t) const
        {
            if constexpr (!std::is_function<T>::value) {
                if (t) {
                    if constexpr (is_signal<T>::value) {
                        slot_->track_signal(*t);
                    }
                    else {
                        add_if_trackable(t);
                    }
                }
            }
            else {
                (void) t;
            }
        }

        void add_if_trackable(const trackable* trackable) const
        {
            slot_->_tracked_objects.push_back(trackable->get_weak_ptr());
        }
        void add_if_trackable(const void *) const {}

        slot_base * slot_;
      };


    } // end namespace detail
  } // end namespace signals2
} // end namespace boost

#endif // BOOST_SIGNALS2_TRACKED_OBJECTS_VISITOR_HPP

