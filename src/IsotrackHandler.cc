#include <cassert>

#include "GlobalCollectionNames.h"
#include "ParticleObjectHelpers.h"
#include "IsotrackHandler.h"
#include "IsotrackSelectionHelpers.h"
#include "MuonSelectionHelpers.h"
#include "ElectronSelectionHelpers.h"
#include "ParticleSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


#define VECTOR_ITERATOR_HANDLER_DIRECTIVES \
ISOTRACK_VARIABLE(float, pt) \
ISOTRACK_VARIABLE(float, eta) \
ISOTRACK_VARIABLE(float, phi) \
ISOTRACK_VARIABLE(int, pdgId) \
ISOTRACK_EXTRA_VARIABLES


const std::string IsotrackHandler::colName = GlobalCollectionNames::colName_isotracks;

IsotrackHandler::IsotrackHandler() : IvyBase()
{
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", IsotrackHandler::colName.data()));
#define ISOTRACK_VARIABLE(TYPE, NAME) this->addConsumed<TYPE* const>(IsotrackHandler::colName + "_" + #NAME);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES;
#undef ISOTRACK_VARIABLE
}

bool IsotrackHandler::constructIsotracks(std::vector<MuonObject*> const* muons, std::vector<ElectronObject*> const* electrons){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;

  GlobalCollectionNames::collsize_t nProducts;
#define ISOTRACK_VARIABLE(TYPE, NAME) TYPE* const* arr_##NAME;
  VECTOR_ITERATOR_HANDLER_DIRECTIVES;
#undef ISOTRACK_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", IsotrackHandler::colName.data()), nProducts);
#define ISOTRACK_VARIABLE(TYPE, NAME) allVariablesPresent &= this->getConsumed<TYPE* const>(IsotrackHandler::colName + "_" + #NAME, arr_##NAME);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES;
#undef ISOTRACK_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "IsotrackHandler::constructIsotracks: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "IsotrackHandler::constructIsotracks: All variables are set up!" << endl;

  if (nProducts==0) return true; // Construction is successful, it is just that no muons exist.

  productList.reserve(nProducts);
#define ISOTRACK_VARIABLE(TYPE, NAME) TYPE* it_##NAME = &((*arr_##NAME)[0]);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES;
#undef ISOTRACK_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "IsotrackHandler::constructIsotracks: Attempting isotrack " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, 0); // Yes you have to do this on a separate line...
      productList.push_back(new ProductType_t(*it_pdgId, momentum));
      ProductType_t*& obj = productList.back();

      // Set extras
#define ISOTRACK_VARIABLE(TYPE, NAME) obj->extras.NAME = *it_##NAME;
      ISOTRACK_EXTRA_VARIABLES;
#undef ISOTRACK_VARIABLE

      // Set the selection bits
      IsotrackSelectionHelpers::setSelectionBits(*obj);

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define ISOTRACK_VARIABLE(TYPE, NAME) if (it_##NAME) it_##NAME++;
      VECTOR_ITERATOR_HANDLER_DIRECTIVES;
#undef ISOTRACK_VARIABLE
    }
  }
  // Sort particles
  ParticleObjectHelpers::sortByGreaterPt(productList);

  bool res = applyCleaning(muons, electrons);

  if (res) this->cacheEvent();
  return res;
}

bool IsotrackHandler::applyCleaning(std::vector<MuonObject*> const* muons, std::vector<ElectronObject*> const* electrons){
  std::vector<ProductType_t*> productList_new; productList_new.reserve(productList.size());
  for (auto*& product:productList){
    bool doSkip=false;
    float const dR_isotrack = IsotrackSelectionHelpers::getIsolationDRmax(*product);
    if (muons){
      for (auto const* part:*(muons)){
        if (!ParticleSelectionHelpers::isParticleForIsotrackCleaning(part)) continue;
        float const separation_deltaR = std::max(dR_isotrack, MuonSelectionHelpers::getIsolationDRmax(*part));
        if (product->deltaR(part)<separation_deltaR){ doSkip=true; break; }
      }
    }
    if (electrons){
      for (auto const* part:*(electrons)){
        if (!ParticleSelectionHelpers::isParticleForIsotrackCleaning(part)) continue;
        float const separation_deltaR = std::max(dR_isotrack, ElectronSelectionHelpers::getIsolationDRmax(*part));
        if (product->deltaR(part)<separation_deltaR){ doSkip=true; break; }
      }
    }
    if (!doSkip) productList_new.push_back(product);
    else delete product;
  }
  productList = productList_new;

  return true;
}

void IsotrackHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", IsotrackHandler::colName.data()), 0);
#define ISOTRACK_VARIABLE(TYPE, NAME) tree->bookArrayBranch<TYPE>(IsotrackHandler::colName + "_" + #NAME, 0, GlobalCollectionNames::colMaxSize_isotracks);
  VECTOR_ITERATOR_HANDLER_DIRECTIVES
#undef ISOTRACK_VARIABLE
}


#undef VECTOR_ITERATOR_HANDLER_DIRECTIVES
