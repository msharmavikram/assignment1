/*
 * Copyright (c) 2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Anthony Gutierrez
 */

/********************* COPIED FROM BI-MODE *********************/

#include "cpu/pred/perceptron.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "debug/perceptron.hh"

Perceptron::Perceptron(const PerceptronParams *params)
    : BPredUnit(params),
      globalHistoryReg(params->numThreads, 0),
      globalHistoryBits(ceilLog2(params->globalPredictorSize)),
      globalPredictorSize(params->globalPredictorSize),
      globalCtrBits(params->globalCtrBits)
{
    if (!isPowerOf2(globalPredictorSize))
        fatal("Invalid global history predictor size.\n");

    historyRegisterMask = mask(globalHistoryBits);
    globalHistoryMask = globalPredictorSize - 1;

}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
Perceptron::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->finalPred = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
}

void
Perceptron::squash(ThreadID tid, void *bpHistory)
{
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;

    delete history;
}

unsigned
Perceptron::getGHR(ThreadID tid, void *bp_history) const
{
    return static_cast<BPHistory*>(bp_history)->globalHistoryReg;
}

void
Perceptron::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;
}

Perceptron*
PerceptronParams::create()
{
    return new Perceptron(this);
}

void
Perceptron::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    globalHistoryReg[tid] &= (historyRegisterMask & ~ULL(1));
}

bool
Perceptron::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    //hashing function
    unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
                                ^ globalHistoryReg[tid])
                                & globalHistoryMask);

    assert(globalHistoryIdx < globalPredictorSize);

    bool finalPrediction;

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];

    int y = ptable[globalHistoryIdx];  
    for (int i =0; i< globalHistoryBits;++i){
        int j = updateHIdx(i);
        y += weights[globalHistoryIdx][i] * hist[j];  
    }
    
    if(y>0) 
       finalPrediction = 1;  
    else 
       finalPrediction = -1;

    history->finalPred = finalPrediction;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, finalPrediction);
 
    return finalPrediction;
}

void
Perceptron::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed)
{
    assert(bpHistory);

    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    signed int t=1;
    if(taken) 
       t = 1;
    else 
       t = -1;

    // We do not update the counters speculatively on a squash.
    // We just restore the global history register.
    if (squashed) {
        globalHistoryReg[tid] = (history->globalHistoryReg << 1) | t;
        return;
    }
    //hashing function
    unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
                                ^ history->globalHistoryReg)
                                & globalHistoryMask);

    assert(globalHistoryIdx < globalPredictorSize);
    
    //Implement perceptorn inference for calcualting training values again -
    int y = ptable[globalHistoryIdx];  
    for (int i =0; i< globalHistoryBits;++i){
        int j = updateHIdx(i);
        y += weights[globalHistoryIdx][i] * hist[j];  
    }
    
    //Implement perceptorn training logic here -
    if((history->finalPred  != t) || abs(y)<=theta){
      //Update the weights. 
       for(int i=0; i< globalHistoryBits;++i){
        int j = updateHIdx(i);
        //unsigned j = (i+start_history_index)% globalHistoryBits;
        int temp_w = weights[globalHistoryIdx][i]+ t * hist[j]; //global history index points to the entry in the ptable.
        weights[globalHistoryIdx][i] =  temp_w;  
       } 
       //Update the ptable ith entry. 
       int temp_ptable_value = ptable[globalHistoryIdx]+ t;
       ptable[globalHistoryIdx] = temp_ptable_value;
    }

    //update circular buffer here itself.
    hist[start_history_index] = t;
    start_history_index = updateHIdx(1); 
    delete history;
}

