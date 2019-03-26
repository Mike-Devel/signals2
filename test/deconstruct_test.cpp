// Tests for boost::signals2::deconstruct_ptr and friends

// Copyright Frank Mori Hess 2007-2008.
// Distributed under the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/signals2 for library home page.

#include <memory>
#include <boost/signals2/deconstruct.hpp>
#include <boost/signals2/deconstruct_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

class X: public boost::signals2::postconstructible {
public:
  X(): _postconstructed(false)
  {}
  ~X()
  {
    BOOST_TEST(_postconstructed);
  }
protected:
  virtual void postconstruct()
  {
    BOOST_TEST(!_postconstructed);
    _postconstructed = true;
  }
  bool _postconstructed;
};

class Y: public boost::signals2::predestructible {
public:
  Y(): _predestructed(false)
  {}
  ~Y()
  {
    BOOST_TEST(_predestructed);
  }
protected:
  virtual void predestruct()
  {
    _predestructed = true;
  }
  bool _predestructed;
};

class Z: public X, public Y
{};

class by_deconstruct_only: public boost::signals2::postconstructible {
public:
  ~by_deconstruct_only()
  {
    BOOST_TEST(_postconstructed);
  }
  int value;
protected:
  virtual void postconstruct()
  {
    BOOST_TEST(!_postconstructed);
    _postconstructed = true;
  }
  bool _postconstructed;
private:
  friend class boost::signals2::deconstruct_access;
  by_deconstruct_only(int value_in):
    value(value_in), _postconstructed(false)
  {}
};

namespace mytest
{
  class A
  {
  public:
    template<typename T> friend
      void adl_postconstruct(const std::shared_ptr<T> &, A *p)
    {
      BOOST_TEST(!p->_postconstructed);
      p->_postconstructed = true;
    }
    template<typename T> friend
      void adl_postconstruct(const std::shared_ptr<T> &, A *p, int val)
    {
      p->value = val;
      BOOST_TEST(!p->_postconstructed);
      p->_postconstructed = true;
    }
    friend void adl_predestruct(A *p)
    {
      p->_predestructed = true;
    }
    ~A()
    {
      BOOST_TEST(_postconstructed);
      BOOST_TEST(_predestructed);
    }
    int value;
  private:
    friend class boost::signals2::deconstruct_access;
    A(int value_in = 0):
      value(value_in),
      _postconstructed(false),
      _predestructed(false)
    {}
    bool _postconstructed;
    bool _predestructed;
  };
}

void deconstruct_ptr_test()
{
  {
    std::shared_ptr<X> x = boost::signals2::deconstruct_ptr(new X);
  }
  {
    std::shared_ptr<Y> x = boost::signals2::deconstruct_ptr(new Y);
  }
  {
    std::shared_ptr<Z> z = boost::signals2::deconstruct_ptr(new Z);
  }
}

class deconstructed_esft : public std::enable_shared_from_this<deconstructed_esft>
{
public:
  deconstructed_esft() : x(0) {}

private:
  friend void adl_postconstruct(std::shared_ptr<void>, deconstructed_esft *) {}
  int x;
};

void deconstruct_test()
{
  {
   std::shared_ptr<X> x = boost::signals2::deconstruct<X>();
  }
  {
   std::shared_ptr<Y> x = boost::signals2::deconstruct<Y>();
  }
  {
   std::shared_ptr<Z> z = boost::signals2::deconstruct<Z>();
  }
  {
   std::shared_ptr<by_deconstruct_only> a = boost::signals2::deconstruct<by_deconstruct_only>(1);
   BOOST_TEST(a->value == 1);
  }
  {
   std::shared_ptr<mytest::A> a = boost::signals2::deconstruct<mytest::A>(1);
   BOOST_TEST(a->value == 1);
  }
  {// deconstruct const type
   std::shared_ptr<const mytest::A> a = boost::signals2::deconstruct<const mytest::A>(3);
   BOOST_TEST(a->value == 3);
  }
  {// passing arguments to postconstructor
   std::shared_ptr<mytest::A> a = boost::signals2::deconstruct<mytest::A>().postconstruct(2);
   BOOST_TEST(a->value == 2);
  }
  {// enable_shared_from_this with deconstruct
      std::shared_ptr<deconstructed_esft> a = boost::signals2::deconstruct<deconstructed_esft>();
	  assert(a);
      BOOST_TEST(!(a->shared_from_this() < a || a < a->shared_from_this()));
  }
}

int main(int, char*[])
{
  deconstruct_ptr_test();
  deconstruct_test();
  return boost::report_errors();;
}
