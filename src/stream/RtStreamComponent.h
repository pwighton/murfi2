/******************************************************************************
 * RtStreamComponent.h is the header for an abstract class for a
 * processing stream component for a real-time fMRI.
 *
 * Oliver Hinds <ohinds@mit.edu> 2007-08-14 
 * 
 *****************************************************************************/

#ifndef RTSTREAMCOMPONENT_H
#define RTSTREAMCOMPONENT_H

#include"ace/Task.h"

#include"RtStream.h"
#include"RtStreamMessage.h"
#include"RtConfig.h"
#include"RtData.h"
//#include"RtPasser.h"

// NOTES FOR SUBCLASSING:
// when this class is subclassed it should get its own 
//   static string moduleString;
// then this string should be defined at the top of the source file and id
// should be assigned to moduleString in all constructors. this rather
// cumbersome process is required to maintain the ability to retreive
// component specific id strings both statically and dynamically. sorry.

class RtPasser;

// class declaration
class RtStreamComponent : public ACE_Task<ACE_MT_SYNCH> {

public:

  //*** constructors/destructors  ***//
  
  // default constructor
  RtStreamComponent();

  // destructor
  virtual ~RtStreamComponent();

  //*** initialization routines  ***//

  // adds an output to receive the data of this stream component
  //  in
  //   output to add
  virtual void addOutput(RtOutput *out, const string &dataId = "");

  // initialize and run thread
  //  out:
  //   0 (for success) or -1 (failure)
  virtual int open(void * = 0);

  // send data when we're done
  //  in 
  //   msg: message to send
  //   to:  timeout
  virtual int put(ACE_Message_Block *msg, ACE_Time_Value *to);

  // close a stream component
  //  in
  //   flags: flags to tell us who called
  virtual int close(u_long flags);
  
  // run the stream
  virtual int svc();

  // get the version
  //  out: char array that represents the cvs version
  virtual char *getVersionString();

  // sets whether this data should be kept around after the stream is done
  void setPersistent(bool p);

  // gets whether this data should be kept around after the stream is done
  bool getPersistent();

  // gets the id for this stream component
  string getID();

  // module name for config
  static string moduleString;

protected:

  typedef ACE_Task<ACE_MT_SYNCH> super;

  // call the next processing step
  virtual int nextStep(ACE_Message_Block *mb);

  // pure virtual for implementation of real processing
  virtual int process(ACE_Message_Block *mb) = 0;

  // pass any results to outputs
  virtual void passData(RtData* data);

  // sets the latest result of processing
  //  in
  //   data result
  void setResult(RtStreamMessage *msg, RtData *data);

  // passer to send the results of our computation to outputs
  RtPasser *passer;

  // whether data created by this component should be persistent
  bool persistent;

  // id string
  string id;
};


class RtEndTask : public RtStreamComponent {

public:
  RtEndTask() : RtStreamComponent() {
    id = "end-task";
  }

protected:
  virtual int process(ACE_Message_Block*) {
    return 0;
  }

  virtual int nextStep(ACE_Message_Block *) {
    return 0;
  }
};


#endif

/*****************************************************************************
 * $Source$
 * Local Variables:
 * mode: c++
 * fill-column: 76
 * comment-column: 0
 * End:
 *****************************************************************************/

