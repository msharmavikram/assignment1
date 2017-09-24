/*
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
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
 * Authors: Kevin Lim
 */

#include "cpu/pred/gshare.hh"

#include "base/intmath.hh"
#include "base/misc.hh"
#include "base/trace.hh"
#include "debug/Fetch.hh"

Gshare::Gshare(const GshareParams *params)
    : BPredUnit(params),
      globalHistoryReg(params->numThreads, 0),
      localPredictorSize(params->localPredictorSize),
      localCtrBits(params->localCtrBits)
{
    if (!isPowerOf2(localPredictorSize)) {
        fatal("Invalid local predictor size!\n");
    }

    localPredictorSets = localPredictorSize / localCtrBits;

    if (!isPowerOf2(localPredictorSets)) {
        fatal("Invalid number of local predictor sets! Check localCtrBits.\n");
    }

    // Setup the index mask.
    //indexMask = localPredictorSets - 1;
    indexMask = localPredictorSize - 1;

    DPRINTF(Fetch, "index mask: %#x \n", indexMask);

    // Setup the array of counters for the local predictor.
    localCtrs.resize(localPredictorSize);
    unsigned localPredictorBits =  ceilLog2(localPredictorSize);
    for (unsigned i = 0; i < localPredictorBits; ++i)
        localCtrs[i].setBits(localCtrBits);

    historyRegisterMask = mask(localCtrBits);
    DPRINTF(Fetch, "local predictor size: %i\n",
            localPredictorSize);

    DPRINTF(Fetch, "local counter bits: %i\n", localCtrBits);

    DPRINTF(Fetch, "instruction shift amount: %i\n",
            instShiftAmt);
}

void
Gshare::reset()
{
    for (unsigned i = 0; i < localPredictorSets; ++i) {
        localCtrs[i].reset();
    }
}

void
Gshare::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
// Place holder for a function that is called to update predictor history when
// a BTB entry is invalid or not found.
    globalHistoryReg[tid] &= (historyRegisterMask & ~ULL(1));
}


bool
Gshare::lookup(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    bool taken;
    uint8_t counter_val;
    unsigned local_predictor_idx = getLocalIndex(tid, branch_addr);

    //BPHistory *history = new BPHistory;
    //history->globalHistoryReg = globalHistoryReg[tid];
    oldhistory = globalHistoryReg[tid];
    DPRINTF(Fetch, "Looking up index %#x\n",
            local_predictor_idx);

    counter_val = localCtrs[local_predictor_idx].read();

    DPRINTF(Fetch, "prediction is %i.\n",
            (int)counter_val);

    taken = getPrediction(counter_val);

#if 0
    // Speculative update.
    if (taken) {
        DPRINTF(Fetch, "Branch updated as taken.\n");
        localCtrs[local_predictor_idx].increment();
    } else {
        DPRINTF(Fetch, "Branch updated as not taken.\n");
        localCtrs[local_predictor_idx].decrement();
    }
#endif

    return taken;
}

void
Gshare::update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed)
{
    assert(bp_history == NULL);
    unsigned local_predictor_idx;

    //BPHistory *history = static_cast<BPHistory*>(bp_history);
    // No state to restore, and we do not update on the wrong
    // path.
    if (squashed) {
        //globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        globalHistoryReg[tid] = (oldhistory << 1) | taken;
        return;
    }

    // Update the local predictor.
    local_predictor_idx = getLocalIndex(tid, branch_addr);

    DPRINTF(Fetch, "Looking up index %#x\n", local_predictor_idx);

    if (taken) {
        DPRINTF(Fetch, "Branch updated as taken.\n");
        localCtrs[local_predictor_idx].increment();
    } else {
        DPRINTF(Fetch, "Branch updated as not taken.\n");
        localCtrs[local_predictor_idx].decrement();
    }
}

inline
bool
Gshare::getPrediction(uint8_t &count)
{
    // Get the MSB of the count
    return (count >> (localCtrBits - 1));
}

inline
unsigned
Gshare::getLocalIndex(ThreadID tid, Addr &branch_addr)
{
    //return ((branch_addr ) ^ globalHistoryReg[tid]) & historyRegisterMask;
    return ((branch_addr ) ^ globalHistoryReg[tid]) & indexMask;
    //return ((branch_addr >> instShiftAmt) ^ globalHistoryReg[tid]) & indexMask;
}

void
Gshare::uncondBranch(ThreadID tid, Addr pc, void *&bp_history)
{
    //BPHistory *history = new BPHistory;
    //history->globalHistoryReg = globalHistoryReg[tid];
    oldhistory = globalHistoryReg[tid];
    updateGlobalHistReg(tid, true);
}

Gshare*
GshareParams::create()
{
    return new Gshare(this);
}
void
Gshare::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;
}
void
Gshare::squash(ThreadID tid, void *bp_history)
{
    BPHistory *history = static_cast<BPHistory*>(bp_history);
//    globalHistoryReg[tid] = history->globalHistoryReg;
    globalHistoryReg[tid] = oldhistory;

    delete history;
    oldhistory =0;
}

