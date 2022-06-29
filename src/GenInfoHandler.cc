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

  bool res = constructCoreGenInfo();

  if (res) this->cacheEvent();
  return res;
}

bool GenInfoHandler::constructCoreGenInfo(){
  // Use non-const pointer here because we might have to modify some of the weights
  float* const* arr_genWeight = nullptr;

  // Beyond this point starts checks and selection
  bool allVariablesPresent = (acquireCoreGenInfo ? this->getConsumed<float* const>("genWeight", arr_genWeight) : true);


  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructCoreGenInfo: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructCoreGenInfo: All variables are set up!" << endl;

  genInfo = new GenInfoObject();
  if (acquireCoreGenInfo) genInfo->acquireWeightsFromArray(*arr_genWeight);

  return true;
}

void GenInfoHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  if (acquireCoreGenInfo){
    tree->bookArrayBranch<float>("genWeight", 0, 500); this->addConsumed<float* const>("genWeight");
  }
}
