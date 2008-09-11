/******************************************************************************
 * RtSlideWinCor.cpp is the implementation of a class that estimates the
 * instantaneous activation at each voxel in an image using a sliding
 * window correlation analysis from Gembris 2000
 *
 *****************************************************************************/
 
#include"RtSlideWinCor.h"
#include"RtMRIImage.h"
#include"gsl/gsl_cdf.h"

string RtSlideWinCor::moduleString("slidewincor");

// default constructor
RtSlideWinCor::RtSlideWinCor() : RtActivationEstimator() {
  componentID = moduleString;

  needsInit = true;
  
  conditionColumn = 0;
  windowLen = 16;
  L = 1; //TODO make into conf variable or initialize from condition matrix
  numData = 0;
  a = c = e = NULL;
  b = d = 0.0;
  rOld = rNew = xOld = xNew = 0.0;
  rWindow = NULL;
  xWindow = NULL;
  rs = NULL;
}

// destructor
RtSlideWinCor::~RtSlideWinCor() {
    if(a != NULL) {
      delete a;
	}

    if(c != NULL) {
      delete c;
  	}

    if(e != NULL) {
      delete e;
  	}

    if(xWindow != NULL) {
      delete xWindow;
  	}

    if(rWindow != NULL) {
      delete rWindow;
  	}

    if(rs != NULL) {
      delete rs;
  	}
  	
  cout << "destroyed" << endl;
}

// process a configuration option in name of the option to process val text of the option node
bool RtSlideWinCor::processOption(const string &name, const string &text, const map<string,string> &attrMap) {

  // look for known options
  if(name == "conditionColumn") {
    return RtConfigVal::convert<unsigned int>(conditionColumn,text);    
  }
  if(name == "windowLen") {
    return RtConfigVal::convert<unsigned int>(windowLen,text);    
  }
  
  return RtActivationEstimator::processOption(name, text, attrMap);
}

