#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"

#include "llvm/Transforms/Utils.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/ADT/GraphTraits.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Passes.h"

#include "llvm/Analysis/ScalarEvolution.h"
//#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/IRBuilder.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>

#include "llvm_cdfg.h"


using namespace llvm;

// mapping unit : loop
typedef struct
{
	bool isInnerLoop = false;
	Loop *lp;
	std::set<BasicBlock *> allBlocks;
	std::set<std::pair<BasicBlock *, BasicBlock *>> entryBlocks;
	std::set<std::pair<BasicBlock *, BasicBlock *>> exitBlocks;
} MappingUnit;

// parse function annotation to add CGRA_Hardware_Op attribute to function
// add by jhlou
#include <regex>

static bool applyHardwareOpAttrs(Function &Fn){
    bool changed = false;
    auto ensureAttr = [&](Attribute::AttrKind kind){
        if(!Fn.hasFnAttribute(kind)){
            Fn.addFnAttr(kind);
            changed = true;
        }
    };
    ensureAttr(Attribute::ReadOnly);
    ensureAttr(Attribute::NoUnwind);
    ensureAttr(Attribute::WillReturn);
    ensureAttr(Attribute::NoSync);
    ensureAttr(Attribute::Speculatable);
    return changed;
}

static bool tryFoldSpeculativeBranch(BasicBlock *BB, unsigned SpecSuccIdx,
                                     SmallVectorImpl<BasicBlock *> &ToErase,
                                     DenseSet<BasicBlock *> &DeadSet){
    Instruction *BrTerm = BB->getTerminator();
    if(!BrTerm){
        return false;
    }
    auto *Br = dyn_cast<BranchInst>(BrTerm);
    if(!Br || !Br->isConditional()){
        return false;
    }
    if(SpecSuccIdx >= Br->getNumSuccessors()){
        return false;
    }
    BasicBlock *SpecBB = Br->getSuccessor(SpecSuccIdx);
    BasicBlock *OtherBB = Br->getSuccessor(1 - SpecSuccIdx);
    if(SpecBB == BB || DeadSet.count(SpecBB)){
        return false;
    }
    if(!SpecBB->hasNPredecessors(1)){
        return false;
    }
    Instruction *SpecTerm = SpecBB->getTerminator();
    if(!SpecTerm){
        return false;
    }
    auto *SpecBr = dyn_cast<BranchInst>(SpecTerm);
    if(!SpecBr || SpecBr->isConditional()){
        return false;
    }
    if(SpecBr->getSuccessor(0) != OtherBB){
        return false;
    }

    SmallVector<Instruction *, 8> MovableInsts;
    for(Instruction &Inst : *SpecBB){
        if(Inst.isTerminator())
            break;
        if(!isSafeToSpeculativelyExecute(&Inst)){
            return false;
        }
        for(User *U : Inst.users()){
            if(auto *UseInst = dyn_cast<Instruction>(U)){
                if(UseInst->getParent() == SpecBB)
                    continue;
                if(auto *PN = dyn_cast<PHINode>(UseInst)){
                    if(PN->getParent() == OtherBB)
                        continue;
                }
                return false;
            }
        }
        MovableInsts.push_back(&Inst);
    }

    SmallVector<PHINode *, 4> Phis;
    for(auto &I : OtherBB->phis()){
        auto *PN = &I;
        if(PN->getNumIncomingValues() != 2){
            return false;
        }
        bool hasSpec = false;
        bool hasBB = false;
        for(unsigned idx = 0; idx < 2; ++idx){
            BasicBlock *IncomingBB = PN->getIncomingBlock(idx);
            if(IncomingBB == SpecBB){
                hasSpec = true;
            }else if(IncomingBB == BB){
                hasBB = true;
            }else{
                return false;
            }
        }
        if(!(hasSpec && hasBB)){
            return false;
        }
        Phis.push_back(PN);
    }
    if(Phis.empty()){
        return false;
    }

    for(Instruction *Inst : MovableInsts){
        Inst->moveBefore(Br);
    }

    IRBuilder<> Builder(Br);
    Value *Cond = Br->getCondition();
    bool SpecIsFalseSucc = (SpecSuccIdx == 1);
    for(PHINode *PN : Phis){
        Value *ValFromSpec = PN->getIncomingValueForBlock(SpecBB);
        Value *ValFromBB = PN->getIncomingValueForBlock(BB);
        Value *TrueVal = SpecIsFalseSucc ? ValFromBB : ValFromSpec;
        Value *FalseVal = SpecIsFalseSucc ? ValFromSpec : ValFromBB;
        Value *Sel = Builder.CreateSelect(Cond, TrueVal, FalseVal, PN->getName());
        PN->replaceAllUsesWith(Sel);
        PN->eraseFromParent();
    }

    BranchInst::Create(OtherBB, BB);
    Br->eraseFromParent();
    DeadSet.insert(SpecBB);
    ToErase.push_back(SpecBB);
    return true;
}

static bool simplifySpeculativeBranches(Function &F){
    bool changed = false;
    SmallVector<BasicBlock *, 8> ToErase;
    DenseSet<BasicBlock *> DeadSet;
    for(auto BI = F.begin(); BI != F.end(); ){
        BasicBlock *BB = &*BI++;
        if(DeadSet.count(BB))
            continue;
        for(unsigned idx = 0; idx < 2; ++idx){
            if(tryFoldSpeculativeBranch(BB, idx, ToErase, DeadSet)){
                changed = true;
                break;
            }
        }
    }
    for(BasicBlock *BB : ToErase){
        BB->eraseFromParent();
    }
    return changed;
}

