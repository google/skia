
template<typename T>
struct A
{
  typedef T Result;
};

void someFunc()
{
  A<A<int>> object;
  (void)object;
}
