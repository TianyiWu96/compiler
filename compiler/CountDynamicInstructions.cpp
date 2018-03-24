#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"

#include <vector>
#include <map>

using namespace llvm;
using namespace std;

namespace {
	struct CountDynamicInstructions : public FunctionPass
	{
		static char ID;
		CountDynamicInstructions() : FunctionPass(ID) {}

		bool runOnFunction(Function &F) override {
			
			Module *mod = F.getParent();

			LLVMContext &Context = mod->getContext();

			Function *print = dyn_cast<Function>(mod->getOrInsertFunction("printOutInstrInfo",
							Type::getVoidTy(Context)));

			Function *update = dyn_cast<Function>(mod->getOrInsertFunction("updateInstrInfo", 
							Type::getVoidTy(Context),
							Type::getInt32Ty(Context),
							Type::getInt32PtrTy(Context),
							Type::getInt32PtrTy(Context)));


			for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B) {
				
				map<int, int> count;

				for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
					++count[I->getOpcode()];
				}

				vector<Constant *> fir;
				vector<Constant *> sec;

				IRBuilder<> Builder(&*B);

				Builder.SetInsertPoint(B->getTerminator());

				for (map<int, int>::iterator map_iter = count.begin(), E = count.end();
						map_iter != E; ++map_iter) {
					fir.push_back(ConstantInt::get(Type::getInt32Ty(Context), map_iter->first));
					sec.push_back(ConstantInt::get(Type::getInt32Ty(Context), map_iter->second));
				}

				vector<Value *> args;

				int size = count.size();

				ArrayType *array_ty = ArrayType::get(Type::getInt32Ty(Context), size);

				Value* val[2] = {ConstantInt::get(Type::getInt32Ty(Context), 0), ConstantInt::get(Type::getInt32Ty(Context), 0)};

				GlobalVariable *g_fir = new GlobalVariable(*mod, array_ty, true, GlobalVariable::InternalLinkage,
										ConstantArray::get(array_ty, fir),
										"g_fir");

				GlobalVariable *g_sec = new GlobalVariable(*mod, ArrayType::get(Type::getInt32Ty(Context), size),
										true,
										GlobalVariable::InternalLinkage,
										ConstantArray::get(array_ty, sec),
										"g_sec");

				args.push_back(ConstantInt::get(Type::getInt32Ty(Context), size));

				args.push_back(Builder.CreateInBoundsGEP(g_fir, val));

				args.push_back(Builder.CreateInBoundsGEP(g_sec, val));

				Builder.CreateCall(update, args);

				for (BasicBlock::iterator i = B->begin(), j = B->end();
					i != j; ++i) {
						if ((string) i->getOpcodeName() == "ret") {
							
							Builder.SetInsertPoint(&*i);

							Builder.CreateCall(print);
						}
				}

			}
			return false;
		}
	};
}

char CountDynamicInstructions::ID = 0;
static RegisterPass<CountDynamicInstructions> X ("cse231-cdi",
												 "CountDynamicInstructions",
												 false, false);