void ParseHardwareImplementFunction(Function &F){
    auto *GV = F.getParent()->getNamedGlobal("llvm.global.annotations");
    if (!GV) return;
    auto *CA = dyn_cast<ConstantArray>(GV->getInitializer());
    if (!CA) return;

    std::regex pattern("^___CGRA_HARDWARE_OP__(.+)___$");
    for (unsigned i = 0, e = CA->getNumOperands(); i != e; ++i) {
        auto *CS = dyn_cast<ConstantStruct>(CA->getOperand(i));
        if (!CS || CS->getNumOperands() < 2) continue;

        Function *fn = nullptr;
        if (auto *CE = dyn_cast<ConstantExpr>(CS->getOperand(0))) {
            if (CE->getOpcode() == Instruction::BitCast && CE->getNumOperands() >= 1)
                fn = dyn_cast<Function>(CE->getOperand(0));
        } else {
            fn = dyn_cast<Function>(CS->getOperand(0));
        }
        if (!fn) continue;

        std::string anno;
        Value *AnnoOp = CS->getOperand(1);
        if (auto *CE = dyn_cast<ConstantExpr>(AnnoOp)) {
            if (CE->getOpcode() == Instruction::GetElementPtr && CE->getNumOperands() >= 1) {
                if (auto *StrGV = dyn_cast<GlobalVariable>(CE->getOperand(0))) {
                    if (auto *CDA = dyn_cast<ConstantDataArray>(StrGV->getInitializer()))
                        anno = CDA->getAsCString().str();
                }
            }
        } else if (auto *StrGV = dyn_cast<GlobalVariable>(AnnoOp)) {
            if (auto *CDA = dyn_cast<ConstantDataArray>(StrGV->getInitializer()))
                anno = CDA->getAsCString().str();
        }
        if (anno.empty()) continue;

        std::smatch match;
        if (std::regex_match(anno, match, pattern) && match.size() > 1) {
            std::string extracted = match[1];
            bool hadAttr = fn->hasFnAttribute("__CGRA_Hardware_Op");
            fn->addFnAttr("__CGRA_Hardware_Op", extracted);
            if(!hadAttr) applyHardwareOpAttrs(*fn);
        }
    }
}

struct HardwareOpAnnotatePass : public ModulePass{
    static char ID;
    HardwareOpAnnotatePass() : ModulePass(ID) {}

    bool runOnModule(Module &M) override{
        bool changed = false;
        for(Function &F : M){
            bool hadAnno = F.hasFnAttribute("__CGRA_Hardware_Op");
            ParseHardwareImplementFunction(F);
            bool hasAnno = F.hasFnAttribute("__CGRA_Hardware_Op");
            if(hasAnno && !hadAnno){
                changed = true;
            }
            if(hasAnno){
                // Ensure attrs are present even if annotation already processed elsewhere.
                changed |= applyHardwareOpAttrs(F);
            }
            changed |= simplifySpeculativeBranches(F);
        }
        return changed;
    }
};

char HardwareOpAnnotatePass::ID = 0;
static RegisterPass<HardwareOpAnnotatePass> HardwareOpAnnotatePassReg("hw-annotate",
    "Annotate hardware ops for upstream optimizations",
    false,
    false);

// parse function annotation, attribute key-values
// sizeAttrMap[attrKey] = attrValue
void ParseSizeAttr(Function &F, std::map<std::string, int>& sizeAttrMap)
{
    auto *GV = F.getParent()->getNamedGlobal("llvm.global.annotations");
    if (GV) {
        if (auto *CA = dyn_cast<ConstantArray>(GV->getInitializer())) {
            for (unsigned i = 0, e = CA->getNumOperands(); i != e; ++i) {
                auto *CS = dyn_cast<ConstantStruct>(CA->getOperand(i));
                if (!CS || CS->getNumOperands() < 2) continue;
                Function *fn = nullptr;
                if (auto *CE = dyn_cast<ConstantExpr>(CS->getOperand(0))) {
                    if (CE->getOpcode() == Instruction::BitCast && CE->getNumOperands() >= 1)
                        fn = dyn_cast<Function>(CE->getOperand(0));
                } else {
                    fn = dyn_cast<Function>(CS->getOperand(0));
                }
                if (!fn) continue;
                Value *AnnoOp = CS->getOperand(1);
                StringRef annoStr;
                if (auto *CE = dyn_cast<ConstantExpr>(AnnoOp)) {
                    if (CE->getOpcode() == Instruction::GetElementPtr && CE->getNumOperands() >= 1) {
                        if (auto *StrGV = dyn_cast<GlobalVariable>(CE->getOperand(0))) {
                            if (auto *CDA = dyn_cast<ConstantDataArray>(StrGV->getInitializer()))
                                annoStr = CDA->getAsCString();
                        }
                    }
                } else if (auto *StrGV = dyn_cast<GlobalVariable>(AnnoOp)) {
                    if (auto *CDA = dyn_cast<ConstantDataArray>(StrGV->getInitializer()))
                        annoStr = CDA->getAsCString();
                }
                if (!annoStr.empty()) {
                    fn->addFnAttr("size", annoStr);
                }
            }
        }
    }
	if (F.hasFnAttribute("size"))
	{
		Attribute attr = F.getFnAttribute("size");
		outs() << "Size attribute : " << attr.getValueAsString() << "\n";
		StringRef sizeAttrStr = attr.getValueAsString();
		SmallVector<StringRef, 8> sizeArr;
		sizeAttrStr.split(sizeArr, ',');
		for (int i = 0; i < sizeArr.size(); ++i)
		{
			std::pair<StringRef, StringRef> splitDuple = sizeArr[i].split(':');
			uint32_t size;
			splitDuple.second.getAsInteger(10, size);
			outs() << "ParseAttr:: name:" << splitDuple.first << ",size:" << size << "\n";
			sizeAttrMap[splitDuple.first.str()] = size;
		}
	}
}


// Depth-first search for the recursively successive basic blocks except the back-edge BBs
void dfsForSuccBBs(SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> BackEdgeBBPs,
		std::map<const BasicBlock *, std::set<const BasicBlock *>> &SuccBBsMap,
		const BasicBlock *currBB, const BasicBlock *startBB){
	for (auto iter = succ_begin(currBB); iter != succ_end(currBB); ++iter){
		const BasicBlock *succ = *iter;
        // skip back-edge BB
		std::pair<const BasicBlock *, const BasicBlock *> bbPair(currBB, succ);
		if (std::find(BackEdgeBBPs.begin(), BackEdgeBBPs.end(), bbPair) != BackEdgeBBPs.end()){//if find back-egde
			continue;
		}
		SuccBBsMap[startBB].emplace(succ);
		dfsForSuccBBs(BackEdgeBBPs, SuccBBsMap, succ, startBB);
	}
}


