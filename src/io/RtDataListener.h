/*=========================================================================
 *  RtDataListener.h declares a class that provides an interface for
 *  data availability notification
 *
 *  Copyright 2007-2013, the MURFI dev team.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef RTDATALISTENER_H
#define RTDATALISTENER_H

#include"RtDataID.h"

// pure virtual listener class
// TODO: impement mechanism to selectively notify (only certain data)
class RtDataListener {

 public:

  RtDataListener() {};
  virtual ~RtDataListener() {};

  // entry function for notification that new data is ready
  virtual void notify(const RtDataID &dataID) = 0;
};

#endif
