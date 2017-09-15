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

#ifndef __CPU_PRED_YAGS_PRED_HH__
#define __CPU_PRED_YAGS_PRED_HH__

#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/YagsBP.hh" // This file does not exit. 

/*
 * Feel free to make any modifications, this is a skeleton code
 * to get you started.
 * Note: Do not change name of class
 */


 #define YAGS_TAG_LEN 8

class YagsBP : public BPredUnit
{
  public:
    YagsBP(const YagsBPParams *params);
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
    void squash(ThreadID tid, void *bp_history);
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed);
    unsigned getGHR(ThreadID tid, void *bp_history) const;
  private:
    void updateGlobalHistReg(ThreadID tid, bool taken);

    //init cache
    void initCache();

    //return true means hit, false means miss
    bool checkTakenCache(const unsigned idx,const uint32_t tag, bool *taken);
    bool checkNotTakenCache(const unsigned idx,const uint32_t tag, bool *taken);
    
    void updateTakenCache(const unsigned idx, const uint32_t tag,const bool taken);
    void updateNotTakenCache(const unsigned idx, const uint32_t tag,const bool taken);

    struct BPHistory {
        unsigned globalHistoryReg;
        // was the taken array's prediction used?
        // 0: choice Predictor used
        // 1: takenPred used
        // 2: notPred used
        uint8_t takenUsed;
        // prediction of the taken array
        // true: predict taken
        // false: predict not-taken
        bool takenPred;
        // prediction of the not-taken array
        // true: predict taken
        // false: predict not-taken
        bool notTakenPred;
        // the final taken/not-taken prediction
        // true: predict taken
        // false: predict not-taken
        bool finalPrediction;
    };

    struct LocalCache
    {
        SatCounter ctr;
        uint32_t tag;
    };

    // choice predictors
    std::vector<SatCounter> choiceCounters;
    // taken direction predictors
    std::vector<LocalCache> takenCounters;
    // not-taken direction predictors
    std::vector<LocalCache> notTakenCounters;

    std::vector<unsigned> globalHistoryReg;
    unsigned globalHistoryBits;
    unsigned globalHistoryUnusedMask;

    unsigned choicePredictorSize;
    unsigned choiceCtrBits;
    unsigned choiceHistoryMask;
    unsigned globalPredictorSize;
    unsigned globalCtrBits;
    unsigned globalHistoryMask;

    unsigned historyRegisterMask;
    unsigned choiceThreshold;
    unsigned globalPredictorThreshold;

    unsigned tagsMask;
};

#endif // __CPU_PRED_YAGS_PRED_HH__
