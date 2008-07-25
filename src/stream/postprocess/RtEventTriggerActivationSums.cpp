/******************************************************************************
 * RtEventTriggerActivationSums.cpp triggers an event based on activation sums
 *
 * Oliver Hinds <ohinds@mit.edu> 2008-04-14
 *
 *****************************************************************************/

#include"RtEventTriggerActivationSums.h"
#include"RtActivation.h"

#include"RtEvent.h"

string RtEventTriggerActivationSums::moduleString("event-trigger-activation-sums");

// default constructor
RtEventTriggerActivationSums::RtEventTriggerActivationSums() : RtEventTrigger() {
  componentID = moduleString;
}

// destructor
RtEventTriggerActivationSums::~RtEventTriggerActivationSums() {}

// process an option
//  in 
//   name of the option to process
//   val  text of the option node
bool RtEventTriggerActivationSums::processOption(const string &name, const string &text) {
  return RtStreamComponent::processOption(name, text);
}  

// process a single acquisition
int RtEventTriggerActivationSums::process(ACE_Message_Block *mb) {  
  ACE_TRACE(("RtEventTriggerActivationSums::process"));

  static int trsSinceTrigger = afterTriggerSkipTRs;
  
  cout << "event trigger dump: " 
    << monitorPos << " "
    << monitorNeg << " "
    << posThresh << " "
    << negThresh << " "
    << posroiID << " "
    << negroiID << endl;

  if(!monitorPos && !monitorNeg) {
    return 0;
  }

  // check if we need to skip triggers
  if(trsSinceTrigger < afterTriggerSkipTRs) {
    trsSinceTrigger++;
    return 0;
  }

  RtStreamMessage *msg = (RtStreamMessage*) mb->rd_ptr();

  // find the positive activation sum with the right roiID
  RtActivation *posact 
    = (RtActivation*) msg->getDataByIDAndRoiID("data.image.activation.activation-sum",
						  posroiID);
  if(monitorPos && posact == NULL) {
    cout << "couldn't find positive roi " << posroiID << endl;

    ACE_DEBUG((LM_INFO, "RtEventTriggerActivationSums:process: no positive ROI found\n"));
    return 0;
  }
  
  // find the positive activation sum with the right roiID
  RtActivation *negact 
    = (RtActivation*) msg->getDataByIDAndRoiID("data.image.activation.activation-sum",
						  negroiID);
  if(monitorNeg && negact == NULL) {
    cout << "couldn't find negative roi " << negroiID << endl;

    ACE_DEBUG((LM_INFO, "RtEventTriggerActivationSums:process: no negative ROI found\n"));
    return 0;
  }

  int tr = ((RtMRIImage*)msg->getCurrentData())->getAcquisitionNum();
  if(tr < initialSkipTRs) {
    return 0;
  }

  cout << "checking for even trigger: " 
       << posact->getPixel(0) << " >= " << posThresh 
       << " and "
       << negact->getPixel(0) << " <= " << negThresh 
       << endl;

  // check for good trigger
  if((!monitorPos || posact->getPixel(0) >= posThresh) 
     && (!monitorNeg || negact->getPixel(0) <= negThresh)) {
    // trigger
    RtEvent *event = new RtEvent();
    event->setTR(tr);
    event->addToID("trigger.good");
    setResult(msg,event);

    cout << "GOOD EVENT TRIGGERED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    cout << posact->getPixel(0) << " " << posThresh << endl;

    trsSinceTrigger = 0;
  }
  
  // check for bad trigger
  if(triggerBothDirections 
     && (!monitorPos || posact->getPixel(0) <= -posThresh) 
     && (!monitorNeg || negact->getPixel(0) >= -negThresh)) {
    // trigger
    RtEvent *event = new RtEvent();
    event->setTR(tr);
    event->addToID("trigger.bad");
    setResult(msg,event);

    cout << "BAD EVENT TRIGGERED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    cout << posact->getPixel(0) << " " << posThresh << endl;

    trsSinceTrigger = 0;
  }
  
  return 0;
}


/*****************************************************************************
 * $Source$
 * Local Variables:
 * mode: c++
 * fill-column: 76
 * comment-column: 0
 * End:
 *****************************************************************************/