// get the recursively successive basic blocks except the back-edge BBs for each BB  
// Including self-BB
void getSuccBBsMap(Function &F, SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> &BackEdgeBBPs, 
        std::map<const BasicBlock *, std::set<const BasicBlock *>> &SuccBBsMap){
    for(auto &BB : F){
        const BasicBlock *BBPtr = &BB;
        SuccBBsMap[BBPtr].emplace(BBPtr);
        dfsForSuccBBs(BackEdgeBBPs, SuccBBsMap, BBPtr, BBPtr);
    }
}


void printSuccBBsMap(std::map<const BasicBlock *, std::set<const BasicBlock *>> &SuccBBsMap){
    outs() << "###### SuccBBsMap ######\n";
	for (auto &elem : SuccBBsMap){
		outs() << "SuccBBs(" << elem.first->getName().str() << ") : ";
		for (auto &succ : elem.second){
			outs() << succ->getName().str() << ", ";
		}
		outs() << "\n";
	}
}


// // get BBs not in the loops
// void getNonLoopBBs(Function &F, std::vector<Loop *> &loops, std::set<BasicBlock *> &nonloopBBs){
// 	nonloopBBs.clear();
//     for (BasicBlock &BB : F){
// 		BasicBlock *BBPtr = &BB;
// 		nonloopBBs.emplace(BBPtr);
// 	}
// 	for (Loop *lp : loops){
// 		for (BasicBlock *BB : lp->getBlocks()){
// 			nonloopBBs.erase(BB);
// 		}
// 	}
// }

// get inner-most loops (mapping units) according to LoopInfo
void getInnerMostLoops(std::map<std::string, MappingUnit> &mappingUnitMap, std::vector<std::string> &MappingUnitQueue, std::map<Loop *, std::string> &loopNames, 
    std::vector<Loop *> loops, std::string lnstr)
{
	for (int i = 0; i < loops.size(); ++i)
	{
        auto& loop = loops[i];
        std::string name = lnstr + "_" + std::to_string(i);
		loopNames[loop] = name;

		if (loop->getSubLoops().size() == 0){
			// innerMostLoops.push_back(loop);
            std::string loopName = "INNERMOST_" + name;
			mappingUnitMap[loopName].isInnerLoop = true;
			mappingUnitMap[loopName].lp = loop;
            mappingUnitMap[loopName].allBlocks.insert(loop->getBlocks().begin(), loop->getBlocks().end());
            SmallVector<std::pair<BasicBlock *, BasicBlock *>, 8> exitEdges;
            loop->getExitEdges(exitEdges);
            mappingUnitMap[loopName].exitBlocks.insert(exitEdges.begin(), exitEdges.end());
            mappingUnitMap[loopName].entryBlocks.insert(std::make_pair(loop->getLoopPredecessor(), loop->getHeader()));
			MappingUnitQueue.push_back(loopName);
			// for (BasicBlock *BB : loop->getBlocks()){
			// 	// loopsExclusieBasicBlockMap[loops[i]].push_back(BB);
			// 	// mappingUnitMap[loopName].allBlocks.insert(BB);
			// 	BB2MUnitMap[BB] = loopName;
			// }
		}else{
			getInnerMostLoops(mappingUnitMap, MappingUnitQueue, loopNames, loop->getSubLoops(), name);

			// for (BasicBlock *BB : loop->getBlocks()){
			// 	bool BBfound = false;
			// 	for (std::pair<Loop *, std::vector<BasicBlock *>> pair : loopsExclusieBasicBlockMap)
			// 	{
			// 		if (std::find(pair.second.begin(), pair.second.end(), BB) != pair.second.end())
			// 		{
			// 			BBfound = true;
			// 			break;
			// 		}
			// 	}
			// 	if (!BBfound)
			// 	{
			// 		loopsExclusieBasicBlockMap[loops[i]].push_back(BB);
			// 	}
			// }
		}
	}
}


void printMappingUnitMap(std::map<std::string, MappingUnit> &mappingUnitMap)
{
	outs() << "###### MappingUnitMap ###### \n";
	for (std::pair<std::string, MappingUnit> pair : mappingUnitMap)
	{
		outs() << pair.first << " :: ";
		for (BasicBlock *bb : pair.second.allBlocks)
		{
			outs() << bb->getName() << ", ";
		}
		outs() << "| entry = ";
		for (std::pair<BasicBlock *, BasicBlock *> bbPair : pair.second.entryBlocks)
		{
			outs() << bbPair.first->getName() << " to " << bbPair.second->getName() << ", ";
		}
		outs() << "| exit = ";
		for (std::pair<BasicBlock *, BasicBlock *> bbPair : pair.second.exitBlocks)
		{
			outs() << bbPair.first->getName() << " to " << bbPair.second->getName() << ", ";
		}
		outs() << "\n";
	}
}

// get target loop name (mapping unit name) using the token function "please_map_me"
std::string getMappingUnitNameUsingPleasemapme(Function &F, std::map<std::string, MappingUnit> &mappingUnitMap)
{
	BasicBlock *MUBB;
	Instruction *checker_ins = NULL;
	for (auto &BB : F){
		// BB.dump();
		for (auto &I : BB){
			if (CallInst *CI = dyn_cast<CallInst>(&I)){
				std::string op_str;
				raw_string_ostream rs(op_str);
				CI->print(rs);
				outs() << "op : " << rs.str() << "\n";
				if (op_str.find("please_map_me") != std::string::npos){
					outs() << "token found in BB = " << BB.getName() << "\n";
					MUBB = &BB;
                    checker_ins = CI;
				}
			}
		}
	}
	assert(MUBB);
	assert(checker_ins);
	checker_ins->eraseFromParent();

	for (auto &elem : mappingUnitMap){
		errs() << "loop name: "<< elem.first << "\n";
		if (elem.second.lp->contains(MUBB)){
			return elem.first;
		}
	}
	assert(false);
}

