// Boost.Signals2 library

// Copyright Frank Mori Hess 2009.
// Copyright Mike Dev 2019.
// 
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SIGNALS2_SLOT_HPP
#define BOOST_SIGNALS2_SLOT_HPP

#include <boost/signals2/slot_base.hpp>

#include <boost/signals2/detail/variadic_arg_type.hpp>
#include <boost/signals2/detail/signals_common.hpp>
#include <boost/signals2/detail/tracked_objects_visitor.hpp>

#include <functional>
#include <cstddef>

namespace boost {
namespace signals2 {
namespace detail {

//// forwading to boost core visit_each for boost types (via adl) if possible
template<typename Visitor, typename T>
inline auto internal_visit_each_impl(Visitor& visitor, const T& t, std::nullptr_t)-> decltype(visit_each(visitor, t))
{
	visit_each(visitor, t);
}

template<typename Visitor, typename T>
inline auto internal_visit_each_impl(Visitor& visitor, const T& t, int*)
{
	visitor(t);
}

template<typename Visitor, typename T>
inline void internal_visit_each(Visitor& visitor, const T& t)
{
	internal_visit_each_impl(visitor, t, nullptr);
}
}

template<typename Signature, typename SlotFunction = std::function<Signature> >
class slot;

template< typename SlotFunction,
	typename R,
	typename ... Args>
	class slot  <R(Args...), SlotFunction>
	: public slot_base, public detail::std_functional_base<Args...>

{
public:
	template<typename prefixSignature, typename OtherSlotFunction>
	friend class slot;

	typedef SlotFunction slot_function_type;
	typedef R result_type;
	//typedef typename mpl::identity<R(Args...)>::type signature_type;
	using signature_type = R(Args...);

	template<unsigned n> class arg {
	public:
		typedef typename detail::variadic_arg_type<n, Args...>::type type;
	};
	static constexpr int arity = sizeof...(Args);

	template<typename F>
	slot(const F& f)
	{
		init_slot_function(f);
	}
	// copy constructors

	template<typename Signature, typename OtherSlotFunction>
	slot(const slot<Signature, OtherSlotFunction>& other_slot) :
		slot_base(other_slot), _slot_function(other_slot._slot_function)
	{
	}
	// bind syntactic sugar
	template<typename A1, typename A2, typename ... BindArgs>
	slot(const A1& arg1, const A2& arg2, const BindArgs& ... args)
	{
		init_slot_function(std::bind(arg1, arg2, args...),arg1,arg2,args...);
	}
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
	slot& track(const std::weak_ptr<void>& tracked) {
		_tracked_objects.push_back(tracked);
		return *this;
	}
	slot& track(const signal_base& signal)
	{
		track_signal(signal);
		return *this;
	}
	slot& track(const slot_base& slot)
	{
		tracked_container_type::const_iterator it;
		for (it = slot.tracked_objects().begin(); it != slot.tracked_objects().end(); ++it) {
			_tracked_objects.push_back(*it);
		}
		return *this;
	}
	template<typename ForeignWeakPtr>
	slot& track_foreign(const ForeignWeakPtr& tracked,
						typename weak_ptr_traits<ForeignWeakPtr>::shared_type* /*SFINAE*/ = 0)
	{
		_tracked_objects.push_back(detail::foreign_void_weak_ptr(tracked));
		return *this;
	}
	template<typename ForeignSharedPtr>
	slot& track_foreign(const ForeignSharedPtr& tracked,
						typename shared_ptr_traits<ForeignSharedPtr>::weak_type* /*SFINAE*/ = 0)
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

	const slot_function_type& slot_function() const { return _slot_function; }
	slot_function_type& slot_function() { return _slot_function; }
private:
	template<typename F, class ...ARGS>
	void init_slot_function(const F& f, const ARGS& ... args)
	{
		if constexpr (detail::is_signal<F>::value) {
			_slot_function = typename F::weak_signal_type(f);
		} else {
			_slot_function = f;
		}
		signals2::detail::tracked_objects_visitor visitor(this);
		signals2::detail::internal_visit_each(visitor, f);
		(signals2::detail::internal_visit_each(visitor, args),...);
	}

	SlotFunction _slot_function;
};

} // end namespace signals2
} // end namespace boost


#endif // BOOST_SIGNALS2_SLOT_HPP
