// Boost.Signals2 library

// Copyright Frank Mori Hess 2007-2008.
// Copyright Timmo Stange 2007.
// Copyright Douglas Gregor 2001-2004. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

// This file is included iteratively, and should not be protected from multiple inclusion

#define BOOST_SIGNALS2_NUM_ARGS 1


namespace boost
{
  namespace signals2
  {
    template<typename Signature, typename SlotFunction = boost::function<Signature> >
      class slot;

#if BOOST_WORKAROUND(BOOST_MSVC, <= 1900)
    template<typename Signature, typename SlotFunction> class slot{};
#endif


    template< typename SlotFunction, 
              typename R, 
              typename ... Args>
      class slot  <R (Args...), SlotFunction>
      : public slot_base, public detail::std_functional_base<Args...>

    {
    public:
      template<typename prefixSignature, typename OtherSlotFunction>
      friend class slot;

      typedef SlotFunction slot_function_type;
      typedef R result_type;
      typedef typename mpl::identity<R (Args...)>::type signature_type;

      template<unsigned n> class arg
      {
      public:
        typedef typename detail::variadic_arg_type<n, Args...>::type type;
      };
      BOOST_STATIC_CONSTANT(int, arity = sizeof...(Args));



      template<typename F>
      slot(const F& f)
      {
        init_slot_function(f);
      }
      // copy constructors

      template<typename Signature, typename OtherSlotFunction>
      slot(const slot<Signature, OtherSlotFunction> &other_slot):
        slot_base(other_slot), _slot_function(other_slot._slot_function)
      {
      }
      // bind syntactic sugar
      BOOST_SIGNALS2_SLOT_N_BINDING_CONSTRUCTORS
      // invocation
      R operator()(Args ... args)
      {
        locked_container_type locked_objects = lock();
        return _slot_function(args...);
      }
      R operator()(Args ... args) const
      {
        locked_container_type locked_objects = lock();
        return _slot_function(args...);
      }
      // tracking
      slot& track(const std::weak_ptr<void> &tracked)      {
        _tracked_objects.push_back(tracked);
        return *this;
      }
      slot& track(const signal_base &signal)
      {
        track_signal(signal);
        return *this;
      }
      slot& track(const slot_base &slot)
      {
        tracked_container_type::const_iterator it;
        for(it = slot.tracked_objects().begin(); it != slot.tracked_objects().end(); ++it)
        {
          _tracked_objects.push_back(*it);
        }
        return *this;
      }
      template<typename ForeignWeakPtr>
      slot& track_foreign(const ForeignWeakPtr &tracked,
        typename weak_ptr_traits<ForeignWeakPtr>::shared_type * /*SFINAE*/ = 0)
      {
        _tracked_objects.push_back(detail::foreign_void_weak_ptr(tracked));
        return *this;
      }
      template<typename ForeignSharedPtr>
      slot& track_foreign(const ForeignSharedPtr &tracked,
        typename shared_ptr_traits<ForeignSharedPtr>::weak_type * /*SFINAE*/ = 0)
      {
        _tracked_objects.push_back
        (
          detail::foreign_void_weak_ptr
          (
            typename shared_ptr_traits<ForeignSharedPtr>::weak_type(tracked)
          )
        );
        return *this;
      }

      const slot_function_type& slot_function() const {return _slot_function;}
      slot_function_type& slot_function() {return _slot_function;}
    private:
      template<typename F>
      void init_slot_function(const F& f)
      {
        _slot_function = detail::get_invocable_slot(f, detail::tag_type(f));
        signals2::detail::tracked_objects_visitor visitor(this);
        boost::visit_each(visitor, f);
      }

      SlotFunction _slot_function;
    };

  } // end namespace signals2
} // end namespace boost

#undef BOOST_SIGNALS2_NUM_ARGS
