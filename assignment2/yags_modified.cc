/* @file
 * Implementation of a YAGS branch predictor
 *
 * 18-640 Foundations of Computer Architecture
 * Carnegie Mellon University
 *
 */

#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "cpu/pred/yags.hh"


/*
 * Constructor for YagsBP
 */
YagsBP::YagsBP(const Params *params)
    : BPredUnit(params), instShiftAmt(params->instShiftAmt),
      globalHistoryReg(0),
      globalHistoryBits(ceilLog2(params->globalPredictorSize)),
      choicePredictorSize(params->choicePredictorSize),
      choiceCtrBits(params->choiceCtrBits),
      globalPredictorSize(params->globalPredictorSize ),
      globalCtrBits(params->globalCtrBits)
{
	//judging the predictor size
    if(!isPowerOf2(globalPredictorSize))
    	fatal("Invalid global predictor size!\n");
    if(!isPowerOf2(choicePredictorSize))
    	fatal("Invalid choice predictor size!\n");

    //set up the tables of counters and Tags
    //taken and notTaken predictor should share the same size as globalPredictorSize
    choiceCounters.resize(choicePredictorSize);
    takenCounters.resize(globalPredictorSize);
    notTakenCounters.resize(globalPredictorSize);

    //initilize the counter's values
    printf("Initilizing choiceCounters with %u 1s\n",choiceCtrBits);
    for(uint32_t count = 0; count < choicePredictorSize; count++)
    {
    	choiceCounters[count].setBits(choiceCtrBits);
    }

    initCache();
    //set up the mask for indexing from branch address
    choicePredictorMask = choicePredictorSize - 1;
    globalPredictorMask = globalPredictorSize - 1;
    globalHistoryMask = mask(globalHistoryBits);
    globalHistoryUnusedMask = globalHistoryMask - (globalHistoryMask );
    printf("globalHistoryBits is %u\n",globalHistoryBits);
    printf("globalHistoryMask is %08x\n",globalHistoryMask);
    printf("globalPredictorMask is %08x\n",globalPredictorMask);
    printf("globalHistoryUnusedMask is %08x\n",globalHistoryUnusedMask);
    //set up the threshold for branch prediction
    choiceThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
    globalPredictorThreshold = (ULL(1) << (globalCtrBits - 1)) - 1;

    //using 8 bits of address as tags.
    tagsMask = mask(YAGS_TAG_LEN);
    printf("YagsBP() Constructor done\n");
}

/*
 * Actions for an unconditional branch
 */
void
YagsBP::uncondBranch(void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg;
    history->takenUsed = 0;
    history->notTakenPred = true;
    history->takenPred = true;
    history->finalPred = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(true);
}

/*
 * Actions for squash
 */
void
YagsBP::squash(void *bpHistory)
{
	if(bpHistory)
    {
    	BPHistory *history = static_cast<BPHistory*>(bpHistory);
    	globalHistoryReg = history->globalHistoryReg;
    	delete history;
    }
}

/*
 * Lookup the actual branch prediction.
 */
bool
YagsBP::lookup(Addr branchAddr, void * &bpHistory)
{
	//printf("Performing lookup\n");
	bool choicePred, finalPred = true;
	unsigned choiceCountersIdx = ((branchAddr >> instShiftAmt) & choicePredictorMask);
	//indexing into either takenPredictor or notTakenPredictor
   	unsigned globalPredictorIdx = ((branchAddr >> instShiftAmt) ^ globalHistoryReg) & globalPredictorMask;

   	//printf("%u,%u\n",choiceCountersIdx,globalPredictorIdx);
   	assert(choiceCountersIdx < choicePredictorSize);
   	assert(globalPredictorIdx < globalPredictorSize);

   	uint32_t tag = ((branchAddr >> instShiftAmt) & tagsMask) | ((globalHistoryReg & globalHistoryUnusedMask) );
   	BPHistory *history = new BPHistory;
  	history->globalHistoryReg = globalHistoryReg;
   	//printf("Getting choice prediction\n");
   	choicePred = choiceCounters[choiceCountersIdx].read() > choiceThreshold;
   	if(choicePred)
   	{
   		//the choice predict taken, try to look into notTaken predictor/cache
   		//printf("Getting taken prediction\n");
   		if(lookupTakenCache(globalPredictorIdx,tag,&finalPred))
   		{
   			//printf("USING PREDICTION FROM TAKEN PREDICTOR\n");
   			history->takenPred = finalPred;
   			history->takenUsed = 1;
   		}
   		else
   		{
   			history->takenUsed = 0;
   			finalPred = choicePred;
   		}
   	}
   	else
   	{
   		//the choice predict not taken, try to look into Taken predictor/cache
   		//printf("Getting not taken prediction\n");
   		if(lookupNotTakenCache(globalPredictorIdx,tag,&finalPred))
   		{
   			//printf("USING PREDICTION FROM NOT TAKEN PREDICTOR\n");
   			history->notTakenPred = finalPred;
   			history->takenUsed = 2;
   		}
   		else
   		{
   			history->takenUsed = 0;
   			finalPred = choicePred;
   		}
   	}
   	//printf("Updating global history\n");
   	history->finalPred = finalPred;
   	bpHistory = static_cast<void*>(history);
   	updateGlobalHistReg(finalPred);
    return finalPred;
}

