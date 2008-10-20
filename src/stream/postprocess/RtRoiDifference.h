/******************************************************************************
 * RtRoiDifference.h is the header for a class that computes the
 * difference between two activation map combinations
 *
 * Oliver Hinds <ohinds@mit.edu> 2008-02-25
 *
 *****************************************************************************/

#ifndef RTROIDIFFERENCE_H
#define RTROIDIFFERENCE_H

#include"RtStreamComponent.h"
#include"RtActivation.h"
#include"RtRoi2Feedback.h"

// class declaration
class RtRoiDifference : public RtStreamComponent {

public:

  static string moduleString;

  //*** constructors/destructors  ***//

  // default constructor
  RtRoiDifference();

  // destructor
  ~RtRoiDifference();

protected:

  // process an option
  //  in 
  //   name of the option to process
  //   attr map bettwen attribute names and values
  virtual bool processOption(const string &name, const string &text, 
			     const map<string,string> &attr);

  // process a single acquisition
  int process(ACE_Message_Block *mb);

  // dataID for the positive and negative activation sums
  string posRoiModuleID;
  string negRoiModuleID;
  string posRoiDataName;
  string negRoiDataName;
  string posRoiRoiID;
  string negRoiRoiID;

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

