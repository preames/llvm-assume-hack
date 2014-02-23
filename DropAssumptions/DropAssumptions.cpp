//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a custom pass which unconditionally removes calls to "llvm.assume_trap" and replaces them with "unreachable".
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "DropAssumptionsPass"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
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
      // Search through each instruction in each basic block to find call 
      // instructions with the appropriate callee.  We don't delete them
      // in this pass to avoid trouble with iterator invalidation.
      std::vector<CallInst*> toRemove;
      for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I) {
        if( CallInst* CI = dyn_cast<CallInst>(&*I) ) {
          Function* F = CI->getCalledFunction();
          if( !F ) continue; // indirect call
          if( F->getName().equals(g_AssumeFunctionName) ||
              F->getName().equals(g_AssumeFunctionName2) ) {
            toRemove.push_back(CI);
          }
        }
      }

      for(unsigned i = 0; i < toRemove.size(); i++) {
        CallInst* call = toRemove[i];
        assert( call );
        assert( call->use_empty() && 
                "Cannot erase instruction that is used!" );
        ++DropCounter;

        BasicBlock* BB = call->getParent();

        // Split the basic block and move the call into the new basic
        // block - this will all become dead code.
        call->getParent()->splitBasicBlock( BasicBlock::iterator(call), 
                                            "dead code");

        // get the newly inserted unconditional branch
        Instruction* br = BB->getTerminator();
        //insert the unreached before the branch and remove the branch
        //so the unreachable becomes the terminating instruction.  This
        //actual makes the new block dead.
        new UnreachableInst( F.getContext(), br);
        br->eraseFromParent();
        br = NULL;

        // Remove this instruction from the basic block.  
        // Warning: calls delete(this); do not double delete.
        call->eraseFromParent(); 
        // This wasn't actually needed since the entire BB is 
        // dead now, but is a useful cleanup.

        // best practice
        call = NULL;
        toRemove[i] = NULL;
      }

      F.dump();
      
      return !toRemove.empty();
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