/*
 * BTB Update actions
 */
void
YagsBP::btbUpdate(Addr branchAddr, void * &bpHistory)
{
    globalHistoryReg &= (globalHistoryMask & ~ULL(1));
}

/*
 * Update data structures after getting actual decison 
 */
void
YagsBP::update(Addr branchAddr, bool taken, void *bpHistory, bool squashed)
{
	//printf("Performing update\n");
    if(bpHistory)
    {
    	BPHistory *history = static_cast<BPHistory *>(bpHistory);
    	unsigned choiceCountersIdx = ((branchAddr >> instShiftAmt) & choicePredictorMask);
    	//indexing into either takenPredictor or notTakenPredictor
    	unsigned globalPredictorIdx = ((branchAddr >> instShiftAmt) ^ history->globalHistoryReg) & globalPredictorMask;
   	uint32_t tag = ((branchAddr >> instShiftAmt) & tagsMask) | ((globalHistoryReg & globalHistoryUnusedMask) );
        
        assert(choiceCountersIdx < choicePredictorSize);
        assert(globalPredictorIdx < globalPredictorSize);
    	switch(history->takenUsed)
    	{
    		case 0:
    			//the choice predictor was used
    			if(history->finalPred == taken)
    			{
    				//the case that the prediction is correct
    				if(taken == true)
    					choiceCounters[choiceCountersIdx].increment();
    				else
    					choiceCounters[choiceCountersIdx].decrement();

    			}
    			else if(history->finalPred == false && taken == true)
    			{
    				//update the taken predictor(cache)
                                updateTakenCache(globalPredictorIdx,tag,taken);
    				choiceCounters[choiceCountersIdx].increment();

    			}
    			else if(history->finalPred == true && taken == false)
    			{
    				//update the not taken predictor(cache)
                                updateNotTakenCache(globalPredictorIdx,tag,taken);
    				choiceCounters[choiceCountersIdx].decrement();
    			}
    		break;
    		case 1:
    			//the taken predictor was used, choice predictor indicates not taken
    			if(taken == history->takenPred && (!taken) == false)
    			{
    				
    			}
    			else
    			{
    				if(taken)
    					choiceCounters[choiceCountersIdx].increment();
    				else
    					choiceCounters[choiceCountersIdx].decrement();
    			}
                        updateTakenCache(globalPredictorIdx,tag,taken);
    		break;
    		case 2:
    			//the not taken predictor was used
    			if(taken == history->notTakenPred && (!taken) == true)
    			{
    				
    			}
    			else
    			{
    				if(taken)
    					choiceCounters[choiceCountersIdx].increment();
    				else
    					choiceCounters[choiceCountersIdx].decrement();
    			}
                        updateNotTakenCache(globalPredictorIdx,tag,taken);
    		break;
    	}

    	if(squashed)
    	{
    		if(taken)
    			globalHistoryReg = (history->globalHistoryReg << 1) | 1;
    		else
    			globalHistoryReg = (history->globalHistoryReg << 1);
    		globalHistoryReg &= globalHistoryMask;
    	}
    	else
    		delete history;
    }

}

/*
 * Retire Squashed Instruction
 */
void
YagsBP::retireSquashed(void *bp_history)
{
	if(bp_history)
    {
    	BPHistory *history = static_cast<BPHistory*>(bp_history);
    	delete history;
    }
}

/*
 * Global History Registor Update 
 */
