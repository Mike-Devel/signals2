/*
  Template for Signa1, Signal2, ... classes that support signals
  with 1, 2, ... parameters

  Begin: 2007-01-23
*/
// Copyright Frank Mori Hess 2007-2008
//
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is included iteratively, and should not be protected from multiple inclusion

#define BOOST_SIGNALS2_NUM_ARGS 1

// R, T1, T2, ..., TN, Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex
#define BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION \
  R (Args...), Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex

namespace boost
{
  namespace signals2
  {
    namespace detail
    {
      // helper for bound_extended_slot_function that handles specialization for void return
      template<typename R>
        class bound_extended_slot_function_invoker
      {
      public:
        typedef R result_type;
        template<typename ExtendedSlotFunction , 
          typename ... Args>
          result_type operator()(ExtendedSlotFunction &func, const connection &conn
            , 
            Args && ... args) const
        {
          return func(conn , 
            std::forward<Args>(args)...);
        }
      };

// wrapper around an signalN::extended_slot_function which binds the
// connection argument so it looks like a normal
// signalN::slot_function

      template<typename ExtendedSlotFunction>
        class bound_extended_slot_function
      {
      public:
        typedef typename result_type_wrapper<typename ExtendedSlotFunction::result_type>::type result_type;
		bound_extended_slot_function(const ExtendedSlotFunction &fun):
          _fun(fun), _connection(new connection)
        {}
        void set_connection(const connection &conn)
        {
          *_connection = conn;
        }

#if BOOST_SIGNALS2_NUM_ARGS > 0
        template<typename ... Args>
#endif // BOOST_SIGNALS2_NUM_ARGS > 0
          result_type operator()(Args && ... args)
        {
          return bound_extended_slot_function_invoker
            <typename ExtendedSlotFunction::result_type>()
            (_fun, *_connection , 
              std::forward<Args>(args)...);
        }
        // const overload
#if BOOST_SIGNALS2_NUM_ARGS > 0
        template<typename ... Args>
#endif // BOOST_SIGNALS2_NUM_ARGS > 0
          result_type operator()(Args && ... args) const
        {
          return bound_extended_slot_function_invoker
            <typename ExtendedSlotFunction::result_type>()
            (_fun, *_connection , 
              std::forward<Args>(args)...);
        }
        template<typename T>
          bool operator==(const T &other) const
        {
          return _fun == other;
        }
      private:
        bound_extended_slot_function()
        {}

        ExtendedSlotFunction _fun;
        std::shared_ptr<connection> _connection;
      };

      template<  typename Signature, typename Combiner, typename Group, typename GroupCompare,   typename SlotFunction,  typename ExtendedSlotFunction,   typename Mutex>
        class signal_impl;

      template<  typename Combiner, typename Group, typename GroupCompare, typename SlotFunction, typename ExtendedSlotFunction, typename Mutex, typename R, typename ... Args>
      class signal_impl <R (Args...), Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex>
      {
      public:
        typedef SlotFunction slot_function_type;
        // typedef slotN<Signature, SlotFunction> slot_type;
        typedef slot
          <R (Args...),
          slot_function_type> slot_type;
        typedef ExtendedSlotFunction extended_slot_function_type;
        // typedef slotN+1<R, const connection &, T1, T2, ..., TN, extended_slot_function_type> extended_slot_type;
        typedef slot<R (const connection &, Args...), extended_slot_function_type>extended_slot_type;
        typedef typename nonvoid<typename slot_function_type::result_type>::type nonvoid_slot_result_type;
      private:
        typedef variadic_slot_invoker<nonvoid_slot_result_type, Args...> slot_invoker;
        typedef slot_call_iterator_cache<nonvoid_slot_result_type, slot_invoker> slot_call_iterator_cache_type;
        typedef typename group_key<Group>::type group_key_type;
        typedef std::shared_ptr<connection_body<group_key_type, slot_type, Mutex> > connection_body_type;
        typedef grouped_list<Group, GroupCompare, connection_body_type> connection_list_type;
        typedef bound_extended_slot_function<extended_slot_function_type>
          bound_extended_slot_function_type;
      public:
        typedef Combiner combiner_type;
        typedef typename result_type_wrapper<typename combiner_type::result_type>::type result_type;
        typedef Group group_type;
        typedef GroupCompare group_compare_type;
        typedef typename detail::slot_call_iterator_t<slot_invoker,
          typename connection_list_type::iterator, connection_body<group_key_type, slot_type, Mutex> > slot_call_iterator;

