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
    struct avaliableExpressionAnalysis: public PassInfoMixin<avaliableExpressionAnalysis>
    {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
        {
            std::unordered_map<BasicBlock*, std::unordered_set<Value*>> global_out;
            llvm::raw_ostream & OS = llvm::outs();
            for(int i = 0;i<2;++i){
                for(auto &B : F){
                    std::unordered_set<Value*> in;
                    std::unordered_set<Value*> out;
                    std::unordered_set<Value*> gen;
                    std::unordered_set<Value*> kill;
                    for(auto &I : B){
                        std::unordered_set<Value*> K;
                        for(auto & Use : I.uses()){
                            // OS<<"Use: " << dyn_cast<Value>(Use) << "\n";
                            K.insert(dyn_cast<Value>(Use));
                        }
                        if(K.find(dyn_cast<Value>(&I)) == K.end()){
                            gen.insert(dyn_cast<Value>(&I));
                            kill.erase(dyn_cast<Value>(&I));
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
                        for(auto item : K){
                            gen.erase(item);
                            kill.insert(item);
                        }
                    }
                    OS<<"in: \n";
                    bool first = false;
                    for(auto Pred : predecessors(&B)){
                        if(!first){
                            for(auto item : global_out[Pred]){
                                in.insert(item);
                            }
                            first = true;
                            continue;
                        }
                        for (auto it = in.begin(); it != in.end();) {
                            if (!global_out[Pred].count(*it)) {
                                it = in.erase(it); 
                            }
                            else{
                                ++it; 
                            }
                        }
                    }
                    for(auto item : in){
                        item->printAsOperand(OS,false,nullptr);
                        OS<<"\n";
                    }
                    OS<<"out: \n";
                    out.insert(in.begin(),in.end());
                    // out.erase(kill.begin(),kill.end());
                    // out.insert(gen.begin(),gen.end());
                    global_out[&B] = out;
                    for(auto item : out){
                        item->printAsOperand(OS,false,nullptr);
                        OS<<"\n";
                    }
                }
            }
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
              if (Name == "avaliable-expression-analysis")
              {
                FPM.addPass(avaliableExpressionAnalysis());
                return true;
              }
              return false;
            });
      }};
}