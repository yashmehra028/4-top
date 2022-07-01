#include <cassert>

#include "GlobalCollectionNames.h"
#include "ParticleObjectHelpers.h"

#include "SamplesCore.h"
#include "HelperFunctions.h"
#include "GenInfoHandler.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


// Not all functions are implemented, but they will be implemented soon...
const std::string GenInfoHandler::colName_lheparticles = GlobalCollectionNames::colName_lheparticles;
const std::string GenInfoHandler::colName_genparticles = GlobalCollectionNames::colName_genparticles;
const std::string GenInfoHandler::colName_genak4jets = GlobalCollectionNames::colName_genak4jets;

GenInfoHandler::GenInfoHandler() :
  IvyBase(),

  acquireCoreGenInfo(true),

  genInfo(nullptr)
{}

void GenInfoHandler::clear(){
  this->resetCache();

  delete genInfo; genInfo=nullptr;
}

bool GenInfoHandler::constructGenInfo(){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;
  if (SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier)) return true;

  bool res = (!acquireCoreGenInfo || constructCoreGenInfo());

  if (res) this->cacheEvent();
  return res;
}

bool GenInfoHandler::constructCoreGenInfo(){
#define GENINFO_NANOAOD_SCALAR_VARIABLE(TYPE, NAME, DEFVAL) TYPE const* NAME = nullptr;
#define GENINFO_NANOAOD_ARRAY_VARIABLE(TYPE, NAME, DEFVAL, MAXSIZE) unsigned int const* n##NAME = nullptr;; TYPE* const* arr_##NAME = nullptr;
  GENINFO_NANOAOD_ALLVARIABLES;
#undef GENINFO_NANOAOD_ARRAY_VARIABLE
#undef GENINFO_NANOAOD_SCALAR_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = true;
#define GENINFO_NANOAOD_SCALAR_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed(#NAME, NAME);
#define GENINFO_NANOAOD_ARRAY_VARIABLE(TYPE, NAME, DEFVAL, MAXSIZE) allVariablesPresent &= this->getConsumed(#NAME, n##NAME); this->getConsumed<TYPE* const>(#NAME, arr_##NAME);
  GENINFO_NANOAOD_ALLVARIABLES;
#undef GENINFO_NANOAOD_ARRAY_VARIABLE
#undef GENINFO_NANOAOD_SCALAR_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructCoreGenInfo: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructCoreGenInfo: All variables are set up!" << endl;

  genInfo = new GenInfoObject();

#define GENINFO_NANOAOD_SCALAR_VARIABLE(TYPE, NAME, DEFVAL) , *NAME
#define GENINFO_NANOAOD_ARRAY_VARIABLE(TYPE, NAME, DEFVAL, MAXSIZE) , *n##NAME, *arr_##NAME
  genInfo->acquireWeights(
    currentTree->sampleIdentifier
    GENINFO_NANOAOD_ALLVARIABLES
  );
#undef GENINFO_NANOAOD_ARRAY_VARIABLE
#undef GENINFO_NANOAOD_SCALAR_VARIABLE

  return true;
}

void GenInfoHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  if (SampleHelpers::checkSampleIsData(tree->sampleIdentifier)) return;

#define GENINFO_NANOAOD_SCALAR_VARIABLE(TYPE, NAME, DEFVAL) \
tree->bookBranch<TYPE>(#NAME, DEFVAL); \
this->addConsumed<TYPE>(#NAME); \
this->defineConsumedSloppy(#NAME);
#define GENINFO_NANOAOD_ARRAY_VARIABLE(TYPE, NAME, DEFVAL, MAXSIZE) \
tree->bookBranch<unsigned int>(Form("n%s", #NAME), MAXSIZE); \
this->addConsumed<unsigned int>(Form("n%s", #NAME)); \
this->defineConsumedSloppy(Form("n%s", #NAME)); \
tree->bookArrayBranch<TYPE>(#NAME, DEFVAL, MAXSIZE); \
this->addConsumed<TYPE* const>(#NAME); \
this->defineConsumedSloppy(#NAME);
  if (acquireCoreGenInfo){
    GENINFO_NANOAOD_ALLVARIABLES;
  }
#undef GENINFO_NANOAOD_ARRAY_VARIABLE
#undef GENINFO_NANOAOD_SCALAR_VARIABLE

}
