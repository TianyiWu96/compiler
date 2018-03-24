#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"

using namespace llvm;
using namespace std;

namespace {
    struct BranchBias : public FunctionPass {
        
        static char ID;

        BranchBias() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            Module *mod = F.getParent();

            LLVMContext &Context = mod->getContext();

            Function *print = dyn_cast<Function>(mod->getOrInsertFunction("printOutBranchInfo", 
                                                        Type::getVoidTy(Context)));

            Function *update = dyn_cast<Function>(mod->getOrInsertFunction("updateBranchInfo", 
                                                        Type::getVoidTy(Context), 
                                                        Type::getInt1Ty(Context)));
            
            for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
                IRBuilder<> Builder(&*B);
                //insert at the end
                Builder.SetInsertPoint(B->getTerminator());

                BranchInst *branch_inst = dyn_cast<BranchInst>(B->getTerminator());

                if (branch_inst != NULL && branch_inst->isConditional()) {
                    vector<Value *> num;
                    // prepare arguments for update function
                    num.push_back(branch_inst->getCondition());
                    // insert function
                    Builder.CreateCall(update, num);
                }

                for (BasicBlock::iterator i = B->begin(), j = B->end(); i != j; ++i) {
                    if ((string) i->getOpcodeName() == "ret") {
                        // get insert position
                        Builder.SetInsertPoint(&*i);
                        
                        Builder.CreateCall(print);
                    }
                }
            }

            return false;
        }
    }; 
}

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "BranchBias",
                             false, false);
