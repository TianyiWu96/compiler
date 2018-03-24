#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include <map>

using namespace llvm;
using namespace std;

namespace {
	struct CountStaticInstructions : public FunctionPass 
	{
		static char ID;
		CountStaticInstructions() : FunctionPass(ID) {}

		bool runOnFunction(Function &F) override {
			
			map<string, int> count;

			for (inst_iterator start = inst_begin(F), 
				end = inst_end(F); 
				start != end; ++start) {
					++count[start->getOpcodeName()];
			}

			for (map<string, int>::iterator start = count.begin(),
				end = count.end();
				start != end; ++start) {
					errs() << start->first << '\t' << start->second << '\n';
			}

			return false;
		}
	};
}

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cse231-csi",
											   "CountStaticInstructions",
											   false, false);