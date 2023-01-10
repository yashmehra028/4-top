#include <cassert>

#include "GlobalCollectionNames.h"
#include "ParticleObjectHelpers.h"

#include "SamplesCore.h"
#include "HelperFunctions.h"
#include "GenInfoHandler.h"
#include "IvyFramework/IvyDataTools/interface/IvyPDGHelpers.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


// Not all functions are implemented, but they will be implemented soon...
const std::string GenInfoHandler::colName_lheparticles = GlobalCollectionNames::colName_lheparticles;
const std::string GenInfoHandler::colName_genparticles = GlobalCollectionNames::colName_genparticles;
const std::string GenInfoHandler::colName_genak4jets = GlobalCollectionNames::colName_genak4jets;
//const std::string GenInfoHandler::colName_genak8jets = GlobalCollectionNames::colName_genak8jets;

GenInfoHandler::GenInfoHandler() :
  IvyBase(),

  KFactor_QCD_ggVV_Sig_handle(nullptr),
  KFactor_QCD_qqVV_Bkg_handle(nullptr),
  KFactor_EW_qqVV_Bkg_handle(nullptr),

  acquireCoreGenInfo(true),
  acquireLHEMEWeights(true),
  acquireLHEParticles(true),
  acquireGenParticles(true),
  acquireGenAK4Jets(true),
  //acquireGenAK8Jets(true),

  genInfo(nullptr)
{}

GenInfoHandler::~GenInfoHandler(){
  clear();

  delete KFactor_QCD_ggVV_Sig_handle;
  delete KFactor_QCD_qqVV_Bkg_handle;
  delete KFactor_EW_qqVV_Bkg_handle;
}

void GenInfoHandler::clear(){
  this->resetCache();

  delete genInfo; genInfo=nullptr;

  for (auto& part:lheparticles) delete part;
  lheparticles.clear();

  for (auto& part:genparticles) delete part;
  genparticles.clear();

  for (auto& part:genak4jets) delete part;
  genak4jets.clear();

  //for (auto& part:genak8jets) delete part;
  //genak8jets.clear();
}

bool GenInfoHandler::constructGenInfo(){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;
  if (SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier)) return true;

  bool const require_kfactor_computations = !kfactor_num_denum_list.empty();
  bool res = (
    ((!acquireCoreGenInfo && !require_kfactor_computations) || constructCoreGenInfo())
    && ((!acquireGenParticles && !require_kfactor_computations) || constructGenParticles())
    && (!(acquireLHEParticles || require_kfactor_computations) || constructLHEParticles())
    && (!acquireGenAK4Jets || constructGenAK4Jets())
    //&& (!acquireGenAK8Jets || constructGenAK8Jets())
    && (!require_kfactor_computations || computeKFactors())
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

  std::unordered_map<TString, float const*> kfactorlist;
  for (TString const& strkfactor:tree_kfactorlist_map[currentTree]){
    kfactorlist[strkfactor] = nullptr;
    allVariablesPresent &= this->getConsumed(strkfactor, kfactorlist.find(strkfactor)->second);
    if (!(kfactorlist.find(strkfactor)->second)){
      if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructCoreGenInfo: K factor handle for " << strkfactor << " is null!" << endl;
      assert(0);
    }
  }

  std::unordered_map<TString, float const*> MElist;
  if (acquireLHEMEWeights){
    for (TString const& strme:tree_MElist_map[currentTree]){
      MElist[strme] = nullptr;
      allVariablesPresent &= this->getConsumed(strme, MElist.find(strme)->second);
      if (!(MElist.find(strme)->second)){
        if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructCoreGenInfo: ME handle for " << strme << " is null!" << endl;
        assert(0);
      }
    }
  }

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructCoreGenInfo: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructCoreGenInfo: All variables are set up!" << endl;

  genInfo = new GenInfoObject();

#define GENINFO_NANOAOD_SCALAR_VARIABLE(TYPE, NAME, DEFVAL) , *NAME
#define GENINFO_NANOAOD_ARRAY_VARIABLE(TYPE, NAME, DEFVAL, MAXSIZE) , *n##NAME, *arr_##NAME
  genInfo->acquireGenInfo(
    currentTree->sampleIdentifier
    GENINFO_NANOAOD_ALLVARIABLES
  );
#undef GENINFO_NANOAOD_ARRAY_VARIABLE
#undef GENINFO_NANOAOD_SCALAR_VARIABLE

  for (auto it:kfactorlist) genInfo->extras.Kfactors[it.first] = (it.second ? *(it.second) : 1.f);
  for (auto it:MElist) genInfo->extras.LHE_ME_weights[it.first] = (it.second ? *(it.second) : 0.f);

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

bool GenInfoHandler::constructLHEParticles(){
  GlobalCollectionNames::collsize_t nProducts;
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) TYPE* const* arr_##NAME = nullptr;
  LHEPARTICLE_VARIABLES;
#undef LHEPARTICLE_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", colName_lheparticles.data()), nProducts);
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed<TYPE* const>(colName_lheparticles + "_" + #NAME, arr_##NAME);
  LHEPARTICLE_VARIABLES;
#undef LHEPARTICLE_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructLHEParticles: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructLHEParticles: All variables are set up!" << endl;

  if (nProducts>GlobalCollectionNames::colMaxSize_lheparticles){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructLHEParticles: The size of the collection (" << nProducts << ") exceeds maximum size (" << GlobalCollectionNames::colMaxSize_lheparticles << ")." << endl;
    assert(0);
  }
  lheparticles.reserve(nProducts);
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) TYPE* it_##NAME = nullptr; if (arr_##NAME) it_##NAME = &((*arr_##NAME)[0]);
  LHEPARTICLE_VARIABLES;
#undef LHEPARTICLE_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructGenParticles: Attempting gen. particle " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      if ((*it_status)!=-1) momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass); // Yes you have to do this on a separate line because CMSSW...
      else momentum = ParticleObject::LorentzVector_t(0, 0, *it_incomingpz, std::sqrt(std::pow(*it_mass, 2)+std::pow(*it_incomingpz, 2)));
      lheparticles.push_back(new LHEParticleObject(*it_pdgId, *it_status, momentum));

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) if (it_##NAME) it_##NAME++;
      LHEPARTICLE_VARIABLES;
#undef LHEPARTICLE_VARIABLE
    }
  }

  return true;
}

