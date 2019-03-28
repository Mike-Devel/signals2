// Boost.Signals2 library

// Copyright Douglas Gregor 2001-2004.
// Copyright Frank Mori Hess 2007-2008.
// Copyright Mike Dev 2019.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SIGNALS2_SLOT_CALL_ITERATOR_HPP
#define BOOST_SIGNALS2_SLOT_CALL_ITERATOR_HPP

#include <boost/signals2/connection.hpp>
#include <boost/signals2/slot_base.hpp>

#include <boost/signals2/detail/auto_buffer.hpp>

//#include <boost/optional.hpp>

#include <optional>
#include <cassert>
#include <type_traits>
#include <functional>

namespace boost {
  namespace signals2 {
    namespace detail {
      template<typename ResultType, typename Function>
        class slot_call_iterator_cache
      {
      public:
        slot_call_iterator_cache(const Function &f_arg):
          f(f_arg),
          connected_slot_count(0),
          disconnected_slot_count(0),
          m_active_slot(0)
        {}

        ~slot_call_iterator_cache()
        {
          if(m_active_slot)
          {
            garbage_collecting_lock<connection_body_base> lock(*m_active_slot);
            m_active_slot->dec_slot_refcount(lock);
          }
        }

        template<typename M>
        void set_active_slot(garbage_collecting_lock<M> &lock,
          connection_body_base *active_slot)
        {
          if(m_active_slot)
            m_active_slot->dec_slot_refcount(lock);
          m_active_slot = active_slot;
          if(m_active_slot)
            m_active_slot->inc_slot_refcount(lock);
        }

		using optional_result_t = std::conditional_t <
			std::is_reference_v<ResultType>,
			std::optional<
				std::reference_wrapper<
					std::remove_reference_t<ResultType>
				>
			>,
			std::optional<ResultType>
		>;
		optional_result_t result;
        typedef auto_buffer<void_shared_ptr_variant, store_n_objects<10> > tracked_ptrs_type;
        tracked_ptrs_type tracked_ptrs;
        Function f;
        unsigned connected_slot_count;
        unsigned disconnected_slot_count;
        connection_body_base *m_active_slot;
      };

      // Generates a slot call iterator. Essentially, this is an iterator that:
      //   - skips over disconnected slots in the underlying list
      //   - calls the connected slots when dereferenced
      //   - caches the result of calling the slots
      template<typename Function, typename Iterator, typename ConnectionBody>
      class slot_call_iterator_t
      {
        typedef typename Function::result_type result_type;

        typedef slot_call_iterator_cache<result_type, Function> cache_type;

	  public:
		  using value_type = std::decay_t<result_type>;
		  using difference_type = std::uintptr_t;
		  using reference = value_type & ;
		  using iterator_category = std::input_iterator_tag;

        slot_call_iterator_t(Iterator iter_in, Iterator end_in,
          cache_type &c):
          iter(iter_in), end(end_in),
          cache(&c), callable_iter(end_in)
        {
          lock_next_callable();
        }

		reference operator*() {
			if (!cache->result) {
				try
				{
				  cache->result.emplace(cache->f(*iter));
				}
				catch(expired_slot &)
				{
					(*iter)->disconnect();
					throw;
				}
			}
			return cache->result.value();
		}

		value_type* operator->() {
			if (!cache->result) {
				try
				{
				  cache->result.emplace(cache->f(*iter));
				}
					catch(expired_slot &)
				{
					(*iter)->disconnect();
					throw;
				}
			}
			return &cache->result.get();
		}

		slot_call_iterator_t& operator++() {
			++iter;
			lock_next_callable();
			cache->result.reset();
			return *this;
		}

		slot_call_iterator_t operator++(int) {
			slot_call_iterator_t copy = *this;
			++iter;
			lock_next_callable();
			cache->result.reset();
			return copy;
		}

		friend bool operator==(const slot_call_iterator_t& l, const slot_call_iterator_t& r) {
			return l.iter == r.iter;
		}
		friend bool operator!=(const slot_call_iterator_t& l, const slot_call_iterator_t& r) {
			return l.iter != r.iter;
		}

      private:
        typedef garbage_collecting_lock<connection_body_base> lock_type;

        void set_callable_iter(lock_type &lock, Iterator newValue) const
        {
          callable_iter = newValue;
          if(callable_iter == end)
            cache->set_active_slot(lock, 0);
          else
            cache->set_active_slot(lock, (*callable_iter).get());
        }

        void lock_next_callable() const
        {
          if(iter == callable_iter)
          {
            return;
          }
          if(iter == end)
          {
            if(callable_iter != end)
            {
              lock_type lock(**callable_iter);
              set_callable_iter(lock, end);
              return;
            }
          }
          // we're only locking the first connection body,
          // but it doesn't matter they all use the same mutex
          lock_type lock(**iter);
          for(;iter != end; ++iter)
          {
            cache->tracked_ptrs.clear();
            (*iter)->nolock_grab_tracked_objects(lock, std::back_inserter(cache->tracked_ptrs));
            if((*iter)->nolock_nograb_connected())
            {
              ++cache->connected_slot_count;
            }else
            {
              ++cache->disconnected_slot_count;
            }
            if((*iter)->nolock_nograb_blocked() == false)
            {
              set_callable_iter(lock, iter);
              break;
            }
          }
          if(iter == end)
          {
            set_callable_iter(lock, end);
          }
        }

        mutable Iterator iter;
        Iterator end;
        cache_type *cache;
        mutable Iterator callable_iter;
      };
    } // end namespace detail
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#endif // BOOST_SIGNALS2_SLOT_CALL_ITERATOR_HPP
