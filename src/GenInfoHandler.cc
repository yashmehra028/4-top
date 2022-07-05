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
  //acquireLHEMEWeights(true),
  //acquireLHEParticles(true),
  acquireGenParticles(true),
  //acquireGenAK4Jets(false),
  //acquireGenAK8Jets(false),

  genInfo(nullptr)
{}

void GenInfoHandler::clear(){
  this->resetCache();

  delete genInfo; genInfo=nullptr;

  //for (auto& part:lheparticles) delete part;
  //lheparticles.clear();

  for (auto& part:genparticles) delete part;
  genparticles.clear();

  //for (auto& part:genak4jets) delete part;
  //genak4jets.clear();

  //for (auto& part:genak8jets) delete part;
  //genak8jets.clear();
}

bool GenInfoHandler::constructGenInfo(){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;
  if (SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier)) return true;

  bool res = (
    (!acquireCoreGenInfo || constructCoreGenInfo())
    //&& (!acquireLHEParticles || constructLHEParticles())
    && (!acquireGenParticles || constructGenParticles())
    //&& (!acquireGenAK4Jets || constructGenAK4Jets())
    //&& (!acquireGenAK8Jets || constructGenAK8Jets())
    );

  if (res) this->cacheEvent();
  return res;
}

bool GenInfoHandler::constructCoreGenInfo(){
#define GENINFO_NANOAOD_SCALAR_VARIABLE(TYPE, NAME, DEFVAL) TYPE const* NAME = nullptr;
#define GENINFO_NANOAOD_ARRAY_VARIABLE(TYPE, NAME, DEFVAL, MAXSIZE) GlobalCollectionNames::collsize_t const* n##NAME = nullptr; TYPE* const* arr_##NAME = nullptr;
  GENINFO_NANOAOD_ALLVARIABLES;
#undef GENINFO_NANOAOD_ARRAY_VARIABLE
#undef GENINFO_NANOAOD_SCALAR_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = true;
#define GENINFO_NANOAOD_SCALAR_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed(#NAME, NAME);
#define GENINFO_NANOAOD_ARRAY_VARIABLE(TYPE, NAME, DEFVAL, MAXSIZE) allVariablesPresent &= this->getConsumed(#NAME, n##NAME) && this->getConsumed<TYPE* const>(#NAME, arr_##NAME);
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
bool GenInfoHandler::constructGenParticles(){
  GlobalCollectionNames::collsize_t nProducts;
#define GENPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) TYPE* const* arr_##NAME = nullptr;
  GENPARTICLE_NANOAOD_VARIABLES;
#undef GENPARTICLE_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", colName_genparticles.data()), nProducts);
#define GENPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed<TYPE* const>(colName_genparticles + "_" + #NAME, arr_##NAME);
  GENPARTICLE_NANOAOD_VARIABLES;
#undef GENPARTICLE_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructGenParticles: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructGenParticles: All variables are set up!" << endl;

  if (nProducts>GlobalCollectionNames::colMaxSize_genparticles){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructGenParticles: The size of the collection (" << nProducts << ") exceeds maximum size (" << GlobalCollectionNames::colMaxSize_genparticles << ")." << endl;
    assert(0);
  }
  genparticles.reserve(nProducts);
  std::vector<std::pair<int, int>> mother_index_pairs; mother_index_pairs.reserve(nProducts);
#define GENPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) TYPE* it_##NAME = nullptr; if (arr_##NAME) it_##NAME = &((*arr_##NAME)[0]);
  GENPARTICLE_NANOAOD_VARIABLES;
#undef GENPARTICLE_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructGenParticles: Attempting gen. particle " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass); // Yes you have to do this on a separate line because CMSSW...
      genparticles.push_back(new GenParticleObject(*it_pdgId, *it_status, momentum));
      mother_index_pairs.emplace_back(*it_genPartIdxMother, -1);
      GenParticleObject*& obj = genparticles.back();

      // Set extras
      obj->assignStatusBits(*it_statusFlags);

      // Set selection bits
      //GenParticleSelectionHelpers::setSelectionBits(*obj);

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define GENPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) if (it_##NAME) it_##NAME++;
      GENPARTICLE_NANOAOD_VARIABLES;
#undef GENPARTICLE_VARIABLE
    }
  }

  {
    assert(mother_index_pairs.size() == genparticles.size());

    auto it_genparticles = genparticles.begin();
    auto it_mother_index_pairs = mother_index_pairs.begin();
    while (it_genparticles != genparticles.end()){
      auto*& part = *it_genparticles;

      int const& imom = it_mother_index_pairs->first;
      if (imom>=0){
        assert((GlobalCollectionNames::collsize_t) imom<nProducts);
        auto*& mother = genparticles.at(imom);
        part->addMother(mother);
        mother->addDaughter(part);
      }

      int const& jmom = it_mother_index_pairs->second;
      if (jmom>=0){
        assert((GlobalCollectionNames::collsize_t) jmom<nProducts);
        auto*& mother = genparticles.at(jmom);
        part->addMother(mother);
        mother->addDaughter(part);
      }

      it_genparticles++;
      it_mother_index_pairs++;
    }
  }

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
tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", #NAME), MAXSIZE); \
this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", #NAME)); \
this->defineConsumedSloppy(Form("n%s", #NAME)); \
tree->bookArrayBranch<TYPE>(#NAME, DEFVAL, MAXSIZE); \
this->addConsumed<TYPE* const>(#NAME); \
this->defineConsumedSloppy(#NAME);
  if (acquireCoreGenInfo){
    GENINFO_NANOAOD_ALLVARIABLES;
  }
#undef GENINFO_NANOAOD_ARRAY_VARIABLE
#undef GENINFO_NANOAOD_SCALAR_VARIABLE

  if (acquireGenParticles){
    tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", colName_genparticles.data()), 0);
#define GENPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) tree->bookArrayBranch<TYPE>(colName_genparticles + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_genparticles);
    GENPARTICLE_NANOAOD_VARIABLES;
#undef GENPARTICLE_VARIABLE
  }
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", colName_genparticles.data())); this->defineConsumedSloppy(Form("n%s", colName_genparticles.data()));
#define GENPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) this->addConsumed<TYPE* const>(colName_genparticles + "_" + #NAME); this->defineConsumedSloppy(colName_genparticles + "_" + #NAME);
  GENPARTICLE_NANOAOD_VARIABLES;
#undef GENPARTICLE_VARIABLE
}