bool GenInfoHandler::constructGenAK4Jets(){
  GlobalCollectionNames::collsize_t nProducts;
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) TYPE* const* arr_##NAME = nullptr;
  GENJET_NANOAOD_VARIABLES;
#undef GENJET_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", colName_genak4jets.data()), nProducts);
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) allVariablesPresent &= this->getConsumed<TYPE* const>(colName_genak4jets + "_" + #NAME, arr_##NAME);
  GENJET_NANOAOD_VARIABLES;
#undef GENJET_VARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructGenAK4Jets: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructGenAK4Jets: All variables are set up!" << endl;

  if (nProducts>GlobalCollectionNames::colMaxSize_genak4jets){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::constructGenAK4Jets: The size of the collection (" << nProducts << ") exceeds maximum size (" << GlobalCollectionNames::colMaxSize_genak4jets << ")." << endl;
    assert(0);
  }
  genak4jets.reserve(nProducts);
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) TYPE* it_##NAME = nullptr; if (arr_##NAME) it_##NAME = &((*arr_##NAME)[0]);
  GENJET_NANOAOD_VARIABLES;
#undef GENJET_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "GenInfoHandler::constructGenAK4Jets: Attempting gen. ak4jet " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass); // Yes you have to do this on a separate line because CMSSW...
      genak4jets.push_back(new GenJetObject(momentum));
      auto& obj = genak4jets.back();

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) if (it_##NAME) it_##NAME++;
      GENJET_NANOAOD_VARIABLES;
#undef GENJET_VARIABLE
    }
  }

  return true;
}

void GenInfoHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  if (SampleHelpers::checkSampleIsData(tree->sampleIdentifier)) return;

  bool const require_kfactor_computations = !kfactor_num_denum_list.empty();

  std::vector<TString> allbranchnames; tree->getValidBranchNamesWithoutAlias(allbranchnames, false);

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

  if (acquireLHEParticles || require_kfactor_computations){
    tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", colName_lheparticles.data()), 0);
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) tree->bookArrayBranch<TYPE>(colName_lheparticles + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_lheparticles);
    LHEPARTICLE_VARIABLES;