        signal_impl(const combiner_type &combiner_arg,
          const group_compare_type &group_compare):
          _shared_state(new invocation_state(connection_list_type(group_compare), combiner_arg)),
          _garbage_collector_it(_shared_state->connection_bodies().end()),
          _mutex(new mutex_type())
        {}
        // connect slot
        connection connect(const slot_type &slot, connect_position position = at_back)
        {
          garbage_collecting_lock<mutex_type> lock(*_mutex);
          return nolock_connect(lock, slot, position);
        }
        connection connect(const group_type &group,
          const slot_type &slot, connect_position position = at_back)
        {
          garbage_collecting_lock<mutex_type> lock(*_mutex);
          return nolock_connect(lock, group, slot, position);
        }
        // connect extended slot
        connection connect_extended(const extended_slot_type &ext_slot, connect_position position = at_back)
        {
          garbage_collecting_lock<mutex_type> lock(*_mutex);
          bound_extended_slot_function_type bound_slot(ext_slot.slot_function());
          slot_type slot = replace_slot_function<slot_type>(ext_slot, bound_slot);
          connection conn = nolock_connect(lock, slot, position);
          bound_slot.set_connection(conn);
          return conn;
        }
        connection connect_extended(const group_type &group,
          const extended_slot_type &ext_slot, connect_position position = at_back)
        {
          garbage_collecting_lock<Mutex> lock(*_mutex);
          bound_extended_slot_function_type bound_slot(ext_slot.slot_function());
          slot_type slot = replace_slot_function<slot_type>(ext_slot, bound_slot);
          connection conn = nolock_connect(lock, group, slot, position);
          bound_slot.set_connection(conn);
          return conn;
        }
        // disconnect slot(s)
        void disconnect_all_slots()
        {
          std::shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          for(it = local_state->connection_bodies().begin();
            it != local_state->connection_bodies().end(); ++it)
          {
            (*it)->disconnect();
          }
        }
        void disconnect(const group_type &group)
        {
          std::shared_ptr<invocation_state> local_state =
            get_readable_state();
          group_key_type group_key(grouped_slots, group);
          typename connection_list_type::iterator it;
          typename connection_list_type::iterator end_it =
            local_state->connection_bodies().upper_bound(group_key);
          for(it = local_state->connection_bodies().lower_bound(group_key);
            it != end_it; ++it)
          {
            (*it)->disconnect();
          }
        }
        template <typename T>
        void disconnect(const T &slot)
        {
          typedef mpl::bool_<(is_convertible<T, group_type>::value)> is_group;
          do_disconnect(slot, is_group());
        }
        // emit signal
        result_type operator ()(Args ... args)
        {
          std::shared_ptr<invocation_state> local_state;
          typename connection_list_type::iterator it;
          {
            garbage_collecting_lock<mutex_type> list_lock(*_mutex);
            // only clean up if it is safe to do so
            if(_shared_state.unique())
              nolock_cleanup_connections(list_lock, false, 1);
            /* Make a local copy of _shared_state while holding mutex, so we are
            thread safe against the combiner or connection list getting modified
            during invocation. */
            local_state = _shared_state;
          }
          slot_invoker invoker = slot_invoker(args...);
          slot_call_iterator_cache_type cache(invoker);
          invocation_janitor janitor(cache, *this, &local_state->connection_bodies());
          return detail::combiner_invoker<typename combiner_type::result_type>()
            (
              local_state->combiner(),
              slot_call_iterator(local_state->connection_bodies().begin(), local_state->connection_bodies().end(), cache),
              slot_call_iterator(local_state->connection_bodies().end(), local_state->connection_bodies().end(), cache)
            );
        }
        result_type operator ()(Args ... args) const
        {
          std::shared_ptr<invocation_state> local_state;
          typename connection_list_type::iterator it;
          {
            garbage_collecting_lock<mutex_type> list_lock(*_mutex);
            // only clean up if it is safe to do so
            if(_shared_state.unique())
              nolock_cleanup_connections(list_lock, false, 1);
            /* Make a local copy of _shared_state while holding mutex, so we are
            thread safe against the combiner or connection list getting modified
            during invocation. */
            local_state = _shared_state;
          }
          slot_invoker invoker = slot_invoker(args...);
          slot_call_iterator_cache_type cache(invoker);
          invocation_janitor janitor(cache, *this, &local_state->connection_bodies());
          return detail::combiner_invoker<typename combiner_type::result_type>()
            (
              local_state->combiner(),
              slot_call_iterator(local_state->connection_bodies().begin(), local_state->connection_bodies().end(), cache),
              slot_call_iterator(local_state->connection_bodies().end(), local_state->connection_bodies().end(), cache)
            );
        }
        std::size_t num_slots() const
        {
          std::shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          std::size_t count = 0;
          for(it = local_state->connection_bodies().begin();
            it != local_state->connection_bodies().end(); ++it)
          {
            if((*it)->connected()) ++count;
          }
          return count;
        }
        bool empty() const
        {
          std::shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          for(it = local_state->connection_bodies().begin();
            it != local_state->connection_bodies().end(); ++it)
          {
            if((*it)->connected()) return false;
          }
          return true;
        }
        combiner_type combiner() const
        {
          unique_lock<mutex_type> lock(*_mutex);
          return _shared_state->combiner();
        }
        void set_combiner(const combiner_type &combiner_arg)
        {
          unique_lock<mutex_type> lock(*_mutex);
          if(_shared_state.unique())
            _shared_state->combiner() = combiner_arg;
          else
            _shared_state.reset(new invocation_state(*_shared_state, combiner_arg));
        }
      private:
        typedef Mutex mutex_type;

