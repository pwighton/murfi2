/******************************************************************************
 * RtImageZScore.cpp is the implementation of a class that computes the
 * instantaneous activation in an Image
 *
 * Oliver Hinds <ohinds@mit.edu> 2007-09-05
 *
 *****************************************************************************/

#include"RtImageZScore.h"

string RtImageZScore::moduleString("voxel-zscore");

// default constructor
RtImageZScore::RtImageZScore() : RtStreamComponent(), threshold(1.0) {
  id = moduleString;
}

// destructor
RtImageZScore::~RtImageZScore() {
  cout << "destroyed" << endl;
}

// process a single acquisition
int RtImageZScore::process(ACE_Message_Block *mb) {
  ACE_TRACE(("RtImageZScore::process"));

  RtStreamMessage *msg = (RtStreamMessage*) mb->rd_ptr();

  // get the current image to operate on
  RtMRIImage *img = (RtMRIImage*)msg->getCurrentData();

  if(img == NULL) {
    cout << "RtImageZScore::process: image passed is NULL" << endl;

    ACE_DEBUG((LM_INFO, "RtImageZScore:process: image passed is NULL\n"));
    return 0;
  }

  // get the mean
  RtMRIImage *mean = (RtMRIImage*)msg->getDataByID("data.image.mri.voxel-mean");

  if(mean == NULL) {
    cout << "RtImageZScore:process: mean image not found" << endl;

    ACE_DEBUG((LM_INFO, "RtImageZScore:process: mean image not found\n"));
    return 0;
  }
  
  // get the variance
  RtMRIImage *var = (RtMRIImage*)msg->getDataByID("data.image.mri.voxel-variance");

  if(var == NULL) {
    cout << "RtImageZScore:process: variance image not found" << endl;

    ACE_DEBUG((LM_INFO, "RtImageZScore:process: variance image not found\n"));
    return 0;
  }
  
  ACE_DEBUG((LM_DEBUG, "including image %d in the z-score estimate\n", 
	     img->getAcquisitionNum()));
  
  // validate sizes
  if(img->getNumPix() != mean->getNumPix() 
     || img->getNumPix() != var->getNumPix()) {
    ACE_DEBUG((LM_INFO, "RtImageZScore::process: image is different size than mean or variance image\n"));
    return -1;    
  }
  
  // allocate a new data image for the zscore
  RtActivation *zscore = new RtActivation(*img);
  
  // compute zscore for each voxel
  for(unsigned int i = 0; i < img->getNumPix(); i++) {
    double z = ((double) img->getPixel(i) - mean->getPixel(i)) 
      / sqrt(var->getPixel(i));

    zscore->setPixel(i, z);
  }  

  // set the image id for handling
  zscore->addToID("voxel-zscore");
  setResult(msg,zscore);

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
