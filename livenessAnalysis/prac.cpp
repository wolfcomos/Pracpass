//===-- Frequent Path Loop Invariant Code Motion Pass --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
//
// EECS583 F23 - This pass can be used as a template for your FPLICM homework
//               assignment.
//               The passes get registered as "fplicm-correctness" and
//               "fplicm-performance".
//
//
////===-------------------------------------------------------------------===//
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include <cstdint>
#include <llvm-14/llvm/IR/Value.h>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include "llvm/CodeGen/MachineBasicBlock.h"
/* *******Implementation Starts Here******* */
// You can include more Header files here
#include "llvm/Analysis/DominanceFrontier.h"
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/Casting.h>
/* *******Implementation Ends Here******* */

using namespace llvm;

namespace
{
    struct livenessAnalysis: public PassInfoMixin<livenessAnalysis>
    {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
        {
            int changed = 2;
            std::unordered_map<BasicBlock*,std::unordered_set<Value*>> global_in;
            while(changed){
                int tracker = 0;
                for(auto &B : F){
                    // set of external variables consumed by bb
                    std::unordered_set<Value*> gen;
                    // set of external variables uses killed by bb
                    std::unordered_set<Value*> kill;
                    // IN = GEN + (OUT – KILL)
                    std::unordered_set<Value*> in;
                    // OUT = Union(IN(succs))
                    std::unordered_set<Value*> out;
                    llvm::raw_ostream & OS = llvm::outs();
                    OS<<"Basic Block: "<<tracker++<<"\n";
                    for(auto &I : reverse(B)){

                        if(I.getNumUses()>0){
                            gen.insert(dynamic_cast<Value*>(&I));
                            kill.erase(dynamic_cast<Value*>(&I));
                        }
                        else{
                            kill.insert(dynamic_cast<Value*>(&I));
                            gen.erase(dynamic_cast<Value*>(&I));
                        }
                                
                    }
                    OS<<"gen: \n";
                    for(auto *item : gen){
                        if(item == nullptr){
                            continue;
                        }
                        item->printAsOperand(OS,false,nullptr);
                        OS<<"\n";
                    }
                    OS<<"kill: \n";
                    for(auto *item : kill){
                        if(item == nullptr){
                            continue;
                        }
                        item->printAsOperand(OS,false,nullptr);
                        OS<<"\n";
                    }
                    for(auto *succ : successors(&B)){
                        out.insert(global_in[succ].begin(),global_in[succ].end());
                    }
                    //calculate in
                    in.insert(gen.begin(),gen.end());
                    in.insert(out.begin(),out.end());
                    // if(in.size() != global_in[&B].size()){
                    //     changed = true;
                    // }
                    global_in[&B] = in;
                    OS<<"in: \n";
                    for(auto *item : in){
                        if(item == nullptr){
                            continue;
                        }
                        item->printAsOperand(OS,false,nullptr);
                        OS<<"\n";
                    }
                    OS<<"out: \n";
                    for(auto *item : out){
                        if(item == nullptr){
                            continue;
                        }
                        item->printAsOperand(OS,false,nullptr);
                        OS<<"\n";
                    }
                }
                changed--;
            }
        // Your pass is modifying the source code. Figure out which analyses
        // are preserved and only return those, not all.
            return PreservedAnalyses::all();
        }
    };
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo()
{
  return {
      LLVM_PLUGIN_API_VERSION, "HW2Pass", "v0.1",
      [](PassBuilder &PB)
      {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM,
               ArrayRef<PassBuilder::PipelineElement>)
            {
              if (Name == "liveness-analysis")
              {
                FPM.addPass(livenessAnalysis());
                return true;
              }
              return false;
            });
      }};
}