        // a struct used to optimize (minimize) the number of std::shared_ptrs that need to be created
        // inside operator()
        class invocation_state
        {
        public:
          invocation_state(const connection_list_type &connections_in,
            const combiner_type &combiner_in): _connection_bodies(new connection_list_type(connections_in)),
            _combiner(new combiner_type(combiner_in))
          {}
          invocation_state(const invocation_state &other, const connection_list_type &connections_in):
            _connection_bodies(new connection_list_type(connections_in)),
            _combiner(other._combiner)
          {}
          invocation_state(const invocation_state &other, const combiner_type &combiner_in):
            _connection_bodies(other._connection_bodies),
            _combiner(new combiner_type(combiner_in))
          {}
          connection_list_type & connection_bodies() { return *_connection_bodies; }
          const connection_list_type & connection_bodies() const { return *_connection_bodies; }
          combiner_type & combiner() { return *_combiner; }
          const combiner_type & combiner() const { return *_combiner; }
        private:
          invocation_state(const invocation_state &);

          std::shared_ptr<connection_list_type> _connection_bodies;
          std::shared_ptr<combiner_type> _combiner;
        };
        // Destructor of invocation_janitor does some cleanup when a signal invocation completes.
        // Code can't be put directly in signal's operator() due to complications from void return types.
        class invocation_janitor: noncopyable
        {
        public:
          typedef signal_impl signal_type;
          invocation_janitor
          (
            const slot_call_iterator_cache_type &cache,
            const signal_type &sig,
            const connection_list_type *connection_bodies
          ):_cache(cache), _sig(sig), _connection_bodies(connection_bodies)
          {}
          ~invocation_janitor()
          {
            // force a full cleanup of disconnected slots if there are too many
            if(_cache.disconnected_slot_count > _cache.connected_slot_count)
            {
              _sig.force_cleanup_connections(_connection_bodies);
            }
          }
        private:
          const slot_call_iterator_cache_type &_cache;
          const signal_type &_sig;
          const connection_list_type *_connection_bodies;
        };

