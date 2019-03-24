/*
  Author: Frank Mori Hess <fmhess@users.sourceforge.net>
  Begin: 2007-01-23
*/
// Copyright Frank Mori Hess 2007-2008
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SIGNALS2_SIGNALS_COMMON_MACROS_HPP
#define BOOST_SIGNALS2_SIGNALS_COMMON_MACROS_HPP

#include <boost/config.hpp>

// #define BOOST_SIGNALS2_SIGNAL_CLASS_NAME(arity) signal
// #define BOOST_SIGNALS2_WEAK_SIGNAL_CLASS_NAME(arity) weak_signal
// #define BOOST_SIGNALS2_SIGNAL_IMPL_CLASS_NAME(arity) signal_impl
// #define BOOST_SIGNALS2_SIGNATURE_TEMPLATE_DECL(arity) typename Signature
// #define BOOST_SIGNALS2_ARGS_TEMPLATE_INSTANTIATION(arity) Args...
// #define BOOST_SIGNALS2_SIGNATURE_TEMPLATE_INSTANTIATION(arity) R (Args...)
// #define BOOST_SIGNALS2_SIGNATURE_FUNCTION_TYPE(arity) R (Args...)
// #define BOOST_SIGNALS2_ARGS_TEMPLATE_DECL(arity) typename ... Args
// #define BOOST_SIGNALS2_FULL_FORWARD_ARGS(arity) Args && ... args
// #define BOOST_SIGNALS2_FORWARDED_ARGS(arity) std::forward<Args>(args)...
// #define BOOST_SIGNALS2_SLOT_CLASS_NAME(arity) slot
// #define BOOST_SIGNALS2_EXTENDED_SLOT_TYPE(arity) slot<R (const connection &, Args...), extended_slot_function_type>
// #define BOOST_SIGNALS2_BOUND_EXTENDED_SLOT_FUNCTION_N(arity) bound_extended_slot_function
// #define BOOST_SIGNALS2_BOUND_EXTENDED_SLOT_FUNCTION_INVOKER_N(arity) bound_extended_slot_function_invoker
// #define BOOST_SIGNALS2_FUNCTION_N_DECL(arity) boost::function<Signature>
// #define BOOST_SIGNALS2_PREFIXED_SIGNATURE_TEMPLATE_DECL(arity, prefix) typename prefixSignature
// #define BOOST_SIGNALS2_PREFIXED_SIGNATURE_TEMPLATE_INSTANTIATION(arity, prefix) prefixSignature
// #define BOOST_SIGNALS2_SIGNATURE_FULL_ARGS(arity) Args ... args
// #define BOOST_SIGNALS2_SIGNATURE_ARG_NAMES(arity) args...
// #define BOOST_SIGNALS2_PORTABLE_SIGNATURE(arity, Signature) Signature

// #define BOOST_SIGNALS2_SLOT_TEMPLATE_SPECIALIZATION_DECL(arity) \
//   typename SlotFunction, \
//   typename R, \
//   typename ... Args
// #define BOOST_SIGNALS2_SLOT_TEMPLATE_SPECIALIZATION \
//   <R (Args...), SlotFunction>

// #define BOOST_SIGNALS2_SIGNAL_TEMPLATE_DECL(arity) \
//   typename Signature, \
//   typename Combiner, \
//   typename Group, \
//   typename GroupCompare, \
//   typename SlotFunction, \
//   typename ExtendedSlotFunction, \
//   typename Mutex
// #define BOOST_SIGNALS2_SIGNAL_TEMPLATE_DEFAULTED_DECL(arity) \
//   typename Signature, \
//   typename Combiner = optional_last_value<typename boost::function_traits<Signature>::result_type>, \
//   typename Group = int, \
//   typename GroupCompare = std::less<Group>, \
//   typename SlotFunction = boost::function<Signature>, \
//   typename ExtendedSlotFunction = typename detail::variadic_extended_signature<Signature>::function_type, \
//   typename Mutex = signals2::mutex
// #define BOOST_SIGNALS2_SIGNAL_TEMPLATE_SPECIALIZATION_DECL(arity) \
//   typename Combiner, \
//   typename Group, \
//   typename GroupCompare, \
//   typename SlotFunction, \
//   typename ExtendedSlotFunction, \
//   typename Mutex, \
//   typename R, \
//   typename ... Args
// #define BOOST_SIGNALS2_SIGNAL_TEMPLATE_SPECIALIZATION <\
//   R (Args...), \
//   Combiner, \
//   Group, \
//   GroupCompare, \
//   SlotFunction, \
//   ExtendedSlotFunction, \
//   Mutex>

// #define BOOST_SIGNALS2_STD_FUNCTIONAL_BASE \
//   std_functional_base<Args...>

// #define BOOST_SIGNALS2_PP_COMMA_IF(arity) ,



#endif // BOOST_SIGNALS2_SIGNALS_COMMON_MACROS_HPP
