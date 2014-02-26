
#include <cassert>

// Marked noreturn so that an unreached gets added after it
extern "C" void llvm_assume_trap(void) __attribute__ ((noreturn));

#define ASSUME(e) if(!(e)) { llvm_assume_trap(); } else {}

/// This simple example shows how a user provided assumption can simplify control flow.  Here, this function unconditionally returns true.
bool foo(int a) {
  ASSUME(a > 10)

  if( a > 10 ) {
    return true;
  } else {
    return false;
  }
}

struct A {
  int a;
  int b;

  int x;
  int y;
  int z;

  inline bool invariant() {
    return a == b && x <= y && y <= z;
  }
};

/// This is an example of a read only routine.  As we'd expect, there is no conditional due to the assert. 
int read_only(A& a) {
  ASSUME( a.invariant() && "precondition" );
  int rval = a.x-a.y+a.z;
  assert( a.invariant() && "post condition" );
  return rval;
}

/// This example shows a limited update to the state of the type which still does not require any runtime checking.
void limited_update(A& a) {
  ASSUME( a.invariant() && "precondition" );
  a.z = a.y;
  assert( a.invariant() && "post condition" );
}

/// In this example, we can't avoid the assert, but we are able to only check part of the condition.  The part not involving 'x <= y' can be optimized away.  With a more expensive invariant, this could be significant.
void partial_check(A& a) {
  ASSUME( a.x > 0 && a.invariant() && "precondition" );
  a.x--;
  a.y = a.z;
  assert( a.invariant() && "post condition" );
}

/// Just for comparison sake, here's the full check from the previous example.  Evwen here, the optimizer is able to remove one of the checks, but - in particular - not the ones involving 'a' and 'b' which are completely unmodified in this function.
void full_check(A& a) {
  a.x--;
  a.y = a.z;
  assert( a.invariant() && "post condition" );
}