        // clean up disconnected connections
        void nolock_cleanup_connections_from(garbage_collecting_lock<mutex_type> &lock,
          bool grab_tracked,
          const typename connection_list_type::iterator &begin, unsigned count = 0) const
        {
          BOOST_ASSERT(_shared_state.unique());
          typename connection_list_type::iterator it;
          unsigned i;
          for(it = begin, i = 0;
            it != _shared_state->connection_bodies().end() && (count == 0 || i < count);
            ++i)
          {
            bool connected;
            if(grab_tracked)
              (*it)->disconnect_expired_slot(lock);
            connected = (*it)->nolock_nograb_connected();
            if(connected == false)
            {
              it = _shared_state->connection_bodies().erase((*it)->group_key(), it);
            }else
            {
              ++it;
            }
          }
          _garbage_collector_it = it;
        }
        // clean up a few connections in constant time
        void nolock_cleanup_connections(garbage_collecting_lock<mutex_type> &lock,
          bool grab_tracked, unsigned count) const
        {
          BOOST_ASSERT(_shared_state.unique());
          typename connection_list_type::iterator begin;
          if(_garbage_collector_it == _shared_state->connection_bodies().end())
          {
            begin = _shared_state->connection_bodies().begin();
          }else
          {
            begin = _garbage_collector_it;
          }
          nolock_cleanup_connections_from(lock, grab_tracked, begin, count);
        }
        /* Make a new copy of the slot list if it is currently being read somewhere else
        */
        void nolock_force_unique_connection_list(garbage_collecting_lock<mutex_type> &lock)
        {
          if(_shared_state.unique() == false)
          {
            _shared_state.reset(new invocation_state(*_shared_state, _shared_state->connection_bodies()));
            nolock_cleanup_connections_from(lock, true, _shared_state->connection_bodies().begin());
          }else
          {
            /* We need to try and check more than just 1 connection here to avoid corner
            cases where certain repeated connect/disconnect patterns cause the slot
            list to grow without limit. */
            nolock_cleanup_connections(lock, true, 2);
          }
        }
        // force a full cleanup of the connection list
        void force_cleanup_connections(const connection_list_type *connection_bodies) const
        {
          garbage_collecting_lock<mutex_type> list_lock(*_mutex);
          // if the connection list passed in as a parameter is no longer in use,
          // we don't need to do any cleanup.
          if(&_shared_state->connection_bodies() != connection_bodies)
          {
            return;
          }
          if(_shared_state.unique() == false)
          {
            _shared_state.reset(new invocation_state(*_shared_state, _shared_state->connection_bodies()));
          }
          nolock_cleanup_connections_from(list_lock, false, _shared_state->connection_bodies().begin());
        }
        std::shared_ptr<invocation_state> get_readable_state() const
        {
          unique_lock<mutex_type> list_lock(*_mutex);
          return _shared_state;
        }
        connection_body_type create_new_connection(garbage_collecting_lock<mutex_type> &lock,
          const slot_type &slot)
        {
          nolock_force_unique_connection_list(lock);
          return connection_body_type(new connection_body<group_key_type, slot_type, Mutex>(slot, _mutex));
        }
        void do_disconnect(const group_type &group, mpl::bool_<true> /* is_group */)
        {
          disconnect(group);
        }
        template<typename T>
        void do_disconnect(const T &slot, mpl::bool_<false> /* is_group */)
        {
          std::shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          for(it = local_state->connection_bodies().begin();
            it != local_state->connection_bodies().end(); ++it)
          {
            garbage_collecting_lock<connection_body_base> lock(**it);
            if((*it)->nolock_nograb_connected() == false) continue;
            if((*it)->slot().slot_function() == slot)
            {
              (*it)->nolock_disconnect(lock);
            }else
            {
              // check for wrapped extended slot
              bound_extended_slot_function_type *fp;
              fp = (*it)->slot().slot_function().template target<bound_extended_slot_function_type>();
              if(fp && *fp == slot)
              {
                (*it)->nolock_disconnect(lock);
              }
            }
          }
        }
        // connect slot
        connection nolock_connect(garbage_collecting_lock<mutex_type> &lock,
          const slot_type &slot, connect_position position)
        {
          connection_body_type newConnectionBody =
            create_new_connection(lock, slot);
          group_key_type group_key;
          if(position == at_back)
          {
            group_key.first = back_ungrouped_slots;
            _shared_state->connection_bodies().push_back(group_key, newConnectionBody);
          }else
          {
            group_key.first = front_ungrouped_slots;
            _shared_state->connection_bodies().push_front(group_key, newConnectionBody);
          }
          newConnectionBody->set_group_key(group_key);
          return connection(newConnectionBody);
        }
        connection nolock_connect(garbage_collecting_lock<mutex_type> &lock,
          const group_type &group,
          const slot_type &slot, connect_position position)
        {
          connection_body_type newConnectionBody =
            create_new_connection(lock, slot);
          // update map to first connection body in group if needed
          group_key_type group_key(grouped_slots, group);
          newConnectionBody->set_group_key(group_key);
          if(position == at_back)
          {
            _shared_state->connection_bodies().push_back(group_key, newConnectionBody);
          }else  // at_front
          {
            _shared_state->connection_bodies().push_front(group_key, newConnectionBody);
          }
          return connection(newConnectionBody);
        }

