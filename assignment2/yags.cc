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

// 
// Implementation of a YAGS branch predictor
//

#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "cpu/pred/yags.hh"


//All the paremeter values are taken from build/ARM/ folder. This is not from the bi_mode.hh. 
// Constructor for YagsBP
YagsBP::YagsBP(const Params *params)
    : BPredUnit(params), instShiftAmt(params->instShiftAmt),
      globalHistoryReg(params->numThreads,0),//FIXME - support no of threass.
      globalHistoryBits(ceilLog2(params->globalPredictorSize)),
      choicePredictorSize(params->choicePredictorSize),
      choiceCtrBits(params->choiceCtrBits),
      globalPredictorSize(params->globalPredictorSize ),
      globalCtrBits(params->globalCtrBits)
{
    if(!isPowerOf2(choicePredictorSize))
        fatal("Invalid choice predictor size!\n");
    if(!isPowerOf2(globalPredictorSize))
        fatal("Invalid global predictor size!\n");

    //implement bimode 
    //size of these counters  = global predictor size.
    choiceCounters.resize(choicePredictorSize);
    takenCounters.resize(globalPredictorSize);
    notTakenCounters.resize(globalPredictorSize);

    //initilize the counter's values
    // Adding tags and used flags for init. 
    initCache();
    
    //set up the mask for indexing from branch address
    globalHistoryMask       = mask(globalHistoryBits);
    choiceHistoryMask       = choicePredictorSize - 1;
    globalHistoryMask       = globalPredictorSize - 1;
    // unsed mask will be used to generate tags. 
    globalHistoryUnusedMask = globalHistoryMask - globalHistoryMask;
    
    printf("globalHistoryBits is %u\n",globalHistoryBits);
    printf("globalHistoryMask is %08x\n",globalHistoryMask);
    printf("globalHistoryMask is %08x\n",globalHistoryMask);
    printf("globalHistoryUnusedMask is %08x\n",globalHistoryUnusedMask);
    //set up the threshold for branch prediction
    choiceThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
    globalPredictorThreshold = (ULL(1) << (globalCtrBits - 1)) - 1;
    
    //Threshold are used to dyanmically find what is the latest tag and what is not. 
    //takenThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
    //notTakenThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
    //using 8 bits of address as tags.
    tagsMask = mask(YAGS_TAG_LEN);
    printf("YagsBP() Constructor done\n");
}

// Actions for an unconditional branch. Nothing to be done. 
//
// For an unconditional branch we set its history such that
// everything is set to taken. I.e., its choice predictor
// chooses the taken array and the taken array predicts taken.
//
void
YagsBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->takenUsed = true;
    history->notTakenPred = true;
    history->takenPred = true;
    history->finalPrediction = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid,true);
}

// Actions for an squash . Nothing to be done. 
void
BiModeBP::squash(ThreadID tid, void *bpHistory)
{
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;

    delete history;
}

//  Lookup the actual branch prediction.
bool
YagsBP::lookup(ThreadID tid,Addr branchAddr, void * &bpHistory)
{
       bool finalPrediction = true;//Default branch will take. 
       BPHistory *history = new BPHistory;
    
        //indexing into either takenPredictor or notTakenPredictor
       unsigned choiceCountersIdx = ((branchAddr >> instShiftAmt) & choiceHistoryMask);
       unsigned globalPredictorIdx = ((branchAddr >> instShiftAmt) ^ globalHistoryReg[tid]) & globalHistoryMask;

       assert(choiceCountersIdx < choicePredictorSize);
       assert(globalPredictorIdx < globalPredictorSize);

        //create tag with tag mask and whatever was remaining in global history register - use unusedmask for the same. Concept - Cache tag creation     
        uint32_t tag = ((branchAddr >> instShiftAmt) & tagsMask) | ((globalHistoryReg & globalHistoryUnusedMask) );
      //Create history register to know what was previous state. This is needed for update phase to figure out which actions to take 
        history->globalHistoryReg = globalHistoryReg[tid];
       
       bool choicePrediction = choiceCounters[choiceCountersIdx].read() > choiceThreshold;
       //There are 4 different cases that can be derivied out of to history takenUsed or not used. This information is cruicial to find what to update.
        // The four cases are taken cache hit or miss and not taken cache hit or miss. 
        // Depending on each scenario - 
        // Say if taken cache is hit then takenPred is finalPred. If its a miss then finalPred is choicePred. 
        // Similarly for nottaken cache is hit then nottakenPred is finalPred. If its a miss then finalPred is choicePred.
        // However there are only three possible outcomes for history - choice is taken or not. 
        // If choice is not taken then did it take from takecache or not takencache.
        // THis is the job of history->TakenUsed flag
        if(choicePrediction){
           //the choice predict taken, try to look into notTaken predictor/cache -- YAGS 3.3 para. 
           //If the tag generated is found in the the cache, then its a hit. Else it is miss. 
           //If its a hit then pass taken after qualifying it on threshold. 
           //Threshold here used is same as global threshold because size we have mapped to global size  
           if(checkTakenCache(globalPredictorIdx,tag,&finalPrediction)){ 
               history->takenPred = finalPrediction;
               history->takenUsed = 1;
           }
           else{
               finalPrediction    = choicePrediction;
               history->takenUsed = 0;
           }
       }
       else{
           //the choice predict not taken, try to look into Taken predictor/cache --YAGS 3.3 para
           //If the tag generated is found in the the cache, then its a hit. Else it is miss. 
           //If its a hit then pass taken after qualifying it on threshold. 
           //Threshold here used is same as global threshold because size we have mapped to global size  
           if(checkNotTakenCache(globalPredictorIdx,tag,&finalPrediction)){
               history->notTakenPred = finalPrediction;
               history->takenUsed    = 2;
           }
           else{
               finalPrediction       = choicePrediction;
               history->takenUsed    = 0;
           }
       }
   //Load the updated finalPreduction  
   history->finalPrediction = finalPrediction;
   bpHistory = static_cast<void*>(history);
   updateGlobalHistReg(tid, finalPrediction);
   return finalPrediction;
}

