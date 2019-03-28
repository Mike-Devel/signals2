/*
  An output iterator which simply discards output.
*/
// Copyright Frank Mori Hess 2008.
// (C) Copyright Andrey Semashev 2017.
// (C) Copyright Mike Dev 2019.
// Distributed under the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/signals2 for library home page.

#ifndef BOOST_SIGNALS2_NULL_OUTPUT_ITERATOR_HPP
#define BOOST_SIGNALS2_NULL_OUTPUT_ITERATOR_HPP

#include <iterator>

namespace boost
{
  namespace signals2
  {
    namespace detail
    {
      class does_nothing
      {
      public:
        template<typename T>
          void operator()(const T&) const
          {}
      };

	  // internalized from boost/function_output_iterator.hpp
	  template <class UnaryFunction>
	  class function_output_iterator {
		  typedef function_output_iterator self;
	  public:
		  typedef std::output_iterator_tag iterator_category;
		  typedef void                value_type;
		  typedef void                difference_type;
		  typedef void                pointer;
		  typedef void                reference;

		  explicit function_output_iterator() {}

		  explicit function_output_iterator(const UnaryFunction& f)
			  : m_f(f) {}

		  struct output_proxy {
			  output_proxy(UnaryFunction& f) : m_f(f) { }
			  template <class T> output_proxy& operator=(const T& value) {
				  m_f(value);
				  return *this;
			  }
			  UnaryFunction& m_f;
		  };
		  output_proxy operator*() { return output_proxy(m_f); }
		  self& operator++() { return *this; }
		  self& operator++(int) { return *this; }
	  private:
		  UnaryFunction m_f;
	  };

	  template <class UnaryFunction>
	  inline function_output_iterator<UnaryFunction>
		  make_function_output_iterator(const UnaryFunction& f = UnaryFunction()) {
		  return function_output_iterator<UnaryFunction>(f);
	  }

      typedef function_output_iterator<does_nothing> null_output_iterator;
    } // namespace detail
  } // namespace signals2
} // namespace boost

#endif  // BOOST_SIGNALS2_NULL_OUTPUT_ITERATOR_HPP
