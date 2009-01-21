#include "FrMainDocument.h"
#include "FrTBCFilter.h"
#include "FrTabSettingsDocObj.h"
#include "FrTabSettingsDocObj.h"
#include "FrDataStore.h"
#include "FrLayerDocObj.h"
#include "FrGraphTabDocObj.h"
#include "FrPointsDocObj.h"
#include "FrGraphDocObj.h"


FrMainDocument::FrMainDocument() 
: FrDocument(), m_DataStore(0){
    m_DataStore = new FrDataStore(this);
}

FrMainDocument::~FrMainDocument(){
    std::vector<FrDocumentObj*> objects;

    // Remove ROI objects
    this->GetObjectsByType(objects, FrDocumentObj::RoiObject);
    std::vector<FrDocumentObj*>::iterator it, itEnd(objects.end());
    for(it = objects.begin(); it != itEnd; ++it){
        this->Remove( (*it) );
    }

    // Remove tab objects
    this->GetObjectsByType(objects, FrDocumentObj::TabSettings);
    itEnd = objects.end();
    for(it = objects.begin(); it != itEnd; ++it){
        this->Remove( (*it) );
    }

    // Remove layer objects
    this->GetObjectsByType(objects, FrDocumentObj::LayerObject);
    itEnd = objects.end();
    for(it = objects.begin(); it != itEnd; ++it){
        this->Remove( (*it) );
    }

    // Remove view objects
    this->GetObjectsByType(objects, FrDocumentObj::ViewObject);
    itEnd = objects.end();
    for(it = objects.begin(); it != itEnd; ++it){
        this->Remove( (*it) );
    }

    // Remove points objects
    this->GetObjectsByType(objects, FrDocumentObj::PointsObject);
    itEnd = objects.end();
    for(it = objects.begin(); it != itEnd; ++it){
        this->Remove( (*it) );
    }

    // Remove image objects
    this->GetObjectsByType(objects, FrDocumentObj::ImageObject);
    itEnd = objects.end();
    for(it = objects.begin(); it != itEnd; ++it){
        // NOTE: Since this type of objects is not deleted do it manually
        FrDocumentObj* obj = (*it);
        this->Remove(obj);
        delete obj;
    }

    // Remove roi objects
    this->GetObjectsByType(objects, FrDocumentObj::RoiObject);
    itEnd = objects.end();
    for(it = objects.begin(); it != itEnd; ++it){
        // NOTE: Since this type of objects is not deleted do it manually
        FrDocumentObj* obj = (*it);
        this->Remove(obj);
        delete obj;
    }

    // delete data store
    if(m_DataStore) delete m_DataStore;
}

FrTabSettingsDocObj* FrMainDocument::GetCurrentTabSettings(){
    // get all tab settings in document
    DocObjCollection tabSets;
    this->GetObjectsByType(tabSets, FrDocumentObj::TabSettings);

    FrTabSettingsDocObj* result = 0L;
    if(tabSets.size() > 0){

        DocObjCollection::iterator it, itEnd(tabSets.end());
        for(it = tabSets.begin(); it != itEnd; ++it){

            if((*it)->GetType() != FrDocumentObj::TabSettings) continue;

            // returns first current
            FrTabSettingsDocObj* ts = (FrTabSettingsDocObj*)(*it);
            if(ts->GetIsCurrent()){
                result = ts;
                break;
            }
        }
    }
    return result;
}

FrGraphTabDocObj* FrMainDocument::GetCurrentGraphTab(){
    // get all graph tab settings in document
    DocObjCollection tabSets;
    this->GetObjectsByType(tabSets, FrDocumentObj::GraphTab);

    FrGraphTabDocObj* result = 0L;
    if(tabSets.size() > 0){
        DocObjCollection::iterator it, itEnd(tabSets.end());
        for(it = tabSets.begin(); it != itEnd; ++it){

            if((*it)->GetType() != FrDocumentObj::GraphTab) continue;

            // returns first current
            FrGraphTabDocObj* ts = (FrGraphTabDocObj*)(*it);
            if(ts->GetIsCurrent()){
                result = ts;
                break;
            }
        }
    }
    return result;
}

void FrMainDocument::AddDataToStore(RtData *data){
    m_DataStore->AddData(data);
}

FrViewDocObj* FrMainDocument::GetCurrentViewObject(){
    // get all view objects
    DocObjCollection viewObjects;
    this->GetObjectsByType(viewObjects, FrDocumentObj::ViewObject);

    FrViewDocObj* result = 0;
    if(viewObjects.size() > 0){
        result = (FrViewDocObj*)viewObjects[0];
    }
    return result;
}

FrPointsDocObj* FrMainDocument::GetCurrentPointsObject(){
    // get all view objects
    DocObjCollection pointsObjects;
    this->GetObjectsByType(pointsObjects, FrDocumentObj::PointsObject);

    FrPointsDocObj* result = 0;
    if(pointsObjects.size() > 0){
        result = (FrPointsDocObj*)pointsObjects[0];
    }
    return result;
}



FrAppSettingsDocObj* FrMainDocument::GetAppSettings(){
    DocObjCollection settings;
    this->GetObjectsByType(settings, FrDocumentObj::AppSettings);

    FrAppSettingsDocObj* result = 0;
    if(settings.size() > 0){

        result = (FrAppSettingsDocObj*)settings[0];
    }
    return result;
}

FrLayerDocObj* FrMainDocument::GetLayerDocObjByID(int id){
    DocObjCollection layers;
    this->GetObjectsByType(layers, FrDocumentObj::LayerObject);    

    FrLayerDocObj* result = 0;
    if(layers.size() > 0){
        for (int i = 0; i < layers.size(); i++){
            result = dynamic_cast<FrLayerDocObj*>(layers[i]);
            if (result->GetID() == id)
                break;
            else
                result = 0;
        }
    }

    return result;
}

FrGraphDocObj* FrMainDocument::GetGraphDocObjByID(int id){
    DocObjCollection graphs;
    this->GetObjectsByType(graphs, FrDocumentObj::GraphObject);    

    FrGraphDocObj* result = 0;
    if(graphs.size() > 0){
        for (int i = 0; i < graphs.size(); i++){
            result = dynamic_cast<FrGraphDocObj*>(graphs[i]);
            // NOTE: we use timeseria number as id for graph
            if (result->GetTimeSeria() == id)
                break;
            else
                result = 0;
        }
    }

    return result;
}
