#include <cassert>

#include "IvyFramework/IvyDataTools/interface/ParticleObjectHelpers.h"
#include "GlobalCollectionNames.h"
#include "HadronicTauHandler.h"
#include "HadronicTauSelectionHelpers.h"


using namespace std;
using namespace IvyStreamHelpers;


#define HADRONICTAU_MOMENTUM_VARIABLES \
HADRONICTAU_VARIABLE(float, pt, 0) \
HADRONICTAU_VARIABLE(float, eta, 0) \
HADRONICTAU_VARIABLE(float, phi, 0) \
HADRONICTAU_VARIABLE(float, mass, 0)


const std::string HadronicTauHandler::colName = GlobalCollectionNames::colName_taus;

HadronicTauHandler::HadronicTauHandler() :
  IvyBase()
{
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", HadronicTauHandler::colName.data()));
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) this->addConsumed<TYPE* const>(HadronicTauHandler::colName + "_" + #NAME);
  HADRONICTAU_MOMENTUM_VARIABLES;
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE
}


bool HadronicTauHandler::constructHadronicTaus(){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;

  bool res = constructHadronicTauObjects();

  if (res) this->cacheEvent();
  return res;
}

bool HadronicTauHandler::constructHadronicTauObjects(){
  GlobalCollectionNames::collsize_t nProducts;
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) TYPE* const* arr_##NAME;
  HADRONICTAU_MOMENTUM_VARIABLES;
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", HadronicTauHandler::colName.data()), nProducts);
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed<TYPE* const>(HadronicTauHandler::colName + "_" + #NAME, arr_##NAME);
  HADRONICTAU_MOMENTUM_VARIABLES;
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "HadronicTauHandler::constructHadronicTauObjects: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "HadronicTauHandler::constructHadronicTauObjects: All variables are set up!" << endl;

  if (nProducts==0) return true; // Construction is successful, it is just that no muons exist.

  productList.reserve(nProducts);
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) TYPE* it_##NAME = &((*arr_##NAME)[0]);
  HADRONICTAU_MOMENTUM_VARIABLES;
  HADRONICTAU_EXTRA_VARIABLES
#undef HADRONICTAU_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "HadronicTauHandler::constructHadronicTauObjects: Attempting muon " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass); // Yes you have to do this on a separate line...
      productList.push_back(new HadronicTauObject(((*it_charge)<0 ? 13 : -13), momentum));
      HadronicTauObject*& obj = productList.back();

      // Set extras
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) obj->extras.NAME = *it_##NAME;
      HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE

      // Set particle index as its unique identifier
      obj->setUniqueIdentifier(ip);

      // Set selection bits
      //HadronicTauSelectionHelpers::setSelectionBits(*obj); // Do not set them here. Migrated to ParticleDisambiguator.

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) if (it_##NAME) it_##NAME++;
      HADRONICTAU_MOMENTUM_VARIABLES;
      HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE
    }
  }
  // Sort particles
  ParticleObjectHelpers::sortByGreaterPt(productList);

  return true;
}

void HadronicTauHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", HadronicTauHandler::colName.data()), 0);
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) tree->bookArrayBranch<TYPE>(HadronicTauHandler::colName + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_muons);
  HADRONICTAU_MOMENTUM_VARIABLES;
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE
}


#undef HADRONICTAU_MOMENTUM_VARIABLES