        // _shared_state is mutable so we can do force_cleanup_connections during a const invocation
        mutable std::shared_ptr<invocation_state> _shared_state;
        mutable typename connection_list_type::iterator _garbage_collector_it;
        // connection list mutex must never be locked when attempting a blocking lock on a slot,
        // or you could deadlock.
        const std::shared_ptr<mutex_type> _mutex;
      };

      template<  typename Signature, typename Combiner, typename Group, typename GroupCompare,   typename SlotFunction,  typename ExtendedSlotFunction,   typename Mutex>
        class weak_signal;
    }

    template<typename Signature, typename Combiner = optional_last_value<typename boost::function_traits<Signature>::result_type>, typename Group = int, typename GroupCompare = std::less<Group>, typename SlotFunction = boost::function<Signature>, typename ExtendedSlotFunction = typename detail::variadic_extended_signature<Signature>::function_type, typename Mutex = signals2::mutex>
      class signal;

    template<  typename Combiner, typename Group, typename GroupCompare, typename SlotFunction, typename ExtendedSlotFunction, typename Mutex, typename R, typename ... Args>
    class signal
      <R (Args...), Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex>: public signal_base,
      public detail::std_functional_base<Args...>
    {
      typedef detail::signal_impl
        <R(Args...), Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex> impl_class;
    public:
      typedef detail::weak_signal
        <BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION> weak_signal_type;
      friend class detail::weak_signal
        <BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION>;

      typedef SlotFunction slot_function_type;
      // typedef slotN<Signature, SlotFunction> slot_type;
      typedef typename impl_class::slot_type slot_type;
      typedef typename impl_class::extended_slot_function_type extended_slot_function_type;
      typedef typename impl_class::extended_slot_type extended_slot_type;
      typedef typename slot_function_type::result_type slot_result_type;
      typedef Combiner combiner_type;
      typedef typename impl_class::result_type result_type;
      typedef Group group_type;
      typedef GroupCompare group_compare_type;
      typedef typename impl_class::slot_call_iterator
        slot_call_iterator;
      typedef typename mpl::identity<R (Args...)>::type signature_type;

      template<unsigned n> class arg
      {
      public:
        typedef typename detail::variadic_arg_type<n, Args...>::type type;
      };
      BOOST_STATIC_CONSTANT(int, arity = sizeof...(Args));

      signal(const combiner_type &combiner_arg = combiner_type(),
        const group_compare_type &group_compare = group_compare_type()):
        _pimpl(new impl_class(combiner_arg, group_compare))
      {}
      virtual ~signal()
      {
      }

      //move support
      signal(
        signal && other)
      {
        using std::swap;
        swap(_pimpl, other._pimpl);
      }

      signal &
        operator=(signal && rhs)
      {
        if(this == &rhs)
        {
          return *this;
        }
        _pimpl.reset();
        using std::swap;
        swap(_pimpl, rhs._pimpl);
        return *this;
      }

      connection connect(const slot_type &slot, connect_position position = at_back)
      {
        return (*_pimpl).connect(slot, position);
      }
      connection connect(const group_type &group,
        const slot_type &slot, connect_position position = at_back)
      {
        return (*_pimpl).connect(group, slot, position);
      }
      connection connect_extended(const extended_slot_type &slot, connect_position position = at_back)
      {
        return (*_pimpl).connect_extended(slot, position);
      }
      connection connect_extended(const group_type &group,
        const extended_slot_type &slot, connect_position position = at_back)
      {
        return (*_pimpl).connect_extended(group, slot, position);
      }
      void disconnect_all_slots()
      {
        (*_pimpl).disconnect_all_slots();
      }
      void disconnect(const group_type &group)
      {
        (*_pimpl).disconnect(group);
      }
      template <typename T>
      void disconnect(const T &slot)
      {
        (*_pimpl).disconnect(slot);
      }
      result_type operator ()(Args ... args)
      {
        return (*_pimpl)(args...);
      }
      result_type operator ()(Args ... args) const
      {
        return (*_pimpl)(args...);
      }
      std::size_t num_slots() const
      {
        return (*_pimpl).num_slots();
      }
      bool empty() const
      {
        return (*_pimpl).empty();
      }
      combiner_type combiner() const
      {
        return (*_pimpl).combiner();
      }
      void set_combiner(const combiner_type &combiner_arg)
      {
        return (*_pimpl).set_combiner(combiner_arg);
      }
      void swap(signal & other)
      {
        using std::swap;
        swap(_pimpl, other._pimpl);
      }
    protected:
      virtual std::shared_ptr<void> lock_pimpl() const
      {
        return _pimpl;
      }
    private:
      std::shared_ptr<impl_class>
        _pimpl;
    };

