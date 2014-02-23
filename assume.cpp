
#include <cassert>

// Marked noreturn so that an unreached gets added after it
extern "C" void llvm_assume_trap(void) __attribute__ ((noreturn));

#define ASSUME(e) if(!(e)) { llvm_assume_trap(); } else {}

//should always return true
bool foo(int a) {
  ASSUME(a > 10)

  if( a > 10 ) {
    return true;
  } else {
    return false;
  }
}

struct A {
  int x;
  int y;
  int z;
};
  
void bar(A& a) {
  ASSUME( a.x <= a.y && a.y <= a.z && "precondition" );
  // Note: what can be between here is weaker than I'd like
  a.x = a.y;
  a.y = a.z;
  assert( a.x <= a.y && a.y <= a.z && "post condition" );
}