// get target loop name (mapping unit name) using the token function "loop_begin()" & "loop_end()"
std::vector<std::map<int, Loop*>> getMappingUnitNameUsingLoopMark(Function &F, std::map<std::string, MappingUnit> &mappingUnitMap,
						std::vector<std::string> &MappingUnitQueue,
						std::map<const llvm::BasicBlock *, std::set<const llvm::BasicBlock *>> SuccBBsMap,
						std::map<Loop*, std::vector<Instruction *>> &outMarkLoopIns, std::vector<BasicBlock*> &TaskIOBBs)
{
	BasicBlock *EntryBB, *ExitBB;
	Instruction *LoopStart = NULL;
	Instruction *LoopEnd = NULL;
	std::vector<std::map<int, Loop*>> LoopVec;
	std::map<int, Loop*> nestLoop;
	std::map<int, Loop*> finalnestloop;
	outMarkLoopIns.clear();
	for (auto &BB : F){
		// BB.dump();
		for (auto &I : BB){
			if (CallInst *CI = dyn_cast<CallInst>(&I)){
				std::string op_str;
				raw_string_ostream rs(op_str);
				CI->print(rs);
				outs() << "op : " << rs.str() << "\n";
				if (op_str.find("loop_begin") != std::string::npos){
					outs() << "EntryBB = " << BB.getName() << "\n";
					EntryBB = &BB;
                    LoopStart = CI;
				}
				if (op_str.find("loop_end") != std::string::npos){
					outs() << "LoopEnd = " << BB.getName() << "\n";
					ExitBB = &BB;
                    LoopEnd = CI;
				}
			}
		}
	}
	TaskIOBBs.push_back(EntryBB);
	TaskIOBBs.push_back(ExitBB);
	if(LoopStart == NULL && LoopEnd == NULL){
		LoopVec.clear();
		nestLoop.clear();
	}else if(LoopStart != NULL && LoopEnd != NULL){
		std::set<BasicBlock*> internalBBs;
		for(auto temBB : SuccBBsMap[EntryBB]){
			if(!SuccBBsMap[ExitBB].count(temBB)){
				internalBBs.insert((BasicBlock*)temBB);
			}
		}
		errs() << "internalBBs: ";
		for(auto iter:internalBBs){
			errs()<< iter->getName() <<"; ";
		}
		errs() << "\n";

		for (auto &elem : MappingUnitQueue){
			errs() << "loop name: "<< elem << "\n";
			nestLoop.clear();
			finalnestloop.clear();
			auto tem = mappingUnitMap[elem].lp;
			bool allin = true;
			int level = 0;
			while(tem != nullptr){
				nestLoop[level] = tem;
				tem = tem->getParentLoop();
				level++; 
			}
			//auto loopBBs = nestLoop[level-1]->getBlocks();
			int targetlevel = -1;
			for(int i = 0; i < level; i++){
				auto loopBBs = nestLoop[i]->getBlocks();
				bool hereEntryBB = false;
				for(auto BB : loopBBs){
					if(BB == EntryBB){
						assert(i!=0);
						targetlevel = i -1;
						hereEntryBB = true;
					}
				}
				if(hereEntryBB)
					break;
				finalnestloop[i] = nestLoop[i];
			}
			if(targetlevel == -1)
				targetlevel = level-1;
			auto loopBBs = finalnestloop[targetlevel]->getBlocks();
			for(auto BB : loopBBs){
				if(!internalBBs.count(BB)){
					allin = false;
					break;
				}
			}
			if (allin){
				LoopVec.push_back(finalnestloop);
			}
		}

		errs() << "Here we try to figure out outMarkLoopIns(especially for the internal blocks between two loops)\n";
		//at the beginning and the end, mark the instructions before Loop_begin and after Loop_end
		auto firstInnermostLoop = *(LoopVec.end() - 1);
		errs() << "begin header: "<< firstInnermostLoop[0]->getLoopPreheader()->getName() << "\n";
		for(auto &ins : *EntryBB){
			if(&ins == LoopStart)
				break;
			outMarkLoopIns[firstInnermostLoop[firstInnermostLoop.size() - 1]].push_back(&ins);
		}
		bool endFlag = false;
		// errs() << "ExitBB order: ";
		auto lastInnermostLoop = *(LoopVec.begin());
		errs() << "end header: "<< lastInnermostLoop[0]->getLoopPreheader()->getName() << "\n";
		for(auto &ins : *ExitBB){
			// ins.dump();
			if(endFlag){
				outMarkLoopIns[lastInnermostLoop[lastInnermostLoop.size() - 1]].push_back(&ins);
			}
			if(&ins == LoopEnd){
				endFlag = true;
			}
		}
		LoopStart->eraseFromParent();
		LoopEnd->eraseFromParent();
		//now to handle the BasicBlocks between subtasks
		errs() << "Now to handle the BasicBlocks between subtasks:\n";
		int i = 0;
		for(auto &elem : LoopVec){
			errs() << " ---------handling Loop " << LoopVec.size() - i << "\n";
			auto outtermostLoop = elem[elem.size() - 1];
			BasicBlock* currEntry = outtermostLoop->getLoopPredecessor();
			BasicBlock* currExit = outtermostLoop->getExitBlock();
			assert(currExit != NULL && "if any loop has multi-exit?");
			for(auto &I : *currEntry){
				Instruction* ins = &I;
				bool notUsed = true;
				std::vector<Instruction*> insStack;
				insStack.push_back(ins);
				while (!insStack.empty())
				{
					auto currIns = insStack.back();
					insStack.pop_back();
					for(User* succ : currIns->users()){
						if(Instruction *succIns = dyn_cast<Instruction>(succ)){
							if(outtermostLoop->contains(succIns)){
								notUsed = false;
								break;
							}else{
								insStack.push_back(succIns);
							}
						}else{
							succ->dump();
							assert(false && "what kind of use");
						}
					}
					if(notUsed == false){
						break;
					}
				}
				if(notUsed){
					errs() << "NOT USED: "; ins->dump();
					outMarkLoopIns[outtermostLoop].push_back(ins);
				}
			}
			for(auto &I : *currExit){
				Instruction* ins = &I;
				bool notUse = true;
				std::vector<Instruction*> insStack;
				insStack.push_back(ins);
				while (!insStack.empty())
				{
					auto currIns = insStack.back();
					insStack.pop_back();
					for(auto &pred : currIns->operands()){
						if(Instruction *predIns = dyn_cast<Instruction>(&pred)){
							if(outtermostLoop->contains(predIns)){
								notUse = false;
								break;
							}else if(predIns->getParent() == currExit){
								insStack.push_back(predIns);
							}else if(predIns->getParent() == currEntry){
								notUse = false;
								break;
							}else{
								///TOFIX: if use the Value of other loops, should it be added to them?
								// predIns->dump();
								// assert(false && "Traced back to which BB? neither ExitBB nor Loop");
							}
						}else{
							// (&pred)->dump();
							// assert(false && "what kind of use");
						}
					}
					if(notUse == false){
						break;
					}
				}
				if(notUse){
					errs() << "NOT USE: "; ins->dump();
					outMarkLoopIns[outtermostLoop].push_back(ins);
				}
			}
			i++;
		}


	}else{
		assert(false && "illegal loop mark");
	}
	return LoopVec;
}