#undef LHEPARTICLE_VARIABLE
  }
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", colName_lheparticles.data())); this->defineConsumedSloppy(Form("n%s", colName_lheparticles.data()));
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) this->addConsumed<TYPE* const>(colName_lheparticles + "_" + #NAME); this->defineConsumedSloppy(colName_lheparticles + "_" + #NAME);
  LHEPARTICLE_VARIABLES;
#undef LHEPARTICLE_VARIABLE

  // K factor and ME reweighting branches are defined as sloppy
  std::vector<TString> kfactorlist;
  std::vector<TString> melist;
  bool has_lheparticles=false;
  for (TString const& bname:allbranchnames){
    if (bname.Contains("KFactor")){
      tree->bookBranch<float>(bname, 1.f);
      this->addConsumed<float>(bname);
      this->defineConsumedSloppy(bname);
      kfactorlist.push_back(bname);
    }
    if (acquireLHEMEWeights && (bname.Contains("p_Gen") || bname.Contains("LHECandMass"))){
      tree->bookBranch<float>(bname, 0.f);
      this->addConsumed<float>(bname);
      this->defineConsumedSloppy(bname);
      melist.push_back(bname);
    }
    else if ((acquireLHEParticles || require_kfactor_computations) && bname.Contains(colName_lheparticles)) has_lheparticles = true;
  }

  tree_kfactorlist_map[tree] = kfactorlist;
  tree_MElist_map[tree] = melist;
  tree_lheparticles_present_map[tree] = has_lheparticles;

  if (acquireGenAK4Jets){
    tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", colName_genak4jets.data()), 0);
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) tree->bookArrayBranch<TYPE>(colName_genak4jets + "_" + #NAME, DEFVAL, GlobalCollectionNames::colMaxSize_genak4jets);
    GENJET_NANOAOD_VARIABLES;
#undef GENJET_VARIABLE
  }
  this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", colName_genak4jets.data())); this->defineConsumedSloppy(Form("n%s", colName_genak4jets.data()));
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) this->addConsumed<TYPE* const>(colName_genak4jets + "_" + #NAME); this->defineConsumedSloppy(colName_genak4jets + "_" + #NAME);
  GENJET_NANOAOD_VARIABLES;
#undef GENJET_VARIABLE
}

