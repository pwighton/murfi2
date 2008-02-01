/******************************************************************************
 * RtActivationSum.h is the header for a class that computes the difference
 * between two images
 *
 * Oliver Hinds <ohinds@mit.edu> 2007-09-05
 *
 *****************************************************************************/

#ifndef RTACTIVATIONSUM_H
#define RTACTIVATIONSUM_H

#include"RtStreamComponent.h"
#include"RtActivation.h"

// class declaration
class RtActivationSum : public RtStreamComponent {

public:

  static string moduleString;

  //*** constructors/destructors  ***//

  // default constructor
  RtActivationSum();

  // destructor
  ~RtActivationSum();

protected:

  // process a single acquisition
  int process(ACE_Message_Block *mb);

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