//extract information of Load/Store in this kernel
void extractLSInfo(LLVMCDFG* CDFG, int kernel, std::map<int, std::pair<std::set<LLVMCDFGNode*>, std::set<LLVMCDFGNode*>>>* LSTable){
	for(auto elem : CDFG->nodes()){
		auto node = elem.second;
		Instruction* ins = node->instruction();
		GetElementPtrInst* GEPins = NULL;
		if(ins != NULL){
			if(dyn_cast<LoadInst>(ins)){
				(*LSTable)[kernel].first.insert(node);
			}
			else if(dyn_cast<StoreInst>(ins)){
				(*LSTable)[kernel].second.insert(node);
			}
		}
		///TODO: maybe use other name as symbol of regIO(?)
		else{
			if(node->finalInstruction() == "INPUT"){
				(*LSTable)[kernel].first.insert(node);
			}
			else if(node->finalInstruction() == "OUTPUT"){
				(*LSTable)[kernel].second.insert(node);
			}
		}
	}
}

//merge tasks and schedule accroding to access dependence
void scheduleTasks(std::map<int, LLVMCDFG*> CDFGs, std::map<int, std::pair<std::set<LLVMCDFGNode*>, std::set<LLVMCDFGNode*>>> LSTable){
	outs() <<">>>>>> Merge tasks and Schedule accroding to access dependence <<<<<<\n";
	std::map<int, std::map<Value*, std::pair<std::set<LLVMCDFGNode*>, std::set<LLVMCDFGNode*>>>> LSaddrTable;
	std::map<int, std::pair<std::set<Value*>, std::set<Value*>>> LSaddrSets;
	//for(int i = 0; i < LSTable.size(); i++){
	for(int i = 0; i < CDFGs.size(); i++){
		outs() << "-----kernel_" << i << "-----\n";
		outs() << "\tload values: ";
		std::set<LLVMCDFGNode*> LoadSet = CDFGs[i]->getLoadList();
		for(auto LoadNode : LoadSet){
			auto addrValue = LoadNode->getLSaddress();
			assert(addrValue != NULL);
			outs() << addrValue->getName().str() << ", ";
			LSaddrTable[i][addrValue].first.insert(LoadNode);
			LSaddrSets[i].first.insert(addrValue);
		}
		outs() << "\n\tstore values: ";
		std::set<LLVMCDFGNode*> StoreSet = CDFGs[i]->getStoreList();
		for(auto StoreNode : StoreSet){
			auto addrValue = StoreNode->getLSaddress();
			assert(addrValue != NULL);
			outs() << addrValue->getName().str() << ", ";
			LSaddrTable[i][addrValue].second.insert(StoreNode);
			LSaddrSets[i].second.insert(addrValue);
		}
		outs() << "\n";
	}

	std::map<Value*, int> maxSizeMap;
	std::map<Value*, int> minOffsetMap;
	for(auto &LSaddrTable_elem : LSaddrTable){
		int CDFGidx = LSaddrTable_elem.first;
		auto nodeMap = LSaddrTable_elem.second;
		for(auto &nodeMap_elem : nodeMap){
			Value* addrValue = nodeMap_elem.first;
			outs() << addrValue->getName() << ":\n";
			for(auto &LodeNode : nodeMap_elem.second.first){
				outs() << "\t" << LodeNode->getLSoffset().to_string() << ", ";
				outs() << LodeNode->getLSstart().to_string() << "\n";
				auto LoadBound = LodeNode->getLSbounds();
				varType size = 0;
				size = LoadBound[1]-LoadBound[0]+LoadBound[2];
				assert(size.index() != 2);
				int sizeint = std::get<int>(size.value);
                int LSoffset = std::get<int>(LodeNode->getLSoffset().value);
                if(minOffsetMap[addrValue] < LSoffset){
                    sizeint += (LSoffset-minOffsetMap[addrValue]);
                }
                if(!maxSizeMap.count(addrValue)){
                    maxSizeMap[addrValue] = sizeint;
                }else if(maxSizeMap[addrValue] < sizeint){
                    maxSizeMap[addrValue] = sizeint;
                }
			}
			for(auto &StoreNode : nodeMap_elem.second.second){
				outs() << "\t" << StoreNode->getLSoffset().to_string() << ", ";
				outs() << StoreNode->getLSstart().to_string() << "\n";
				auto StoreBound = StoreNode->getLSbounds();
				varType size = 0;
				size = StoreBound[1]-StoreBound[0]+StoreBound[2];
				assert(size.index() != 2);
				int sizeint = std::get<int>(size.value);
                int LSoffset = std::get<int>(StoreNode->getLSoffset().value);
                if(minOffsetMap[addrValue] < LSoffset){
                    sizeint += (LSoffset-minOffsetMap[addrValue]);
                }
                if(!maxSizeMap.count(addrValue)){
                    maxSizeMap[addrValue] = sizeint;
                }else if(maxSizeMap[addrValue] < sizeint){
                    maxSizeMap[addrValue] = sizeint;
                }
			}
		}
	}

	for(int i = 0; i < CDFGs.size(); i++){
		CDFGs[i]->setmaxSizeMap(maxSizeMap);
		CDFGs[i]->setminOffsetMap(minOffsetMap);
	}

	std::ofstream ofs;
	ofs.open("whole_DFG.dot");
	ofs << "digraph g{\n";
	
	// std::set<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>> nodepairs;
	std::set<LLVMCDFGNode*> dependedNodes;
	int nextflag = 0;
	// for(int i = 0; i < CDFGs.size(); i++){
	for(int i = CDFGs.size()-1; i >= 0; i--){
		bool isFirstStore = true;
		std::set<Value*> preStoreAddrSet = LSaddrSets[i].second;
		for(int j = i+1; j < CDFGs.size(); j++){
			//RAW
			std::set<Value*> LoadAddrSet = LSaddrSets[j].first;
			for(auto LoadAddr:LoadAddrSet){
				if(preStoreAddrSet.count(LoadAddr)){
					for(auto src:LSaddrTable[i][LoadAddr].second){
						auto StoreIns = src->instruction();
						for(auto dst:LSaddrTable[j][LoadAddr].first){
							if(dependedNodes.count(dst)){
								continue;
							}
							LLVMCDFGNode* srcCp = src;
							if(!isFirstStore){
								///duplicate memory nodes for multiport accesses
								// srcCp = CDFGs[i]->addCpNode(src);
								// errs() << "add new store node: " << srcCp->getName() << ";";
								// for(auto srcParent:src->inputNodes()){
								// 	CDFGs[i]->connectNodes(srcParent, srcCp, src->getInputIdx(srcParent), EDGE_TYPE_DATA);
								// }
							}
							if(isFirstStore){
								isFirstStore = false;
							}
							std::string scrName = srcCp->getName() + "_" + std::to_string(i);
							std::string dstName = dst->getName() + "_" + std::to_string(j);
							dependedNodes.insert(dst);
							int trueFlag;
							///duplicate memory nodes for multiport accesses
							// if(dst->getDependenceFlag() == -1){//the dstNode is added dependent srcNode for the first time
							// 	//use the new flag
							// 	trueFlag = nextflag;
							// 	nextflag++;
							// }else{
							// 	trueFlag = dst->getDependenceFlag();
							// }
							if(src->getDependenceFlag() == -1 && dst->getDependenceFlag() == -1){
								trueFlag = nextflag;
								nextflag++;
							}else{
								trueFlag = (src->getDependenceFlag() == -1) ? dst->getDependenceFlag() : src->getDependenceFlag();
							}
							srcCp->setDependenceFlag(trueFlag);
							dst->setDependenceFlag(trueFlag);
							outs() << scrName << " -> " << dstName << " label = \"RAW\"\n";
							ofs << scrName << " -> " << dstName << "[color = blue, label = \"RAW, ";
							ofs << "depflag = " << trueFlag << "\", ";
							ofs << "dependency = " << trueFlag << "];\n";
						}
					}
				}
			}
			//WAW
			std::set<Value*> StoreAddrSet = LSaddrSets[j].second;
			for(auto StoreAddr:StoreAddrSet){
				if(preStoreAddrSet.count(StoreAddr)){
					for(auto src:LSaddrTable[i][StoreAddr].second){
						auto StoreIns = src->instruction();
						for(auto dst:LSaddrTable[j][StoreAddr].second){
							if(dependedNodes.count(dst)){
								continue;
							}
							LLVMCDFGNode* srcCp = src;
							if(!isFirstStore){
								///duplicate memory nodes for multiport accesses
								// srcCp = CDFGs[i]->addCpNode(src);
								// errs() << "add new store node: " << srcCp->getName() << ";";
								// for(auto srcParent:src->inputNodes()){
								// 	CDFGs[i]->connectNodes(srcParent, srcCp, src->getInputIdx(srcParent), EDGE_TYPE_DATA);
								// }
							}
							if(isFirstStore){
								isFirstStore = false;
							}
							std::string scrName = srcCp->getName() + "_" + std::to_string(i);
							std::string dstName = dst->getName() + "_" + std::to_string(j);
							dependedNodes.insert(dst);
							int trueFlag;
							///duplicate memory nodes for multiport accesses
							// if(dst->getDependenceFlag() == -1){//the dstNode is added dependent srcNode for the first time
							// 	//use the new flag
							// 	trueFlag = nextflag;
							// 	nextflag++;
							// }else{
							// 	trueFlag = dst->getDependenceFlag();
							// }
							if(src->getDependenceFlag() == -1 && dst->getDependenceFlag() == -1){
								trueFlag = nextflag;
								nextflag++;
							}else{
								trueFlag = (src->getDependenceFlag() == -1) ? dst->getDependenceFlag() : src->getDependenceFlag();
							}
							srcCp->setDependenceFlag(trueFlag);
							dst->setDependenceFlag(trueFlag);
							outs() << scrName << " -> " << dstName << " label = \"WAW\"\n";
							ofs << scrName << " -> " << dstName << "[color = blue, label = \"WAW, ";
							ofs << "depflag = " << trueFlag << "\", ";
							ofs << "dependency = " << trueFlag << "];\n";
						}
					}
				}
			}
		}
	}
	for(int i = 0; i < CDFGs.size(); i++){
		CDFGs[i]->printAsSubTask(ofs, i);
	}
	ofs << "}\n";
	ofs.close();
}


