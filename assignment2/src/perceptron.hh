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



// This is an implementation of perceptron branch predictor. 



/********************* COPIED FROM BI-MODE *********************/

#ifndef __CPU_PRED_PERCEPTRON_HH__
#define __CPU_PRED_PERCEPTRON_HH__

#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/Perceptron.hh"

class Perceptron : public BPredUnit
{
  public:
    Perceptron(const PerceptronParams *params);
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
    void squash(ThreadID tid, void *bp_history);
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
    unsigned getGHR(ThreadID tid, void *bp_history) const;
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);//gets the prediction. 
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed);//does the training

  private:
    void updateGlobalHistReg(ThreadID tid, bool taken);

    struct BPHistory {
        unsigned globalHistoryReg;
        signed finalPred;
    };

    std::vector<unsigned> globalHistoryReg;
    unsigned globalHistoryBits;//globalhistory register size. 

    unsigned historyRegisterMask;
    unsigned globalPredictorSize;
    unsigned globalCtrBits;
    unsigned globalHistoryMask;
    unsigned choicePredictorSize;
    unsigned choiceCtrBits;
    unsigned choiceHistoryMask;
    static const int nperceptrons =14 ;// currently the global predictor size is 8k
    unsigned theta = (1.93*globalHistoryBits+14);//this number is from paper. 
    unsigned weightsize = 8;//in bits. 1 for sign and 7 for value. Assignment needs 6 bits. Taken 7 as of now. May reduce later. FIXME
    unsigned start_history_index = 0;
    
    int ptable[nperceptrons] = {1};// only first w_0 is assigned to 1 as per paper
    //int weights[nperceptrons][globalHistoryBits] = {{0}};
    std::vector<std::vector<signed>> weights = {{0}};
     
    //Creating an array of history as the gem5 flexibility to create is tedious. Hence creating a circular buffer for history 
    //int hist[globalPredictorSize] = {-1};     
    std::vector<signed> hist = {-1};     
 
    inline int updateHIdx(int i) const {
      return (i + start_history_index) % globalHistoryBits;
    }
};

#endif // __CPU_PRED_PERCEPTRON__
