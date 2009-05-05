#include"ace/SOCK_Stream.h"
#include"ace/SOCK_Connector.h"
#include"ace/SOCK_Stream.h"
#include<iostream>
#include<string>
#include<sstream>

using namespace std;

int main(int argc, char **args) {
  // no parms
  if(argc < 2) {
    cout << "usage: " << args[0] << " dataName roiName [tr host port]" << endl;
    return 1;
  }

  // parms
  int arg = 1;
  string dataName(args[arg]); arg++;
  string roiName((argc > arg) ? args[arg] : "active"); arg++;
  float tr = atof((argc > arg) ? args[arg] : "2"); arg++;
  string host((argc > arg) ? args[arg] : "localhost"); arg++;
  unsigned short port = (unsigned short) atoi((argc > arg) ? args[arg] : "15002"); arg++;

  // Local server address.
  ACE_INET_Addr my_addr(port, host.c_str());
  // Data transfer object.
  ACE_SOCK_Stream stream;
  // Initialize the connector.
  ACE_SOCK_Connector connector;
  char messageTerminationChar = '\n';
  bool entered = false;

  // keep making new connections FOREVER!
  for(int t = 1; !connector.connect(stream, my_addr); t++) {
    entered = true;
    stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"  
	<< "<info>" 
	<< "<data name=\"" << dataName 
	<< "\" roi=\"" << roiName 
	<< "\" tr=\"" << t << "\">" 
	<< time(NULL) 
	<< "</data>" 
	<< "</info>";
    
    unsigned int sent = stream.send_n(xml.str().c_str(), xml.str().length());
    if(sent < xml.str().length()) {
      cerr << "incomplete send" << endl;
      return false;
    }
  
    stream.send_n(&messageTerminationChar,1);
    stream.close();

    cout << "sent " << xml.str() << endl;    

    sleep(tr);
    return 0;
  }

  if(!entered) {
    cout << "error connecting" << endl;    
  }
  
  return 0;
}