namespace {
    cl::opt<std::string> targetFuncName("fn", cl::init("na"), cl::desc("function name"));
    cl::opt<bool> noACC("noACC", cl::init(false), cl::desc("forbid extraction of ACC-series operators"));
    cl::opt<bool> noMAC("noMAC", cl::init(true), cl::desc("forbid extraction of MAC operators"));
    cl::opt<bool> noPattern("noPattern", cl::init(false), cl::desc("forbid the extraction of memory access pattern"));
    cl::opt<bool> suppRem("suppRem", cl::init(false), cl::desc("support Remainder operator"));
    cl::opt<bool> loopSafety("loopSafety", cl::init(false), cl::desc("enable loop safety controls, otherwise all loops are accessible by default"));
    cl::opt<bool> CDFGhelp("CDFGhelp", cl::init(false), cl::desc("print help of our CDFG pass"));
    // cl::opt<std::string> cdfgType("type", cl::init("PartPred"), cl::desc("cdfg type, valid types = PartPred, Trig, TrMap, BrMap, DFGDISE"));
	bool helpPrinted = false;

    struct CDFGPass : public FunctionPass {
        static char ID;
	    CDFGPass() : FunctionPass(ID){}
        // function annotation: size attribute
        std::map<std::string, int> SizeAttrMap;
        // basic block pairs that there is a back edge in between, <srcBB, dstBB>
        SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> BackEdgeBBPs;    
        // the recursively successive basic blocks except the back-edge BBs    
        std::map<const BasicBlock *, std::set<const BasicBlock *>> SuccBBsMap;
        // BBs in the target loop
        std::set<const BasicBlock *> LoopBBs;
        // mapping unit to CGRA/FGRA: loop, <loop-name, MappingUnit>
        std::map<std::string, MappingUnit> MappingUnitMap;
        std::vector<std::string> MappingUnitQueue; //Arrange the loops in serial execution order
        // BB to the corresponding loop (MappingUnit)
        // std::map<BasicBlock *, std::string> BB2MUnitMap;
		