// process a single acquisition
int RtSlideWinCor::process(ACE_Message_Block *mb) {
  ACE_TRACE(("RtSlideWinCor::process"));

  RtStreamMessage *msg = (RtStreamMessage*) mb->rd_ptr();

  // get the current image to operate on
  RtMRIImage *dat = (RtMRIImage*)msg->getCurrentData();

  if(dat == NULL) {
    cout << "RtSlideWinCor::process: data passed is NULL" << endl;

    ACE_DEBUG((LM_INFO, "RtSlideWinCor:process: data passed is NULL\n"));
    return 0;
  }
  
  numTimepoints++;

  // initialize the computation if necessary
  if(needsInit) {

    // get the number of datapoints we must process
    numData = dat->getNumEl();
	
    a = new vnl_vector<double>(numData);
	a->fill(0.0);
	
	c = new vnl_vector<double>(numData);
	c->fill(0.0);
	
	e = new vnl_vector<double>(numData);
	e->fill(0.0);

	rWindow = new vnl_vector<double>(windowLen);
	rWindow->fill(0.0);
	
	xWindow = new vnl_matrix<double>(windowLen,numData);
	xWindow->fill(0.0);

	rs = new vnl_vector<double>(windowLen);
	rs->fill(0.0);

    // build the mask image
    initEstimation(*dat);

    needsInit = false;
  }
  
  ACE_DEBUG((LM_DEBUG, "including image %d in the sliding window correlation estimate\n", dat->getAcquisitionNum()));
  
  // validate sizes
  if(dat->getNumEl() != numData) {
    ACE_DEBUG((LM_INFO, "RtSlideWinCor::process: new data has wrong number of elements\n"));
    return -1;    
  }

  // allocate a new data image for the correlation
  // TODO perhaps make a function so this can grow more nicely
  RtActivation *cor   = new RtActivation(*dat);
  RtActivation *beta1 = new RtActivation(*dat);
  RtActivation *beta2 = new RtActivation(*dat);
  RtActivation *stat = new RtActivation(*dat); 
  cor->initToZeros();
  beta1->initToZeros();
  beta2->initToZeros();
  stat->initToZeros();
  if(numTimepoints > numTrends+1) {
    cor->setThreshold(getTStatThreshold(1));
	beta1->setThreshold(getTStatThreshold(1));
    beta2->setThreshold(getTStatThreshold(1));
    stat->setThreshold(getTStatThreshold(1));
  }
  // end allocation

  // set up indexing variables
  unsigned int newIndex = numTimepoints-1, oldIndex = 0;
  unsigned int N = newIndex;
  
  // initialize h for correlation calculation
  double h = 0.0;
  
  // Accumulation phase. Use numTimepoints here to get the current number of data sets in window (N in pseudocode)
  if(N < windowLen) {
    rNew = conditions.get(newIndex,conditionColumn);
    rWindow->put(newIndex,rNew);  // store a window's length of reg vector

    b = b + rNew;
    d = d + rNew*rNew;
    
    // fm for stat calculation TODO this will spit out NaN until numerator is positive
    double fm = sqrt(((double)numTimepoints-(double)L-1.0)/(double)numTimepoints);
    
    // loop through voxel data
    for(unsigned int dataIndex = 0; dataIndex < dat->getNumEl(); dataIndex++) {
		if(!mask.getPixel(dataIndex)) {
      		cor->setPixel(dataIndex,fmod(1.0,0.0));  // assign nan
      		beta1->setPixel(dataIndex,fmod(1.0,0.0));
      		beta2->setPixel(dataIndex,fmod(1.0,0.0));
      		stat->setPixel(dataIndex,fmod(1.0,0.0));
			continue;
		}
		
		// Update storage variables
		xNew = dat->getElement(dataIndex);
		
		// Add data to new row in xWindow
		xWindow->put(N,dataIndex,xNew);  // store window length number of rows of x vectors
		
		a->put(dataIndex,a->get(dataIndex)+xNew*rNew);  // sum of x*r
		c->put(dataIndex,c->get(dataIndex)+xNew);       // sum of x
		e->put(dataIndex,e->get(dataIndex)+xNew*xNew);  // sum of x*x

		h = (numTimepoints*d - b*b)*(numTimepoints*e->get(dataIndex) - c->get(dataIndex)*c->get(dataIndex));
		cor->setPixel(dataIndex,(numTimepoints*a->get(dataIndex) - b*c->get(dataIndex)) / sqrt(h));
		
		// beta = (w'*w)^-1*w'*xWindow(:,dataIndex).
        // The calculation below is a sort of hard-coded
        // version of this equation to make things a bit easier. It will need to be rewritten more
        // generally if there are more than two betas. Column 1 in w is ones. Column 2 in w is rWindow
		double tempb1 = 0, tempb2 = 0;
		for(unsigned int winInd = 0; winInd < N; winInd++) {
			tempb1 += (d-b*rWindow->get(winInd))*xWindow->get(winInd,dataIndex);
			tempb2 += (-b+numTimepoints*rWindow->get(winInd))*xWindow->get(winInd,dataIndex);
		}
		beta1->setPixel(dataIndex,tempb1/(numTimepoints*d - b*b));
		beta2->setPixel(dataIndex,tempb2/(numTimepoints*d - b*b));
		
		// stat calculation
		double tempd = 0.0;
        for(unsigned int winInd = 0; winInd < N; winInd++) {
			rs->put(winInd,rWindow->get(winInd) - b/numTimepoints);
			//double xW = xWindow->get(winInd,dataIndex);
			//double rW = rWindow->get(winInd);
			tempd = tempd + pow(xWindow->get(winInd,dataIndex) - beta1->getPixel(dataIndex) - beta2->getPixel(dataIndex)*rWindow->get(winInd),2);
		}
	    stat->setPixel(dataIndex,beta2->getPixel(dataIndex)*((double)numTimepoints/tempd)*rs->one_norm()*fm);
    }
  }
  // Steady-state calculation
  else {
  	// Update storage variables
  	oldIndex = newIndex - windowLen;
	rOld = conditions.get(oldIndex,conditionColumn); // discarded element of reference vector
	rNew = conditions.get(newIndex,conditionColumn); // new element of reference vector
	
	// Slide rWindow
	for(unsigned int k = 0; k < windowLen-1; k++) {
		rWindow->put(k,rWindow->get(k+1)); // drop rOld, shift values left
	}
	
	// Add new element to rWindow
	rWindow->put(windowLen-1,rNew);
	
	// Update subsidiary variables
	b = b - rOld + rNew;              // sum of r
	d = d - rOld*rOld + rNew*rNew;    // sum of r^2

    // fm for stat calculation
    double fm = sqrt(((double)windowLen-(double)L-1.0)/(double)windowLen);

	// loop through voxel data
	for(unsigned int dataIndex = 0; dataIndex < dat->getNumEl(); dataIndex++) {
		if(!mask.getPixel(dataIndex)) {
      		cor->setPixel(dataIndex,fmod(1.0,0.0));  // assign nan
      		beta1->setPixel(dataIndex,fmod(1.0,0.0));
      		beta2->setPixel(dataIndex,fmod(1.0,0.0));
      		stat->setPixel(dataIndex,fmod(1.0,0.0));
			continue;
		}
		
		// Update xOld
		xOld = xWindow->get(0,dataIndex); // dataIndex-th value from oldest dat

		// Slide dataIndex-th xWindow values
		for(unsigned int k = 0; k < windowLen-1; k++) {
			xWindow->put(k,dataIndex,xWindow->get(k+1,dataIndex));
		}
		
		// Update xNew
		xNew = dat->getElement(dataIndex); // dataIndex-th value from newest dat		
		
		// Add dataIndex-th new data to new row in xWindow
		xWindow->put(windowLen-1,dataIndex,xNew);
		
		a->put(dataIndex,a->get(dataIndex) - xOld*rOld + xNew*rNew); // sum of x*r
		c->put(dataIndex,c->get(dataIndex) - xOld + xNew);           // sum of x
		e->put(dataIndex,e->get(dataIndex) - xOld*xOld + xNew*xNew); // sum of x*x
		
		h = (windowLen*d - b*b)*(windowLen*e->get(dataIndex) - c->get(dataIndex)*c->get(dataIndex));
		cor->setPixel(dataIndex,(windowLen*a->get(dataIndex) - b*c->get(dataIndex)) / sqrt(h));

        // Beta calculations
        // betas = (w'*w)^-1*w'*xWindow(:,dataIndex)
        // The new beta calculation below is a sort of hard-coded version of this 
        // equation to make things a bit easier. TODO it will need to be rewritten more
        // generally if there are more than two betas. Column 1 in w is ones. 
        // Column 2 in w is rWindow
		double tempb1 = 0.0;
		double tempb2 = 0.0;
		for(unsigned int winInd = 0; winInd < windowLen; winInd++) {
			tempb1 += (d-b*rWindow->get(winInd))*xWindow->get(winInd,dataIndex);
			tempb2 += (-b+windowLen*rWindow->get(winInd))*xWindow->get(winInd,dataIndex);
		}
		beta1->setPixel(dataIndex,tempb1/(windowLen*d - b*b));
		beta2->setPixel(dataIndex,tempb2/(windowLen*d - b*b));
		
		// stat calculation
		double tempd = 0.0;
        for(unsigned int winInd = 0; winInd < windowLen; winInd++) {
			rs->put(winInd,rWindow->get(winInd) - b/windowLen);
			//double xW = xWindow->get(winInd,dataIndex);
			//double rW = rWindow->get(winInd);
			tempd = tempd + pow(xWindow->get(winInd,dataIndex) - beta1->getPixel(dataIndex) - beta2->getPixel(dataIndex)*rWindow->get(winInd),2);
		}
	    stat->setPixel(dataIndex,beta2->getPixel(dataIndex)*((double)windowLen/tempd)*rs->one_norm()*fm);
		
        // fill log file
	    if(dumpAlgoVars) {
    		dumpFile
			<< numTimepoints << " " 
			<< dataIndex << " " 
			<< xNew << " "
			<< conditions.get(numTimepoints-1,0) << " "
			<< cor->getPixel(dataIndex) << " "
			<< beta1->getPixel(dataIndex) << " "
			<< beta2->getPixel(dataIndex) << " "
			<< stat->getPixel(dataIndex) << " ";
			dumpFile << endl;
		}
  	}
  }

  // set the image id for handling
  cor->addToID("voxel-slidewincor");
  beta1->addToID("voxel-beta1");
  beta2->addToID("voxel-beta2");
  stat->addToID("voxel-stat");
  
  setResult(msg,cor);
  setResult(msg,beta1);
  setResult(msg,beta2);
  setResult(msg,stat);

//  // test if this is the last measurement for mask conversion and saving
//  if(numTimepoints == numMeas && saveTVol) {
//    cout << "saving t vol to " << saveTVolFilename << endl;
//    cout.flush();
//    cor->write(saveTVolFilename);
//  }
//  if(numTimepoints == numMeas && savePosResultAsMask) {
//    cout << "saving pos mask to " << savePosAsMaskFilename << endl;
//    cout.flush();
//
//    RtMaskImage *activationMask = cor->toMask(POS);
//    activationMask->setFilename(savePosAsMaskFilename);
//    activationMask->save();
//  }
//  if(numTimepoints == numMeas && saveNegResultAsMask) {
//    cout << "saving neg mask to " << saveNegAsMaskFilename << endl;
//    cout.flush();
//
//    RtMaskImage *activationMask = cor->toMask(NEG);
//    activationMask->setFilename(saveNegAsMaskFilename);
//    activationMask->save();
//  }

  return 0;
}

// start a logfile
void RtSlideWinCor::startDumpAlgoVarsFile() {
  dumpFile << "started at ";
  printNow(dumpFile);
  dumpFile << endl
	   << "time_point "
	   << "voxel_index "
	   << "voxel_intensity "
	   << "condition "
	   << "correlation "
	   << "beta1 "
	   << "beta2 "
	   << "stat";
  dumpFile << endl;  
}

/*****************************************************************************
 * $Source:  $
 * Local Variables:
 * mode: c++
 * fill-column: 76
 * comment-column: 0
 * End:
 *****************************************************************************/