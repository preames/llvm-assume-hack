

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
