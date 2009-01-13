#include "FrApplication.h"
#include "FrMainWindow.h"
#include "FrMainDocument.h"
#include "FrMainController.h"


#include"ace/SOCK_Stream.h"
#include"ace/SOCK_Acceptor.h"
#include"ace/SOCK_Stream.h"


// Control block for leak detection
#ifdef WIN32
    //#define MEM_LEAK_TEST 

    #ifdef MEM_LEAK_TEST
        #include <crtdbg.h>
        //#define BREAK_ON_LEAK
        #define BREAK_LEAK_ID 24648
    #endif
#endif


//
// ------- MAIN ENTRY ---------
//
int ACE_TMAIN(int argc, char **argv){
#ifdef MEM_LEAK_TEST
    char* b = new char[111];
    strcpy(b, "TEST MEMORY LEAK");

    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );

    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
#ifdef BREAK_ON_LEAK
    tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;
#endif
    tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);

#ifdef BREAK_ON_LEAK
    _CrtSetBreakAlloc(BREAK_LEAK_ID);
#endif
#endif

    FrApplication application(argc, argv);

    // Create main view and document of app
    FrMainWindow* mainWindow = new FrMainWindow();
    FrMainDocument* document = new FrMainDocument();

    // Create main controller.
    // It takes care about all the stuff (i.e. init, manage and delete).
    FrMainController controller(mainWindow, document);
    controller.Initialize();

    int result = 0;
    if(!application.exec()){
        result = 1;
    }

    return result;
}
