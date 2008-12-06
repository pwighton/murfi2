#ifndef FR_MAIN_DOCUMENT
#define FR_MAIN_DOCUMENT

// Forward declarations
class FrDataStore;
class FrDocumentObj;
class FrTabSettingsDocObj;
class RtData;

// Some includes
#include "FrDocument.h"
#include "FrMacro.h"
#include <vector>

class FrMainDocument: public FrDocument {
public:
    FrMainDocument();
    virtual ~FrMainDocument();

    // Some helper methods
    void GetAllImages(std::vector<FrDocumentObj*>& images);
    FrTabSettingsDocObj* GetCurrentTabSettings();
    void AddDataToStore(RtData *data);

    // Properties
    friend class FrDataStore;
    FrPropMacro(FrDataStore*, DataStore);
};

#endif