void GenInfoHandler::setupKFactorHandles(std::vector<std::string> const& strkfactoropts){
  std::vector<std::pair<std::string, std::string>> kfactorsets;
  for (auto const& strkfactoropt:strkfactoropts){
    if (strkfactoropt == "applyKFactorQCDNNLOtoN3LOggVVSig") kfactorsets.emplace_back("kfactor_qcd_nnlo_n3lo_ggvv_sig", "");
    else if (strkfactoropt == "applyKFactorQCDLOtoNNLOggVVSig" || strkfactoropt == "applyKFactorQCDNLOtoNNLOggVVSig") kfactorsets.emplace_back(
      "kfactor_qcd_nnlo_ggvv_sig",
      (strkfactoropt == "applyKFactorQCDLOtoNNLOggVVSig" ? "" : "kfactor_qcd_nlo_ggvv_sig")
    );
    else if (strkfactoropt == "applyKFactorQCDNLOtoNNLOqqZZBkg" || strkfactoropt == "applyKFactorQCDNLOtoNNLOqqWZBkg" || strkfactoropt == "applyKFactorQCDNLOtoNNLOqqWWBkg"){
      std::string strnumerator = "";
      if (strkfactoropt == "applyKFactorQCDNLOtoNNLOqqZZBkg") strnumerator = "kfactor_qcd_nnlo_qqzz_bkg";
      else if (strkfactoropt == "applyKFactorQCDNLOtoNNLOqqWZBkg") strnumerator = "kfactor_qcd_nnlo_qqwz_bkg";
      else strnumerator = "kfactor_qcd_nnlo_qqww_bkg";
      kfactorsets.emplace_back(strnumerator, "");
    }
    else if (strkfactoropt == "applyKFactorEWLOtoNLOqqZZBkg" || strkfactoropt == "applyKFactorEWLOtoNLOqqWZBkg" || strkfactoropt == "applyKFactorEWLOtoNLOqqWWBkg"){
      std::string strnumerator = "";
      if (strkfactoropt == "applyKFactorEWLOtoNLOqqZZBkg") strnumerator = "kfactor_ew_nlo_qqzz_bkg";
      else if (strkfactoropt == "applyKFactorEWLOtoNLOqqWZBkg") strnumerator = "kfactor_ew_nlo_qqwz_bkg";
      else strnumerator = "kfactor_ew_nlo_qqww_bkg";
      kfactorsets.emplace_back(strnumerator, "");
    }
  }

  if (!kfactorsets.empty() && this->verbosity>=MiscUtils::INFO) IVYout << "GenInfoHandler::setupKFactorHandles: The following K factor sets are set up:" << kfactorsets << "." << endl;

  if (!kfactorsets.empty()) kfactor_num_denum_list.reserve(kfactorsets.size());
  for (auto const& pp:kfactorsets){
    // Get K factor specifications
    std::string const& strkfactor_num = pp.first;
    std::string const& strkfactor_den = pp.second;
    // Use lowercase letters for comparison
    std::string strkfactor_num_lower, strkfactor_den_lower;
    HelperFunctions::lowercase(strkfactor_num, strkfactor_num_lower);
    HelperFunctions::lowercase(strkfactor_den, strkfactor_den_lower);

    // Build K factors
    KFactorHelpers::KFactorType numerator = KFactorHelpers::nKFactorTypes;
    KFactorHelpers::KFactorType denominator = KFactorHelpers::nKFactorTypes;
    if (strkfactor_num_lower == "kfactor_qcd_nnlo_n3lo_ggvv_sig") numerator = KFactorHelpers::kf_QCD_NNLO_N3LO_GGVV_SIG;
    else if (strkfactor_num_lower == "kfactor_qcd_nnlo_ggvv_sig") numerator = KFactorHelpers::kf_QCD_NNLO_GGVV_SIG;
    else if (strkfactor_num_lower == "kfactor_qcd_nlo_ggvv_sig") numerator = KFactorHelpers::kf_QCD_NLO_GGVV_SIG;
    else if (strkfactor_num_lower == "kfactor_qcd_nnlo_qqzz_bkg") numerator = KFactorHelpers::kf_QCD_NNLO_QQZZ_BKG;
    else if (strkfactor_num_lower == "kfactor_qcd_nnlo_qqwz_bkg") numerator = KFactorHelpers::kf_QCD_NNLO_QQWZ_BKG;
    else if (strkfactor_num_lower == "kfactor_qcd_nnlo_qqww_bkg") numerator = KFactorHelpers::kf_QCD_NNLO_QQWW_BKG;
    else if (strkfactor_num_lower == "kfactor_ew_nlo_qqzz_bkg") numerator = KFactorHelpers::kf_EW_NLO_QQZZ_BKG;
    else if (strkfactor_num_lower == "kfactor_ew_nlo_qqwz_bkg") numerator = KFactorHelpers::kf_EW_NLO_QQWZ_BKG;
    else if (strkfactor_num_lower == "kfactor_ew_nlo_qqww_bkg") numerator = KFactorHelpers::kf_EW_NLO_QQWW_BKG;
    else{
      if (this->verbosity>=MiscUtils::ERROR) IVYerr << Form("GenInfoHandler::setupKFactorHandles: Cannot identify the numerator of the K factor pair (%s, %s).", strkfactor_num.data(), strkfactor_den.data()) << endl;
      assert(0);
    }

    bool doBuild_KFactor_QCD_ggVV_Sig_handle = (numerator==KFactorHelpers::kf_QCD_NNLO_N3LO_GGVV_SIG || numerator==KFactorHelpers::kf_QCD_NNLO_GGVV_SIG || numerator==KFactorHelpers::kf_QCD_NLO_GGVV_SIG);
    if (doBuild_KFactor_QCD_ggVV_Sig_handle){
      if (strkfactor_den_lower == "kfactor_qcd_nnlo_n3lo_ggvv_sig") denominator = KFactorHelpers::kf_QCD_NNLO_N3LO_GGVV_SIG;
      else if (strkfactor_den_lower == "kfactor_qcd_nnlo_ggvv_sig") denominator = KFactorHelpers::kf_QCD_NNLO_GGVV_SIG;
      else if (strkfactor_den_lower == "kfactor_qcd_nlo_ggvv_sig") denominator = KFactorHelpers::kf_QCD_NLO_GGVV_SIG;
      else if (strkfactor_den_lower != ""){
        if (this->verbosity>=MiscUtils::ERROR) IVYerr << Form("GenInfoHandler::setupKFactorHandles: K factor pair (%s, %s) is not implemented.", strkfactor_num.data(), strkfactor_den.data()) << endl;
        assert(0);
      }
      if (!KFactor_QCD_ggVV_Sig_handle) KFactor_QCD_ggVV_Sig_handle = new KFactorHelpers::KFactorHandler_QCD_ggVV_Sig(SampleHelpers::getDataYear());
    }

    bool doBuild_KFactor_QCD_qqVV_Bkg_handle = (numerator==KFactorHelpers::kf_QCD_NNLO_QQZZ_BKG || numerator==KFactorHelpers::kf_QCD_NNLO_QQWZ_BKG || numerator==KFactorHelpers::kf_QCD_NNLO_QQWW_BKG);
    if (doBuild_KFactor_QCD_qqVV_Bkg_handle && !KFactor_QCD_qqVV_Bkg_handle) KFactor_QCD_qqVV_Bkg_handle = new KFactorHelpers::KFactorHandler_QCD_qqVV_Bkg(SampleHelpers::getDataYear());

    bool doBuild_KFactor_EW_qqVV_Bkg_handle = (numerator==KFactorHelpers::kf_EW_NLO_QQZZ_BKG || numerator==KFactorHelpers::kf_EW_NLO_QQWZ_BKG || numerator==KFactorHelpers::kf_EW_NLO_QQWW_BKG);
    if (doBuild_KFactor_EW_qqVV_Bkg_handle){
      if (KFactor_EW_qqVV_Bkg_handle){
        if (this->verbosity>=MiscUtils::ERROR) IVYerr << "GenInfoHandler::setupKFactorHandles: KFactor_EW_qqVV_Bkg_handle is already set up!" << endl;
        assert(0);
      }
      KFactor_EW_qqVV_Bkg_handle = new KFactorHelpers::KFactorHandler_EW_qqVV_Bkg(SampleHelpers::getDataYear(), numerator);
    }

    if (numerator!=KFactorHelpers::nKFactorTypes) kfactor_num_denum_list.emplace_back(numerator, denominator);
  }
}