void
YagsBP::updateGlobalHistReg(bool taken)
{
    globalHistoryReg = taken ? globalHistoryReg << 1 | 1 : globalHistoryReg << 1;
    globalHistoryReg = globalHistoryReg & globalHistoryMask;
}

bool YagsBP::lookupTakenCache(const unsigned idx,const uint32_t tag, bool *taken)
{
  bool found = 0;
  //{
    if(takenCounters[idx].tag == tag)
    {
      updateTakenCacheLRU(idx);  
      *taken = takenCounters[idx].ctr.read() > globalPredictorThreshold;
      found = true;
      return true;
    }
  //}

  if(found)
    return true;
  else
    return false;
}

bool YagsBP::lookupNotTakenCache(const unsigned idx,const uint32_t tag,bool *taken)
{

  bool found = 0;
  //{
    if(notTakenCounters[idx].tag == tag)
    {
      updateNotTakenCacheLRU(idx);
      *taken = notTakenCounters[idx].ctr.read() > globalPredictorThreshold;
      found = true;
      return true;
    }
  //}

  if(found)
    return true;
  else
    return false;
}

void YagsBP::updateTakenCache(const unsigned idx, const uint32_t tag,const bool taken)
{
  bool found = false;
  //{
    if(takenCounters[idx].tag == tag)
    {
      updateTakenCacheLRU(idx);
      if(taken)
        takenCounters[idx].ctr.increment();
      else
        takenCounters[idx].ctr.decrement();
      found = true;
    }
  //}
  //if did not find any matching tag
  //replace the least-recently-used one
  
  if(!found)
  {
    uint8_t LRU = takenCounters[idx].LRU;
    takenCounters[idx].tag[LRU] = tag;
    //reset the counter
    takenCounters[idx].ctr[LRU].setBits(globalCtrBits);
    
    if(taken)
      takenCounters[idx].ctr[LRU].increment();
    else
      takenCounters[idx].ctr[LRU].decrement();
    
  }
}

void YagsBP::updateNotTakenCache(const unsigned idx, const uint32_t tag,const bool taken)
{
  bool found = false;
  //{
    if(notTakenCounters[idx].tag== tag)
    {
      updateNotTakenCacheLRU(idx);
      if(taken)
        notTakenCounters[idx].ctr.increment();
      else
        notTakenCounters[idx].ctr.decrement();

      found = true;
    }
  //}
  //if did not find any matching tag
  //replace the least-recently-used one
  if(!found)
  {
    uint8_t LRU = takenCounters[idx].LRU;
    notTakenCounters[idx].tag[LRU] = tag;
    //reset the counter 
    notTakenCounters[idx].ctr[LRU].setBits(globalCtrBits);
    if(taken)
      notTakenCounters[idx].ctr[LRU].increment();
    else
      notTakenCounters[idx].ctr[LRU].decrement();
  }
}

void YagsBP::initCache()
{
    printf("Initilizing taken/notTaken counters with %u 1s\n",globalCtrBits);
    for(uint32_t count = 0; count < globalPredictorSize; count++)
    {
        takenCounters[count].ctr.setBits(globalCtrBits);
        takenCounters[count].tag = 0;
        takenCounters[count].used = 0;
        notTakenCounters[count].ctr.setBits(globalCtrBits);
        notTakenCounters[count].tag = 0;
        notTakenCounters[count].used = 0;
      //FIXME
      takenCounters[count].LRU = 0;
      notTakenCounters[count].LRU = 0;
    }
    printf("Cache initilization done\n");
}

void YagsBP::updateTakenCacheLRU(const unsigned idx, const uint8_t entry_idx)
{
  uint8_t threshold_used = takenCounters[idx].used[entry_idx];
  takenCounters[idx].used[entry_idx] = 0;
  if(takenCounters[idx].used> threshold_used && count != entry_idx) //FIXME
    takenCounters[idx].used--;
  if(takenCounters[idx].used == 0)
    takenCounters[idx].LRU =0;
}

void YagsBP::updateNotTakenCacheLRU(const unsigned idx, const uint8_t entry_idx)
{
  uint8_t threshold_used = notTakenCounters[idx].used[entry_idx];
  notTakenCounters[idx].used[entry_idx] = 0;
  if(notTakenCounters[idx].used > threshold_used && count != entry_idx) //FIXME
    notTakenCounters[idx].used--;
  if(notTakenCounters[idx].used == 0)
    notTakenCounters[idx].LRU =;
}

