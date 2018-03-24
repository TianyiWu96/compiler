#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

class ReachingInfo : public Info {

	public:

		set<unsigned> reaching_set;

		ReachingInfo() {}

		ReachingInfo(set<unsigned> set) {
			reaching_set = set;
		}

		set<unsigned> & getReaching_set() {
			return reaching_set;
		}

		void setReaching_set(set<unsigned> set) {
			reaching_set = set;
		}

		static bool equals(ReachingInfo * first, ReachingInfo * second) {
			return first->getReaching_set() == second->getReaching_set();
		}

		static ReachingInfo* join(ReachingInfo * info1, ReachingInfo * info2, ReachingInfo * res) {
			
			set<unsigned> firstSet = info1->getReaching_set();
			set<unsigned> secondSet = info2->getReaching_set();

			for (auto sb = secondSet.begin(), se = secondSet.end(); sb != se; ++sb) {
				firstSet.insert(*sb);
			}

			res->setReaching_set(firstSet);

			return nullptr;
		}

		void print() {
			for (auto sb = reaching_set.begin(), se = reaching_set.end(); sb != se; ++sb) {
				errs() << *sb << '|';
			}

			errs() << '\n';
		}
};


class ReachingDefinitionAnalysis : public DataFlowAnalysis<ReachingInfo, true> {
	private:
		typedef pair<unsigned, unsigned> Edge;
		map<Edge, ReachingInfo *> EdgeToInfo;
		map<string, int> opToidx = {{"alloca", 1}, {"load", 1}, 
										 {"fcmp", 1}, {"icmp", 1},
										 {"select", 1}, {"getelementptr", 1},
										 {"br", 2}, {"switch", 2}, {"store", 2}, 
										 {"phi", 3}};
	
	public:
		ReachingDefinitionAnalysis(ReachingInfo & bottom, ReachingInfo & initialState) : 
			DataFlowAnalysis(bottom, initialState) {}

		void flowfunction(Instruction * I, std::vector<unsigned> & IncomingEdges,
									   std::vector<unsigned> & OutgoingEdges,
									   std::vector<ReachingInfo *> & Infos) {
			
			string opName = I->getOpcodeName();

			int idx = opToidx.count(opName) ? opToidx[opName] : 2;
			
			if (I->isBinaryOp()) {
				idx = 1;
			}
			std::map<Edge, ReachingInfo *> edgeToInfo = getEdgeToInfo();

			unsigned index = getInstrToIndex()[I];
			
			ReachingInfo * info_out = new ReachingInfo();

			for (size_t i = 0; i < IncomingEdges.size(); ++i) {
				Edge in_edge = make_pair(IncomingEdges[i], index);
				ReachingInfo::join(info_out, edgeToInfo[in_edge], info_out);
			}

			if (idx == 1) {
				set<unsigned> temp = {index};
				ReachingInfo::join(info_out, new ReachingInfo(temp), info_out);
			}

			else if (idx == 3) {
				Instruction * non_phi = I->getParent()->getFirstNonPHI();

				unsigned non_phi_index = getInstrToIndex()[non_phi];
				
				set<unsigned> temp;

				for (unsigned i = index; i < non_phi_index; ++i) {
					temp.insert(i);
				}

				ReachingInfo::join(info_out, new ReachingInfo(temp), info_out);
			}

			for (size_t i = 0; i < OutgoingEdges.size(); ++i)
				Infos.push_back(info_out);
		}
};


namespace {
    struct ReachingDefinitionAnalysisPass : public FunctionPass {
        static char ID;
        ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            
            ReachingInfo initialState;
            ReachingInfo bottom;
            
            ReachingDefinitionAnalysis * rda = new ReachingDefinitionAnalysis(bottom, initialState);

            rda->runWorklistAlgorithm(&F);
            rda->print();

            return false;

        }
    }; 
}  

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching", "Reaching definition analysis on CFG",
                             							false ,
                             							false);