// BTB Update actions
void
YagsBP::btbUpdate(Addr branchAddr, void * &bpHistory)
{
    globalHistoryReg &= (globalHistoryMask & ~ULL(1));
}

// Update data structures after getting actual decison 
void
YagsBP::update(Addr branchAddr, bool taken, void *bpHistory, bool squashed)
{
    assert(bpHistory);
    
    BPHistory *history = static_cast<BPHistory *>(bpHistory);
    if(squashed){
       globalHistoryReg = (history->globalHistoryReg << 1) | taken;
       return;
    }
    //indexing into either takenPredictor or notTakenPredictor
    unsigned choiceCountersIdx = ((branchAddr >> instShiftAmt) & choiceHistoryMask);
    unsigned globalPredictorIdx = ((branchAddr >> instShiftAmt) ^ history->globalHistoryReg) & globalHistoryMask;
    
    assert(choiceCountersIdx < choicePredictorSize);
    assert(globalPredictorIdx < globalPredictorSize);
    //create tag with tag mask and whatever was remaining in global history register - use unusedmask for the same. Concept - Cache tag creation     
    uint32_t tag = ((branchAddr >> instShiftAmt) & tagsMask) | ((globalHistoryReg & globalHistoryUnusedMask) );
    
    switch(history->takenUsed)
    {
        case 0:
            //the choice predictor was used
            if(history->finalPrediction == taken)
            {
                //the case that the prediction is correct
                if(taken == true)
                    choiceCounters[choiceCountersIdx].increment();
                else
                    choiceCounters[choiceCountersIdx].decrement();
            }
            else if(history->finalPrediction == false && taken == true)
            {
                //update the taken predictor(cache)
                updateTakenCache(globalPredictorIdx,tag,taken);
                choiceCounters[choiceCountersIdx].increment();
            }
            else if(history->finalPrediction == true && taken == false)
            {
                //update the not taken predictor(cache)
                updateNotTakenCache(globalPredictorIdx,tag,taken);
                choiceCounters[choiceCountersIdx].decrement();
            }
            break;
        case 1:// Taken cache used. Choice says - check not taken.  
            if((history->takenPred == taken) && (!taken) == false)
                 //Keep the choice counter same. Dont change in this scenario.However update the taken cache.  
            else{
                if(taken)
                    choiceCounters[choiceCountersIdx].increment();
                else
                    choiceCounters[choiceCountersIdx].decrement();
            }
            updateTakenCache(globalPredictorIdx,tag,taken);
            break;
        case 2:// Taken cache used. Choice says - check not taken.  
            if((history->notTakenPred==taken) && (!taken) == true)
                 //Keep the choice counter same. Dont change in this scenario.However update the not taken cache.  
            else{
                if(taken)
                    choiceCounters[choiceCountersIdx].increment();
                else
                    choiceCounters[choiceCountersIdx].decrement();
            }
            updateNotTakenCache(globalPredictorIdx,tag,taken);
            break;
    }

    delete history;
}

//  Global History Registor Update 
BiModeBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;
}

bool YagsBP::checkTakenCache(const unsigned idx,const uint32_t tag, bool *taken)
{
//If the tag generated is found in the the cache, then its a hit. Else it is miss. 
//If its a hit then pass taken after qualifying it on threshold. Threshold here used is same as global threshold because size we have mapped to global size  
  if(takenCounters[idx].tag == tag)
  {
    *taken = takenCounters[idx].ctr.read() > globalPredictorThreshold;
    return true;
  }
  return false;
}

bool YagsBP::checkNotTakenCache(const unsigned idx,const uint32_t tag,bool *taken)
{
//If the tag generated is found in the the cache, then its a hit. Else it is miss. 
//If its a hit then pass taken after qualifying it on threshold. Threshold here used is same as global threshold because size we have mapped to global size  
  if(notTakenCounters[idx].tag == tag)
  {
    *taken = notTakenCounters[idx].ctr.read() > globalPredictorThreshold;
    return true;
  }
  return false;
}

void YagsBP::updateTakenCache(const unsigned idx, const uint32_t tag,const bool taken)
{
//If the tag is present in the cache then increase the counter. If its not present then decrement tha counter.
  if(takenCounters[idx].tag == tag)
  {
    if(taken)
      takenCounters[idx].ctr.increment();
    else
      takenCounters[idx].ctr.decrement();
  }
}

void YagsBP::updateNotTakenCache(const unsigned idx, const uint32_t tag,const bool taken)
{
//If the tag is present in the cache then increase the counter. If its not present then decrement tha counter.
    if(notTakenCounters[idx].tag== tag)
    {
      if(taken)
        notTakenCounters[idx].ctr.increment();
      else
        notTakenCounters[idx].ctr.decrement();
    }
}
//Initialise caches
void YagsBP::initCache()
{
    printf("Initilizing choice/taken/notTaken counters with choice %u as and taken/nottaken as %u 1s\n",choiceCtrBits, globalCtrBits);
    for(uint32_t count = 0; count < choicePredictorSize; count++)
    {
        choiceCounters[count].setBits(choiceCtrBits);
    }
    for(uint32_t count = 0; count < globalPredictorSize; count++)
    {
        takenCounters[count].ctr.setBits(globalCtrBits);
        notTakenCounters[count].ctr.setBits(globalCtrBits);
        takenCounters[count].tag     = 0;
        notTakenCounters[count].tag  = 0;
    }
    printf("Cache initilization complete\n");
}
