#ifndef FR_MAIN_DOCUMENT
#define FR_MAIN_DOCUMENT

class FrTabSettingsDocObj;

#include <vector>
#include "FrDocument.h"

class FrMainDocument: public FrDocument {
public:
    FrMainDocument();
    virtual ~FrMainDocument();

    virtual void SetDefaultValues();

    void GetAllImages(std::vector<FrDocumentObj*>& images);
    FrTabSettingsDocObj* GetCurrentTabSettings();

public:
    // Properties
    FrPropMacro(double,Threshold);
    FrPropMacro(double,Brightness);
    FrPropMacro(double,Contrast);
	FrPropMacro(int, Slice);
	FrPropMacro(int, XCoord);
	FrPropMacro(int, YCoord);

};

#endif