bool GenInfoHandler::computeKFactors(){
  if (KFactor_QCD_ggVV_Sig_handle){
    std::vector<GenParticleObject*> Higgses;
    std::vector<GenParticleObject*> Vbosons;

    for (auto const& part:genparticles){
      if (part->extras.isHardProcess){
        if (IvyPDGHelpers::isAHiggs(part->pdgId())) Higgses.push_back(part);
        else if (IvyPDGHelpers::isAZBoson(part->pdgId()) || IvyPDGHelpers::isAWBoson(part->pdgId()) || IvyPDGHelpers::isAPhoton(part->pdgId())) Vbosons.push_back(part);
      }
    }

    if (Higgses.size()==1) genInfo->extras.Kfactors[KFactorHelpers::KFactorHandler_QCD_ggVV_Sig::KFactorArgName] = Higgses.front()->mass();
    else if (Vbosons.size()==2) genInfo->extras.Kfactors[KFactorHelpers::KFactorHandler_QCD_ggVV_Sig::KFactorArgName] = (Vbosons.front()->p4() + Vbosons.back()->p4()).M();
    else{
      IVYerr << "GenInfoHandler::computeKFactors: No single Higgs candidate or two intermediate V bosons are found to pass to KFactor_QCD_ggVV_Sig_handle." << endl;
      assert(0);
    }

    for (auto const& kfpair:kfactor_num_denum_list){
      if (
        kfpair.first == KFactorHelpers::kf_QCD_NNLO_N3LO_GGVV_SIG
        ||
        kfpair.first == KFactorHelpers::kf_QCD_NNLO_GGVV_SIG
        ||
        kfpair.first == KFactorHelpers::kf_QCD_NLO_GGVV_SIG
        ) KFactor_QCD_ggVV_Sig_handle->eval(kfpair.first, kfpair.second, genInfo->extras.Kfactors);
    }
  }

  if (KFactor_QCD_qqVV_Bkg_handle || KFactor_EW_qqVV_Bkg_handle){
    KFactorHelpers::KFactorType corr_type = KFactorHelpers::nKFactorTypes;
    if (KFactor_EW_qqVV_Bkg_handle) corr_type = KFactor_EW_qqVV_Bkg_handle->getType();
    else{
      for (auto const& kfpair:kfactor_num_denum_list){
        if (
          kfpair.first == KFactorHelpers::kf_QCD_NNLO_QQZZ_BKG
          ||
          kfpair.first == KFactorHelpers::kf_QCD_NNLO_QQWZ_BKG
          ||
          kfpair.first == KFactorHelpers::kf_QCD_NNLO_QQWW_BKG
          ){
          corr_type = kfpair.first;
          break;
        }
      }
    }

    KFactorHelpers::VVFinalStateType final_state_type = KFactorHelpers::nVVFinalStateTypes;
    switch (corr_type){
    case KFactorHelpers::kf_QCD_NNLO_QQZZ_BKG:
    case KFactorHelpers::kf_EW_NLO_QQZZ_BKG:
      final_state_type = KFactorHelpers::kZZ;
      break;
    case KFactorHelpers::kf_QCD_NNLO_QQWZ_BKG:
    case KFactorHelpers::kf_EW_NLO_QQWZ_BKG:
      final_state_type = KFactorHelpers::kWZ;
      break;
    case KFactorHelpers::kf_QCD_NNLO_QQWW_BKG:
    case KFactorHelpers::kf_EW_NLO_QQWW_BKG:
      final_state_type = KFactorHelpers::kWW;
      break;
    default:
      IVYerr << "GenInfoHandler::computeKFactors: No known VV final state for the correction type " << corr_type << "." << endl;
      assert(0);
      break;
    }

    std::vector<GenParticleObject*> incomingQuarks;
    std::vector<GenParticleObject*> incomingGluons;
    std::vector<GenParticleObject*> outgoingQuarks;
    std::vector<GenParticleObject*> outgoingGluons;
    std::pair<GenParticleObject*, GenParticleObject*> V1pair;
    std::pair<GenParticleObject*, GenParticleObject*> V2pair;
    KFactorHelpers::getVVTopology(
      final_state_type, genparticles,
      incomingQuarks, incomingGluons,
      outgoingQuarks, outgoingGluons,
      V1pair, V2pair
    );

    float PDF_x1=0, PDF_x2=0;
    {
      double const xsf = 2./(SampleHelpers::getSqrts()*1000.);
      int id1=-9000, id2=-9000;
      float pz1=0, pz2=0;
      for (auto const& part:lheparticles){
        if (part->status()!=-1) continue;
        if (id1==-9000){
          id1 = part->pdgId();
          pz1 = part->pz();
        }
        else{
          id2 = part->pdgId();
          pz2 = part->pz();
        }
      }
      PDF_x1 = (pz1>pz2 ? pz1 : pz2);
      PDF_x2 = (pz1>pz2 ? pz2 : pz1);
      PDF_x1 *= xsf;
      PDF_x2 *= xsf;
    }

    for (auto const& kfpair:kfactor_num_denum_list){
      if (
        kfpair.first == KFactorHelpers::kf_QCD_NNLO_QQZZ_BKG
        ||
        kfpair.first == KFactorHelpers::kf_QCD_NNLO_QQWZ_BKG
        ||
        kfpair.first == KFactorHelpers::kf_QCD_NNLO_QQWW_BKG
        ) KFactor_QCD_qqVV_Bkg_handle->eval(kfpair.first, V1pair, V2pair, genInfo->extras.Kfactors);
      if (
        kfpair.first == KFactorHelpers::kf_EW_NLO_QQZZ_BKG
        ||
        kfpair.first == KFactorHelpers::kf_EW_NLO_QQWZ_BKG
        ||
        kfpair.first == KFactorHelpers::kf_EW_NLO_QQWW_BKG
        ) KFactor_EW_qqVV_Bkg_handle->eval(
          PDF_x1, PDF_x2,
          incomingQuarks, V1pair, V2pair,
          genInfo->extras.Kfactors
        );
    }
  }

  return true;
}
