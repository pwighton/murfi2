// classify fmri data using a model trained using fmri_svm-light_train

#include"fmri_svm_test.h"
#include"fmri_svm_common.h"

#include<nifti1_io.h>

extern "C" {
#include<svm_struct_api.h>
}

#include<fstream>
#include<iostream>
#include<iomanip>
#include<limits>

using namespace std;

static bool quiet = false;

///// parm reading /////
extern char *optarg;
extern int optind, opterr, optopt;

#include"getopt.h"

int getopt_long(int argc, char * const argv[],
		const char *optstring,
		const struct option *longopts, int *longindex);

// main
int main(int argc, char **argv) {

  Parms parms = parseArgs(argc,argv);
  parms.executeName = argv[0];
  if(!validateParams(parms) || parms.error) {
    cerr << "exit" << endl;
    printUsage(argv[0]);
    return 0;
  }

  if(!quiet) cout << "reading in test data " << endl;

  // get the test data
  if(!createTestingData(parms)) {
    if(!quiet) cerr << "creating test data failed" << endl;
    return 0;
  }

  // compute weight vector for linear kernel
  if(parms.model.svm_model->kernel_parm.kernel_type == 0) {
    add_weight_vector_to_linear_model(parms.model.svm_model);
  }

  bool haveTruth = !parms.labelFile.empty();

  // classify each label
  LABEL *results = new LABEL[parms.sample.n];
  int correct = 0, total = 0;
  for(int i = 0; i < parms.sample.n; i++) {
    results[i] = classify_struct_example(parms.sample.examples[i].x,
					 &parms.model, &parms.strctParm);
    if(!quiet) cout << "example " << i << ": " << results[i].cls;
    if(haveTruth) {
      if(results[i].cls == parms.sample.examples[i].y.cls) {
	correct++;
      }
      if(parms.sample.examples[i].y.cls > numeric_limits<double>::epsilon()) {
	total++;
      }
      if(!quiet) cout << ": " << parms.sample.examples[i].y.cls;
    }
    if(!quiet) cout << endl;
  }

  // print performance
  if(haveTruth) {
    if(!quiet) cout << "percent correct: "
	 << setprecision(4) << 100 * correct / (double) total << "% ("
	 << correct << "/" << total << ")" << endl;
  }

  // save results
  if(!parms.resultFile.empty()) {
    ofstream res(parms.resultFile.c_str(), ios::out);
    for(int i = 0; i < parms.sample.n; i++) {
      res << results[i].cls << " ";
      for(int j = 0; j < results[i].num_classes; j++) {
	res << results[i].scores[j] << " ";
      }
      res << endl;
    }
    res.close();
  }

  delete [] results;

  return 0;
}


void printUsage(string executename) {
  cout << executename << " [options]" << endl
       << " -t name: test data nifti filename (required)" << endl
       << " -l name: train data label text file (if known)" << endl
       << " -s name: svm training model file (required)" << endl
       << " -m name: !!!!mask file (default is to use model mask)" << endl
       << " -r name: save results filename" << endl
       << " -q     : quiet (don't print anything)" << endl
       << " -?: print usage" << endl << endl
       << "EXAMPLE:" << endl << endl
       << "fmri_svm_test -t fmri_test.nii -l fmri_design.tl "
       << "-m mask.nii -s model.svm -r results.nii" << endl << endl
       << "!!!!: means this gets clobbered, dont try to use it" << endl
       << endl << endl << flush;

  //print_help();
  fflush(stdout);
}

// parse the command line arguments
Parms parseArgs(int argc, char **argv) {
  int opt, option_index = 0;
  char *format;
  Parms p;

  // loop through input arguments
  bool done = false, crossVal = false;
  int ind = 1;
  for(; !done; ind+=2) {
    opt = getopt_long (argc, argv, "-t:l:m:s:r:q?",
		       NULL, &option_index);
    if(opt == -1) {
      break;
    }
    if(opt == ':') {
      printUsage(argv[0]);
      p.error = true;
      return p;
    }

    if(opt == '?') {
      printUsage(argv[0]);
      p.error = true;
      return p;
    }

    switch(opt) {
    case '-':
      done = true;
      break;
    case 't':
      p.exampleFile = optarg;
      break;
    case 'l':
      p.labelFile = optarg;
      break;
    case 'm':
      p.maskFile = optarg;
      break;
    case 's':
      p.modelFile = optarg;
      break;
    case 'r':
      p.resultFile = optarg;
      break;
    case 'q':
      quiet = true;
      break;
    case '?':
      printUsage(argv[0]);
      p.error = true;
      return p;
      break;
    }
  }

  // parse svm-light options
//  read_input_parameters(argc - ind, argv + ind,
//			&p.learnParm, &p.kernelParm);

  return p;
}

bool validateParams(Parms &p) {
  if(p.exampleFile.empty()) {
    cerr << "example data file ia a required argument" << endl;
    p.error = true;
  }

  if(p.modelFile.empty()) {
    cerr << "model file ia a required argument" << endl;
    p.error = true;
  }

  return !p.error;
}

// read data
bool createTestingData(Parms &parms) {

  // read the model
  if(!readFmriModel(parms.modelFile, parms)) {
    cerr << "could not read trained model file " << parms.modelFile << endl;
    return false;
  }

  nifti_image *mask = NULL;

  // read the mask header
  short *maskdat = NULL;
  long numOnVox = 0;

  mask = nifti_image_read(parms.maskFile.c_str(), 0);
  if(mask == NULL) {
    cerr << "could not open " << parms.maskFile
	 << " for reading a mask nifti image" << endl;
    return false;
  }

  // validate type
  if(mask->datatype != DT_SIGNED_SHORT) {
    cout << "only short int masks are currently supported, sorry" << endl;
    return 1;
  }

  // read the mask data
  nifti_image_load(mask);
  maskdat = (short*) mask->data;


  // count the mask voxels
  for(long i = 0; i < mask->nvox; i++) {
    if(maskdat[i]) numOnVox++;
  }
  parms.numWords = numOnVox;

  if(numOnVox < 1) {
    cerr << "no voxels in mask" << endl;
    return false;
  }

  // read data

  // testing input
  if(!readSample(parms.exampleFile, parms.labelFile, maskdat, numOnVox,
		 parms.sample)) {
    cerr << "error reading testing examples" << endl;
    return false;
  }

  return true;
}
