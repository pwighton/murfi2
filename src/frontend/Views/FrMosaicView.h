#ifndef FR_MOSAIC_VIEW
#define FR_MOSAIC_VIEW

class vtkRenderer;

class QVTKWidget;
class FrMainWindow;
class FrDocumentReader;
class FrTBCFilter;
class Fr2DSliceActor;

#include "FrBaseView.h"

// Mosaic view
class FrMosaicView : public FrBaseView
{ 
public:
    FrMosaicView(FrMainWindow* mainWindow);
    virtual ~FrMosaicView();

    virtual void Initialize();
    virtual void UpdatePipeline(int point);
    virtual void SetupRenderers();
    virtual void RemoveRenderers();

    // Accessors
    FrTBCFilter* GetTBCFilter() { return m_tbcFilter; }
    vtkRenderer* GetRenderer(){ return m_renderer; }

private:
    // Pipline 
    FrDocumentReader* m_docReader;
    FrTBCFilter* m_tbcFilter;
    vtkRenderer* m_renderer;

    // Actors
    Fr2DSliceActor* m_actor;
};

#endif