    namespace detail
    {
      // wrapper class for storing other signals as slots with automatic lifetime tracking
      template<typename Signature, typename Combiner, typename Group, typename GroupCompare, typename SlotFunction, typename ExtendedSlotFunction, typename Mutex>
        class weak_signal;

      template<typename Combiner, typename Group, typename GroupCompare, typename SlotFunction, typename ExtendedSlotFunction, typename Mutex, typename R, typename ... Args>
        class weak_signal
        <R (Args...), Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex>
      {
      public:
        typedef typename signal
          <BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION>::result_type
          result_type;

        weak_signal
          (const signal
          <BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION>
          &signal):
          _weak_pimpl(signal._pimpl)
        {}
        result_type operator ()(Args ... args)
        {
          std::shared_ptr<detail::signal_impl
            <BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION> >
            shared_pimpl(_weak_pimpl.lock());
          return (*shared_pimpl)(args...);
        }
        result_type operator ()(Args ... args) const
        {
          std::shared_ptr<detail::signal_impl
            <BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION> >
            shared_pimpl(_weak_pimpl.lock());
          return (*shared_pimpl)(args...);
        }
      private:
        std::weak_ptr<detail::signal_impl
          <BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION> > _weak_pimpl;
      };

      template<int arity, typename Signature>
        class extended_signature: public variadic_extended_signature<Signature>
      {};

    } // namespace detail
  } // namespace signals2
} // namespace boost

#undef BOOST_SIGNALS2_NUM_ARGS
#undef BOOST_SIGNALS2_SIGNAL_TEMPLATE_INSTANTIATION
