//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "DropAssumptionsPass"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
using namespace llvm;

STATISTIC(DropCounter, 
          "Counts number of assumptions converted to unreachables");

namespace {
  const std::string g_AssumeFunctionName("llvm_assume_trap");
  const std::string g_AssumeFunctionName2("llvm.assume.trap");

  /* Finds all calls to llvm.assume function inserted by codegen and
     drops them from the program. */
  struct DropAssumptionsPass : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    DropAssumptionsPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      bool functionChanged = false;

      // Search through each instruction in each basic block to find call 
      // instructions with the appropriate callee.

      for(Function::iterator bbitr = F.begin();
          bbitr != F.end(); bbitr++) {

        BasicBlock::iterator iitr = bbitr->begin();
        while( iitr != bbitr->end()) {
          // note: once we erase the call instruction, it's not clear
          // what happens to the basic block instruction iterator pointing
          // to it.  To be safe, avoid the need to increment that (probably 
          // invalid) iterator.
          BasicBlock::iterator next = iitr;
          next++;
          if( Instruction::Call == iitr->getOpcode() ) {
            CallInst* call = dyn_cast<CallInst>(iitr);
            assert( call );
            std::string sCallee = call->getCalledFunction()->getNameStr();
            if( g_AssumeFunctionName  == sCallee ||
                g_AssumeFunctionName2 == sCallee) {
              assert( call->use_empty() && 
                      "Cannot erase instruction that is used!" );
              ++DropCounter;
              // Remove this instruction from the basic block.  
              // Warning: calls delete(this); do not double delete.
              call->eraseFromParent(); 
              functionChanged = true;
            }
          }
          iitr = next;
        } // end while
      }
      return functionChanged;
    }

#if 0
    // For now, use the safe default which is to preserve nothing
    // This is rather wasteful and should be restricted to only those
    // analysis neccessary. (TODO)
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
#endif
  };
}

char DropAssumptionsPass::ID = 0;
static RegisterPass<DropAssumptionsPass> X("drop-assumptions", "Drop any calls to llvm.assume");