		//a set of nested-loops
		std::map<int, Loop*> nestloops;

        virtual void getAnalysisUsage(AnalysisUsage &AU) const {	    
	    	AU.setPreservesAll();
	    	AU.addRequired<LoopInfoWrapperPass>();
	    	AU.addRequired<ScalarEvolutionWrapperPass>();
	    	// AU.addRequired<AAResultsWrapperPass>();
	    	AU.addRequired<DominatorTreeWrapperPass>();
	    	// AU.addRequired<PostDominatorTreeWrapperPass>();
	    	AU.addRequired<DependenceAnalysisWrapperPass>();
	    	AU.addRequiredID(LoopSimplifyID);
	    	AU.addRequiredID(LCSSAID);
	    	AU.addPreserved<DominatorTreeWrapperPass>();
	    	// AU.addPreserved<PostDominatorTreeWrapperPass>();
	    }

	    virtual bool runOnFunction(Function &F) {
			if(CDFGhelp && !helpPrinted){
				errs() << "\t--noACC\t\t forbid extraction of ACC-series operators\n";
				errs() << "\t--noMAC\t\t forbid extraction of MAC operators\n";
				errs() << "\t--noPattern\t forbid the extraction of memory access pattern\n";
				errs() << "\t--suppRem\t support Remainder operator\n";
    			errs() << "\t--loopSafety\t enable loop safety controls, otherwise all loops are accessible by default";
				errs() << "\t--CDFGhelp\t print help of our CDFG pass\n";
				helpPrinted = true;
                return false;
			}else if(CDFGhelp && helpPrinted){
                return false;
			}
            std::string funcName = F.getName().str();
            if((targetFuncName != "na") && (targetFuncName != funcName)){        
				//errs() << funcName <<"\n\n\n\n";        
                return false;
            }
            std::string cfgName = funcName + "_cfg.dot";
            std::error_code EC;
            raw_fd_ostream File(cfgName, EC, sys::fs::OF_Text);
            if(!EC){
                WriteGraph(File, (const Function *)&F);
            }else{
                errs() << "Cannot open cfg file for WriteGraph!\n";
            }
			File.close();
			ParseHardwareImplementFunction(F); //jhlou add
            ParseSizeAttr(F, SizeAttrMap);
			PostDominatorTree* PostDT=new PostDominatorTree();
			PostDT->recalculate(F);
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
		    ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
		    DependenceInfo *DI = &getAnalysis<DependenceAnalysisWrapperPass>().getDI();
		    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
		    // PostDominatorTree *PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
		    PostDominatorTree *PDT = PostDT;
		    const DataLayout &DL = F.getParent()->getDataLayout();

            // find the back edges between BBs
            FindFunctionBackedges(F, BackEdgeBBPs);
            // get SuccBBsMap
            getSuccBBsMap(F, BackEdgeBBPs, SuccBBsMap);
            printSuccBBsMap(SuccBBsMap);
            // get inner-most loops
            std::vector<Loop *> loops;
            // std::vector<Loop *> innerMostLoops;
            std::map<Loop *, std::string> loopNames;
            for(auto iter = LI.begin(); iter != LI.end(); iter++){
                loops.push_back(*iter);
            }
            getInnerMostLoops(MappingUnitMap, MappingUnitQueue, loopNames, loops, "LN");
            printMappingUnitMap(MappingUnitMap);
			outs() << ">>>>>> get Function input arguments <<<<<<\n";
			std::map<int, Value*> funcArgs;
			for(auto iter = F.arg_begin(); iter != F.arg_end(); iter++){
				iter->dump();
				funcArgs[iter->getArgNo()] = &*iter;
			}
			///TODO: to support multi-loops in the same level
			std::map<Loop*, std::vector<Instruction*>> outMarkLoopIns;
			std::vector<BasicBlock*> TaskIOBBs;
            auto LoopVec = getMappingUnitNameUsingLoopMark(F, MappingUnitMap, MappingUnitQueue, SuccBBsMap, outMarkLoopIns, TaskIOBBs);//check LoopMark first
			if(LoopVec.empty()){
				std::string munitName = getMappingUnitNameUsingPleasemapme(F, MappingUnitMap);//then check please_map_me()
				Loop *tem = MappingUnitMap[munitName].lp;
				int i = 0;
				nestloops.clear();
				while (tem != nullptr)
				{
					nestloops[i] = tem;
					i++;
					tem = tem->getParentLoop();
				}
				outs() << ">>>>>> Print the whole flattened DFG <<<<<<\n";
				Loop *outtermostLoop = nestloops[i - 1];
				MappingUnit OutterMostUnit;
				OutterMostUnit.lp = outtermostLoop;
				OutterMostUnit.allBlocks.insert(outtermostLoop->block_begin(), outtermostLoop->block_end());
				std::vector<Loop *> loopStack;
				loopStack.push_back(outtermostLoop);
				while (loopStack.size() != 0)
				{
					Loop *loop = loopStack.back();
					loopStack.pop_back();
					OutterMostUnit.entryBlocks.insert(std::make_pair(loop->getLoopPredecessor(), loop->getHeader()));
					SmallVector<std::pair<BasicBlock *, BasicBlock *>, 8> exitEdges;
					loop->getExitEdges(exitEdges);
					OutterMostUnit.exitBlocks.insert(exitEdges.begin(), exitEdges.end());
					loopStack.insert(loopStack.end(), loop->getSubLoops().begin(), loop->getSubLoops().end());
				}
				std::map<std::string, MappingUnit> temmap;
				temmap["the whole DFG"] = OutterMostUnit;
				printMappingUnitMap(temmap);
				LLVMCDFG *CDFG = new LLVMCDFG(munitName);
				CDFG->SE = SE;
				CDFG->DT = DT;
				CDFG->PDT = PDT;
				CDFG->DI = DI;
				CDFG->DL = &DL;
				
				CDFG->_noACC = noACC ? true : false;
				CDFG->_noMAC = noMAC ? true : false;
				CDFG->_noPattern = noPattern ? true : false;
				CDFG->_suppRem = suppRem ? true : false;
				CDFG->_loopSafety = loopSafety ? true : false;

				CDFG->setBackEdgeBBPs(BackEdgeBBPs);
				CDFG->setSuccBBMap(SuccBBsMap);
				CDFG->setLoopBBs(OutterMostUnit.allBlocks, OutterMostUnit.entryBlocks, OutterMostUnit.exitBlocks);
				CDFG->setLoops(nestloops);
				CDFG->setFuncArgs(funcArgs);
				// CDFG->LoopIdxAnalyze();
				CDFG->initialize();
				CDFG->printDOT("DFG_ori.dot");
				// CDFG->accessAnalyze();
				outs() << "########################################################\n";
				outs() << "Generate CDFG Started\n";
				CDFG->printHierarchyDOT("hierarchyDOT.dot");
				CDFG->generateCDFG();
			}
			else
			{
				std::map<int, std::pair<std::set<LLVMCDFGNode*>, std::set<LLVMCDFGNode*>>> LSTable;
				std::map<int, LLVMCDFG*> CDFGs;
				int kernel = 0;
				while (!LoopVec.empty())
				{	
					nestloops = LoopVec.back();//To obey the kernels' order of execution
					LoopVec.pop_back();
					outs() << ">>>>>> Print the whole flattened DFG <<<<<<\n";
					Loop *outtermostLoop = nestloops[nestloops.size()-1];
					MappingUnit OutterMostUnit;
					OutterMostUnit.lp = outtermostLoop;
					OutterMostUnit.allBlocks.insert(outtermostLoop->block_begin(), outtermostLoop->block_end());
					std::vector<Loop *> loopStack;
					loopStack.push_back(outtermostLoop);
					while (loopStack.size() != 0)
					{
						Loop *loop = loopStack.back();
						loopStack.pop_back();
						OutterMostUnit.entryBlocks.insert(std::make_pair(loop->getLoopPredecessor(), loop->getHeader()));
						SmallVector<std::pair<BasicBlock *, BasicBlock *>, 8> exitEdges;
						loop->getExitEdges(exitEdges);
						OutterMostUnit.exitBlocks.insert(exitEdges.begin(), exitEdges.end());
						///TOFIX: here include all subloops
						loopStack.insert(loopStack.end(), loop->getSubLoops().begin(), loop->getSubLoops().end());
					}
					errs() << "[][][]entryBlocks:\n";
					for(auto iter:OutterMostUnit.entryBlocks){
						errs() << iter.first->getName() <<"->>"<< iter.second->getName()<<";";
					}
					errs() << "\n[][][]entryBlocks\n";
					std::map<std::string, MappingUnit> temmap;
					temmap["the whole DFG"] = OutterMostUnit;
					printMappingUnitMap(temmap);
					LLVMCDFG *CDFG = new LLVMCDFG("kernel_"+std::to_string(kernel));
					CDFG->SE = SE;
					CDFG->DT = DT;
					CDFG->PDT = PDT;
					CDFG->DI = DI;
					CDFG->DL = &DL;

					CDFG->_noACC = noACC ? true : false;
					CDFG->_noMAC = noMAC ? true : false;
					CDFG->_noPattern = noPattern ? true : false;
					CDFG->_suppRem = suppRem ? true : false;
					CDFG->_loopSafety = loopSafety ? true : false;

					CDFG->setBackEdgeBBPs(BackEdgeBBPs);
					CDFG->setSuccBBMap(SuccBBsMap);
					CDFG->setLoopBBs(OutterMostUnit.allBlocks, OutterMostUnit.entryBlocks, OutterMostUnit.exitBlocks, TaskIOBBs);
					CDFG->setLoops(nestloops);
					CDFG->setFuncArgs(funcArgs);
					CDFG->setExInsList(outMarkLoopIns[outtermostLoop]);
					// CDFG->LoopIdxAnalyze();
					CDFG->initialize();
					// CDFG->accessAnalyze();
					CDFG->printHierarchyDOT("hierarchyDOT_"+std::to_string(kernel)+".dot");
					CDFG->generateCDFG();
					//extractLSInfo(CDFG, kernel, &LSTable);
					CDFGs[kernel] = CDFG;
					kernel++;
				}
				if(CDFGs.size()>1){
					outs() << "########################################################\n";
					outs() << "Tasks Schedule Started\n";
					scheduleTasks(CDFGs, LSTable);
					outs() << "Tasks Schedule Ended\n";
					outs() << "########################################################\n";
				}
			}
            return true;

        } // end runOnFunction
    };
} // end namespace


char CDFGPass::ID = 0;
static RegisterPass<CDFGPass> X("cdfg", "CDFGPass", false, false);
