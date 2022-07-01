#include <cassert>

#include "IvyFramework/IvyDataTools/interface/ParticleObjectHelpers.h"
#include "GlobalCollectionNames.h"
#include "MuonHandler.h"
#include "MuonSelectionHelpers.h"


using namespace std;
using namespace IvyStreamHelpers;


#define MUON_MOMENTUM_VARIABLES \
MUON_VARIABLE(float, pt) \
MUON_VARIABLE(float, eta) \
MUON_VARIABLE(float, phi) \
MUON_VARIABLE(float, mass) \
MUON_VARIABLE(int, pdgId)


const std::string MuonHandler::colName = GlobalCollectionNames::colName_muons;

MuonHandler::MuonHandler() :
  IvyBase()
{
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", MuonHandler::colName.data()));
#define MUON_VARIABLE(TYPE, NAME) this->addConsumed<TYPE* const>(MuonHandler::colName + "_" + #NAME);
  MUON_MOMENTUM_VARIABLES;
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE
}


bool MuonHandler::constructMuons(){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;

  bool res = constructMuonObjects();

  if (res) this->cacheEvent();
  return res;
}

bool MuonHandler::constructMuonObjects(){
  GlobalCollectionNames::collsize_t nProducts;
#define MUON_VARIABLE(TYPE, NAME) TYPE* const* arr_##NAME;
  MUON_MOMENTUM_VARIABLES;
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", MuonHandler::colName.data()), nProducts);
#define MUON_VARIABLE(TYPE, NAME) allVariablesPresent &= this->getConsumed<TYPE* const>(MuonHandler::colName + "_" + #NAME, arr_##NAME);
  MUON_MOMENTUM_VARIABLES;
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "MuonHandler::constructMuonObjects: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "MuonHandler::constructMuonObjects: All variables are set up!" << endl;

  if (nProducts==0) return true; // Construction is successful, it is just that no muons exist.

  productList.reserve(nProducts);
#define MUON_VARIABLE(TYPE, NAME) TYPE* it_##NAME = &((*arr_##NAME)[0]);
  MUON_MOMENTUM_VARIABLES;
  MUON_EXTRA_VARIABLES
#undef MUON_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "MuonHandler::constructMuonObjects: Attempting muon " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass); // Yes you have to do this on a separate line...
      productList.push_back(new MuonObject(*it_pdgId, momentum));
      MuonObject*& obj = productList.back();

      // Set extras
#define MUON_VARIABLE(TYPE, NAME) obj->extras.NAME = *it_##NAME;
      MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE

      // Set particle index as its unique identifier
      obj->setUniqueIdentifier(ip);

      // Set selection bits
      MuonSelectionHelpers::setSelectionBits(*obj);

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define MUON_VARIABLE(TYPE, NAME) it_##NAME++;
      MUON_MOMENTUM_VARIABLES;
      MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE
    }
  }
  // Sort particles
  ParticleObjectHelpers::sortByGreaterPt(productList);

  return true;
}

void MuonHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", MuonHandler::colName.data()), 0);
#define MUON_VARIABLE(TYPE, NAME) tree->bookArrayBranch<TYPE>(MuonHandler::colName + "_" + #NAME, 0, GlobalCollectionNames::colMaxSize_muons);
  MUON_MOMENTUM_VARIABLES;
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE
}


#undef MUON_MOMENTUM_VARIABLES
