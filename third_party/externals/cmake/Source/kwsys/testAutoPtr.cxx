/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifdef __BORLANDC__
# pragma warn -8027 /* 'for' not inlined.  */
# pragma warn -8026 /* exception not inlined.  */
#endif 
#include "kwsysPrivate.h"
#include KWSYS_HEADER(auto_ptr.hxx)
// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "auto_ptr.hxx.in"
#endif

#include <stdio.h>

#define ASSERT(x,y) if (!(x)) { printf("FAIL: " y "\n"); status = 1; }

int instances = 0; // don't declare as static

struct A
{
  A() { ++instances; }
  ~A() { --instances; }
  A* self() {return this; }
};
struct B: public A {};

static int function_call(kwsys::auto_ptr<A> a)
{
  return a.get()? 1:0;
}

static A* get_A(A& a) { return &a; }

static kwsys::auto_ptr<A> generate_auto_ptr_A()
{
  return kwsys::auto_ptr<A>(new A);
}

static kwsys::auto_ptr<B> generate_auto_ptr_B()
{
  return kwsys::auto_ptr<B>(new B);
}

int testAutoPtr(int, char*[])
{
  int status = 0;

  // Keep everything in a subscope so we can detect leaks.
  {
    kwsys::auto_ptr<A> pa0;
    kwsys::auto_ptr<A> pa1(new A());
    kwsys::auto_ptr<B> pb1(new B());
    kwsys::auto_ptr<B> pb2(new B());
    kwsys::auto_ptr<A> pa2(new B());

    A* ptr = get_A(*pa1);
    ASSERT(ptr == pa1.get(),
          "auto_ptr does not return correct object when dereferenced");
    ptr = pa1->self();
    ASSERT(ptr == pa1.get(),
          "auto_ptr does not return correct pointer from operator->");

    A* before = pa0.get();
    pa0.reset(new A());
    ASSERT(pa0.get() && pa0.get() != before,
          "auto_ptr empty after reset(new A())");

    before = pa0.get();
    pa0.reset(new B());
    ASSERT(pa0.get() && pa0.get() != before,
          "auto_ptr empty after reset(new B())");

    delete pa0.release();
    ASSERT(!pa0.get(), "auto_ptr holds an object after release()");

    kwsys::auto_ptr<A> pa3(pb1);
    ASSERT(!pb1.get(),
           "auto_ptr full after being used to construct another");
    ASSERT(pa3.get(),
          "auto_ptr empty after construction from another");

    {
    kwsys::auto_ptr<A> pa;
    pa = pa3;
    ASSERT(!pa3.get(),
           "auto_ptr full after assignment to another");
    ASSERT(pa.get(),
          "auto_ptr empty after assignment from another");
    }

    {
    kwsys::auto_ptr<A> pa;
    pa = pb2;
    ASSERT(!pb2.get(),
           "auto_ptr full after assignment to compatible");
    ASSERT(pa.get(),
          "auto_ptr empty after assignment from compatible");
    }

    {
    int receive = function_call(pa2);
    ASSERT(receive,
           "auto_ptr did not receive ownership in called function");
    ASSERT(!pa2.get(),
           "auto_ptr did not release ownership to called function");
    }

    {
    int received = function_call(generate_auto_ptr_A());
    ASSERT(received,
           "auto_ptr in called function did not take ownership "
           "from factory function");
    }

#if 0
    // Is this allowed by the standard?
    {
    int received = function_call(generate_auto_ptr_B());
    ASSERT(received,
           "auto_ptr in called function did not take ownership "
           "from factory function with conversion");
    }
#endif

    {
    kwsys::auto_ptr<A> pa(generate_auto_ptr_A());
    ASSERT(pa.get(),
      "auto_ptr empty after construction from factory function");
    }

    {
    kwsys::auto_ptr<A> pa;
    pa = generate_auto_ptr_A();
    ASSERT(pa.get(),
      "auto_ptr empty after assignment from factory function");
    }

    {
    kwsys::auto_ptr<A> pa(generate_auto_ptr_B());
    ASSERT(pa.get(),
      "auto_ptr empty after construction from compatible factory function");
    }

    {
    kwsys::auto_ptr<A> pa;
    pa = generate_auto_ptr_B();
    ASSERT(pa.get(),
      "auto_ptr empty after assignment from compatible factory function");
    }
  }

  ASSERT(instances == 0, "auto_ptr leaked an object");

  return status;
}
