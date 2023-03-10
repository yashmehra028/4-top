#include <cassert>

#include "IvyFramework/IvyDataTools/interface/ParticleObjectHelpers.h"
#include "GlobalCollectionNames.h"
#include "PhotonHandler.h"
#include "PhotonSelectionHelpers.h"


using namespace std;
using namespace IvyStreamHelpers;


#define PHOTON_MOMENTUM_VARIABLES \
PHOTON_VARIABLE(float, pt, 0) \
PHOTON_VARIABLE(float, eta, 0) \
PHOTON_VARIABLE(float, phi, 0) \
PHOTON_VARIABLE(float, mass, 0)


const std::string PhotonHandler::colName = GlobalCollectionNames::colName_photons;

PhotonHandler::PhotonHandler() :
  IvyBase()
{
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", PhotonHandler::colName.data()));
#define PHOTON_VARIABLE(TYPE, NAME, DEFVAL) this->addConsumed<TYPE* const>(PhotonHandler::colName + "_" + #NAME);
  PHOTON_MOMENTUM_VARIABLES;
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE
}


bool PhotonHandler::constructPhotons(){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;

  bool res = constructPhotonObjects();

  if (res) this->cacheEvent();
  return res;
}

bool PhotonHandler::constructPhotonObjects(){
  GlobalCollectionNames::collsize_t nProducts;
#define PHOTON_VARIABLE(TYPE, NAME, DEFVAL) TYPE* const* arr_##NAME;
  PHOTON_MOMENTUM_VARIABLES;
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", PhotonHandler::colName.data()), nProducts);
#define PHOTON_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed<TYPE* const>(PhotonHandler::colName + "_" + #NAME, arr_##NAME);
  PHOTON_MOMENTUM_VARIABLES;
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "PhotonHandler::constructPhotonObjects: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "PhotonHandler::constructPhotonObjects: All variables are set up!" << endl;

  if (nProducts==0) return true; // Construction is successful, it is just that no photons exist.

  productList.reserve(nProducts);
#define PHOTON_VARIABLE(TYPE, NAME, DEFVAL) TYPE* it_##NAME = &((*arr_##NAME)[0]);
  PHOTON_MOMENTUM_VARIABLES;
  PHOTON_EXTRA_VARIABLES
#undef PHOTON_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "PhotonHandler::constructPhotonObjects: Attempting photon " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass); // Yes you have to do this on a separate line...
      productList.push_back(new PhotonObject(momentum));
      PhotonObject*& obj = productList.back();

      // Set extras
#define PHOTON_VARIABLE(TYPE, NAME, DEFVAL) obj->extras.NAME = *it_##NAME;
      PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE

      // Set particle index as its unique identifier
      obj->setUniqueIdentifier(ip);

      // Set selection bits
      PhotonSelectionHelpers::setSelectionBits(*obj);

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define PHOTON_VARIABLE(TYPE, NAME, DEFVAL) if (it_##NAME) it_##NAME++;
      PHOTON_MOMENTUM_VARIABLES;
      PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE
    }
  }
  // Sort particles
  ParticleObjectHelpers::sortByGreaterPt(productList);

  return true;
}

void PhotonHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", PhotonHandler::colName.data()), 0);
#define PHOTON_VARIABLE(TYPE, NAME, DEFVAL) tree->bookArrayBranch<TYPE>(PhotonHandler::colName + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_photons);
  PHOTON_MOMENTUM_VARIABLES;
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE
}


#undef PHOTON_MOMENTUM_VARIABLES
