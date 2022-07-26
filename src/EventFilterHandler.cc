#include <cassert>
#include "TRandom3.h"

#include "GlobalCollectionNames.h"
#include "EventFilterHandler.h"
#include "SamplesCore.h"
#include "HelperFunctionsCore.h"
#include "RunLumiEventBlock.h"
#include "AK4JetSelectionHelpers.h"
#include "ParticleSelectionHelpers.h"
#include "ParticleObjectHelpers.h"
#include "FourTopTriggerHelpers.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "IvyFramework/IvyDataTools/interface/HostHelpersCore.h"


using namespace std;
using namespace IvyStreamHelpers;


const std::string EventFilterHandler::colName_triggerobjects = GlobalCollectionNames::colName_triggerObjects;
const std::string EventFilterHandler::colName_metfilters = GlobalCollectionNames::colName_metfilter;

EventFilterHandler::EventFilterHandler(std::vector<TriggerHelpers::TriggerType> const& requestedTriggers_) :
  IvyBase(),
  requestedTriggers(requestedTriggers_),
  trackDataEvents(true),
  checkUniqueDataEvent(true),
  checkHLTPathRunRanges(true),
  trackTriggerObjects(false),
  checkTriggerObjectsForHLTPaths(false),
  product_uniqueEvent(true),
  product_passDataCert(true)
{
  // HLT triggers
  for (auto const& trigtype:requestedTriggers){
    auto hltnames = TriggerHelpers::getHLTMenus(trigtype);
    for (auto hltname:hltnames){
      auto ipos = hltname.find_last_of("_v");
      if (ipos!=std::string::npos){
        hltname = hltname.substr(0, ipos-1);
      }
      this->addConsumed<bool>(hltname);
    }
  }
}

void EventFilterHandler::clear(){
  this->resetCache();

  product_uniqueEvent = true;
  product_passDataCert = true;
  for (auto*& prod:product_HLTpaths) delete prod;
  product_HLTpaths.clear();
  for (auto*& prod:product_triggerobjects) delete prod;
  product_triggerobjects.clear();
}

bool EventFilterHandler::constructFilters(SimEventHandler const* simEventHandler){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;

  bool res = (
    (!trackTriggerObjects || this->constructTriggerObjects())
    &&
    this->constructHLTPaths(simEventHandler)
    &&
    this->constructMETFilters()
    &&
    this->accumulateRunLumiEventBlock()
    &&
    (datacert_run_lumirangelist_map.empty() || this->testDataCert())
    );

  if (res) this->cacheEvent();
  return res;
}

bool EventFilterHandler::hasMatchingTriggerPath(std::vector<std::string> const& hltpaths_) const{
  bool res = false;
  for (auto const& str:hltpaths_){
    for (auto const* prod:product_HLTpaths){ if (prod->name.find(str)!=std::string::npos){ res = true; break; } }
  }
  return res;
}
float EventFilterHandler::getTriggerWeight(std::vector<std::string> const& hltpaths_) const{
  if (hltpaths_.empty()) return 0;
  float failRate = 1;
  bool foundAtLeastOneTrigger = false;
  for (auto const& str:hltpaths_){
    for (auto const* prod:product_HLTpaths){
      if (prod->isValid() && prod->name.find(str)!=std::string::npos && prod->passTrigger){
        float wgt = 1.f;
        if (prod->L1prescale>0) wgt *= static_cast<float>(prod->L1prescale);
        if (prod->HLTprescale>0) wgt *= static_cast<float>(prod->HLTprescale);
        if (wgt == 1.f) return wgt; // If the event passes an unprescaled trigger, its weight is 1.
        else if (wgt == 0.f) continue;
        foundAtLeastOneTrigger = true;
        failRate *= 1.f-1.f/wgt;
      }
    }
  }
  return (foundAtLeastOneTrigger ? 1.f/(1.f-failRate) : 0.f);
}
float EventFilterHandler::getTriggerWeight(
  std::vector< std::unordered_map< TriggerHelpers::TriggerType, std::vector<HLTTriggerPathProperties> >::const_iterator > const& hltpathprops_,
  std::vector<MuonObject*> const* muons,
  std::vector<ElectronObject*> const* electrons,
  std::vector<PhotonObject*> const* photons,
  std::vector<AK4JetObject*> const* ak4jets,
  METObject const* pfmet,
  HLTTriggerPathObject const** firstPassingHLTPath,
  std::vector<ParticleObject const*>* outparticles_TOmatched
) const{
  unsigned int isize=0;
  for (auto const& p:hltpathprops_) isize += p->second.size();

  std::vector< std::pair<TriggerHelpers::TriggerType, HLTTriggerPathProperties const*> > hltpathprops_new; hltpathprops_new.reserve(isize);
  for (auto const& p:hltpathprops_){ for (auto const& pp:p->second) hltpathprops_new.emplace_back(p->first, &pp); }

  return getTriggerWeight(
    hltpathprops_new,
    muons, electrons, photons, ak4jets, pfmet,
    firstPassingHLTPath, outparticles_TOmatched
  );
}
float EventFilterHandler::getTriggerWeight(
  std::vector< std::pair<TriggerHelpers::TriggerType, HLTTriggerPathProperties const*> > const& hltpathprops_,
  std::vector<MuonObject*> const* muons,
  std::vector<ElectronObject*> const* electrons,
  std::vector<PhotonObject*> const* photons,
  std::vector<AK4JetObject*> const* ak4jets,
  METObject const* pfmet,
  HLTTriggerPathObject const** firstPassingHLTPath,
  std::vector<ParticleObject const*>* outparticles_TOmatched
) const{
  if (hltpathprops_.empty()) return 0;

  std::vector<MuonObject const*> muons_trigcheck; if (muons){ muons_trigcheck.reserve(muons->size()); for (auto const& part:(*muons)){ if (ParticleSelectionHelpers::isParticleForTriggerChecking(part)) muons_trigcheck.push_back(part); } }
  std::vector<ElectronObject const*> electrons_trigcheck; if (electrons){ electrons_trigcheck.reserve(electrons->size()); for (auto const& part:(*electrons)){ if (ParticleSelectionHelpers::isParticleForTriggerChecking(part)) electrons_trigcheck.push_back(part); } }
  std::vector<PhotonObject const*> photons_trigcheck; if (photons){ photons_trigcheck.reserve(photons->size()); for (auto const& part:(*photons)){ if (ParticleSelectionHelpers::isParticleForTriggerChecking(part)) photons_trigcheck.push_back(part); } }
  std::vector<AK4JetObject const*> ak4jets_trigcheck; if (ak4jets){ ak4jets_trigcheck.reserve(ak4jets->size()); for (auto const& jet:(*ak4jets)){ if (ParticleSelectionHelpers::isJetForTriggerChecking(jet)) ak4jets_trigcheck.push_back(jet); } }

  ParticleObject::LorentzVector_t pfmet_p4, pfmet_nomus_p4;
  if (pfmet){
    pfmet_p4 = pfmet->p4();
    pfmet_nomus_p4 = pfmet_p4;
    for (auto const& part:muons_trigcheck) pfmet_nomus_p4 += part->p4();
  }

  float ht_pt=0, ht_nomus_pt=0;
  ParticleObject::LorentzVector_t ht_p4, ht_nomus_p4;
  for (auto const& jet:ak4jets_trigcheck){
    auto jet_p4_nomus = jet->p4()/*jet->p4_nomus_basic()*/;
    auto const& jet_p4 = jet->p4();

    ht_pt += jet_p4.Pt();
    ht_p4 += jet_p4;

    ht_nomus_pt += jet_p4_nomus.Pt();
    ht_nomus_p4 += jet_p4_nomus;
  }
  ht_p4 = ParticleObject::PolarLorentzVector_t(ht_pt, 0, 0, ht_p4.Pt());
  ht_nomus_p4 = ParticleObject::PolarLorentzVector_t(ht_nomus_pt, 0, 0, ht_nomus_p4.Pt());

  for (auto const& enumType_props_pair:hltpathprops_){
    assert(enumType_props_pair.second != nullptr);
    auto const& hltprop = *(enumType_props_pair.second);
    for (auto const& prod:product_HLTpaths){
      if (prod->isValid() && prod->passTrigger && hltprop.isSameTrigger(prod->name)){
        std::vector<MuonObject const*> muons_trigcheck_TOmatched;
        std::vector<ElectronObject const*> electrons_trigcheck_TOmatched;
        std::vector<PhotonObject const*> photons_trigcheck_TOmatched;

        if (checkTriggerObjectsForHLTPaths){
          HLTTriggerPathProperties::TriggerObjectExceptionType const& TOexception = hltprop.getTOException();
          auto const& triggerObjects = prod->getTriggerObjects();

          TriggerObject::getMatchedPhysicsObjects(
            triggerObjects, { TriggerMuon }, 0.2,
            muons_trigcheck, muons_trigcheck_TOmatched
          );
          TriggerObject::getMatchedPhysicsObjects(
            triggerObjects, { TriggerElectron }, 0.2,
            electrons_trigcheck, electrons_trigcheck_TOmatched
          );
          TriggerObject::getMatchedPhysicsObjects(
            triggerObjects, { TriggerPhoton }, 0.2,
            photons_trigcheck, photons_trigcheck_TOmatched
          );

          if (this->verbosity>=MiscUtils::DEBUG){
            IVYout << "\t- Number of matched muons: " << muons_trigcheck_TOmatched.size() << " / " << muons_trigcheck.size() << endl;
            IVYout << "\t- Number of matched electrons: " << electrons_trigcheck_TOmatched.size() << " / " << electrons_trigcheck.size() << endl;
            IVYout << "\t- Number of matched photons: " << photons_trigcheck_TOmatched.size() << " / " << photons_trigcheck.size() << endl;
          }
        }

        if (
          hltprop.testCuts(
            (!checkTriggerObjectsForHLTPaths ? muons_trigcheck : muons_trigcheck_TOmatched),
            (!checkTriggerObjectsForHLTPaths ? electrons_trigcheck : electrons_trigcheck_TOmatched),
            (!checkTriggerObjectsForHLTPaths ? photons_trigcheck : photons_trigcheck_TOmatched),
            ak4jets_trigcheck,
            pfmet_p4,
            pfmet_nomus_p4,
            ht_p4,
            ht_nomus_p4
          )
          ){
          float wgt = 1.f;
          if (prod->L1prescale>0) wgt *= static_cast<float>(prod->L1prescale);
          if (prod->HLTprescale>0) wgt *= static_cast<float>(prod->HLTprescale);
          if (wgt == 0.f) continue;
          else{
            if (firstPassingHLTPath) *firstPassingHLTPath = prod;
            if (outparticles_TOmatched && checkTriggerObjectsForHLTPaths){
              outparticles_TOmatched->clear();
              outparticles_TOmatched->reserve(muons_trigcheck_TOmatched.size() + electrons_trigcheck_TOmatched.size() + photons_trigcheck_TOmatched.size());
              for (auto const& part:muons_trigcheck_TOmatched) outparticles_TOmatched->push_back(part);
              for (auto const& part:electrons_trigcheck_TOmatched) outparticles_TOmatched->push_back(part);
              for (auto const& part:photons_trigcheck_TOmatched) outparticles_TOmatched->push_back(part);
            }
            return wgt; // Take the first trigger that passed.
          }
        }
      }
    }
  }

  return 0;
}

bool EventFilterHandler::passMETFilters() const{
  bool res = true;
  auto strmetfilters = EventFilterHandler::acquireMETFilterFlags(currentTree);
  for (auto const& it:product_metfilters){ if (HelperFunctions::checkListVariable(strmetfilters, it.first)) res &= it.second; }
  return res;
}

bool EventFilterHandler::test2018HEMFilter(
  SimEventHandler const* simEventHandler,
  std::vector<ElectronObject*> const* electrons,
  std::vector<PhotonObject*> const* photons,
  std::vector<AK4JetObject*> const* ak4jets
) const{
  if (SampleHelpers::getDataYear() != 2018) return true;
  if (verbosity>=MiscUtils::DEBUG) IVYerr << "Begin EventFilterHandler::test2018HEMFilter..." << endl;

  // Do not run clear because this is a special filter that does not modify the actual class
  if (!currentTree){
    if (verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::test2018HEMFilter: Current tree is null!" << endl;
    return false;
  }

  if (!SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier)){
    if (!simEventHandler){
      if (verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::test2018HEMFilter: MC checks require a SimEventHandler!" << endl;
      assert(0);
      return false;
    }
    if (!simEventHandler->getHasHEM2018Issue()) return true;
  }
  else{
    bool allVariablesPresent = true;
#define RUNLUMIEVENT_VARIABLE(TYPE, NAME, NANONAME) TYPE const* NAME = nullptr; allVariablesPresent &= this->getConsumed(#NANONAME, NAME);
    RUNLUMIEVENT_VARIABLES;
#undef RUNLUMIEVENT_VARIABLE
    if (!allVariablesPresent){
      if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::test2018HEMFilter: Not all variables of the data case are consumed properly!" << endl;
      assert(0);
    }
    if (!SampleHelpers::isHEM2018Affected(*RunNumber)) return true; // All ok runs
  }

  // For affected runs, check object presence.
  static const std::pair<float, float> eta_region(-3.2, -1.3);
  static const std::pair<float, float> phi_region(-1.57, -0.87);
  bool doVeto = false;
  if (!doVeto && electrons){
    for (auto const* part:(*electrons)){
      if (!ParticleSelectionHelpers::isLooseParticle(part)) continue;

      float const eta = part->eta();
      float const phi = part->phi();
      if (eta>=eta_region.first && eta<=eta_region.second && phi>=phi_region.first && phi<=phi_region.second){ doVeto=true; break; }
    }
    if (verbosity>=MiscUtils::DEBUG && doVeto) IVYout << "EventFilterHandler::test2018HEMFilter: Found at least one electron satisfying HEM15/16." << endl;
  }
  if (!doVeto && photons){
    for (auto const* part:(*photons)){
      if (!ParticleSelectionHelpers::isLooseParticle(part)) continue;

      float const eta = part->eta();
      float const phi = part->phi();
      if (eta>=eta_region.first && eta<=eta_region.second && phi>=phi_region.first && phi<=phi_region.second){ doVeto=true; break; }
    }
    if (verbosity>=MiscUtils::DEBUG && doVeto) IVYout << "EventFilterHandler::test2018HEMFilter: Found at least one photon satisfying HEM15/16." << endl;
  }
  // Require a pT>30 GeV cut on jets
  if (!doVeto && ak4jets){
    for (auto const* part:(*ak4jets)){
      if (!ParticleSelectionHelpers::isJetForHEMVeto(part)) continue;

      float const eta = part->eta();
      float const phi = part->phi();
      if (eta>=eta_region.first && eta<=eta_region.second && phi>=phi_region.first && phi<=phi_region.second){ doVeto=true; break; }
    }
    if (verbosity>=MiscUtils::DEBUG && doVeto) IVYout << "EventFilterHandler::test2018HEMFilter: Found at least one AK4 jet satisfying HEM15/16." << endl;
  }

  if (verbosity>=MiscUtils::DEBUG) IVYerr << "End EventFilterHandler::test2018HEMFilter successfully." << endl;
  return !doVeto;
}

bool EventFilterHandler::constructHLTPaths(SimEventHandler const* simEventHandler){
  bool isData = SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier);
  if (!isData && simEventHandler){
    if (!simEventHandler->isAlreadyCached()){
      if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::constructHLTPaths: Need to update the SimEventHandler object first!" << endl;
      assert(0);
    }
  }

  // Construct HLT paths
  std::unordered_map<std::string, bool const*> trigger_flags;

  // Run number etc.
#define RUNLUMIEVENT_VARIABLE(TYPE, NAME, NANONAME) TYPE const* NAME = nullptr;
  RUNLUMIEVENT_VARIABLES;
#undef RUNLUMIEVENT_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = true;
#define RUNLUMIEVENT_VARIABLE(TYPE, NAME, NANONAME) allVariablesPresent &= this->getConsumed(#NANONAME, NAME);
  if (isData){
    RUNLUMIEVENT_VARIABLES;
  }
#undef RUNLUMIEVENT_VARIABLE
  for (auto const& trigtype:requestedTriggers){
    auto hltnames = TriggerHelpers::getHLTMenus(trigtype);
    for (auto const& hltname:hltnames){
      auto hltnanoname = hltname;
      auto ipos = hltnanoname.find_last_of("_v");
      if (ipos!=std::string::npos){
        hltnanoname = hltnanoname.substr(0, ipos-1);
      }
      bool const* flag_pass = nullptr;
      allVariablesPresent &= this->getConsumed(hltnanoname, flag_pass);
      trigger_flags[hltname] = flag_pass;
    }
  }
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::constructHLTPaths: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::constructHLTPaths: All variables are set up!" << endl;

  unsigned int itrig=0;
  unsigned int RunNumber_sim=0;
  bool set_RunNumber_sim=false;
  for (auto const& trigtype:requestedTriggers){
    auto hltnames = TriggerHelpers::getHLTMenus(trigtype);
    for (auto const& hltname:hltnames){
      product_HLTpaths.push_back(new HLTTriggerPathObject());
      HLTTriggerPathObject*& obj = product_HLTpaths.back();
      obj->name = hltname;
      obj->passTrigger = *(trigger_flags.find(hltname)->second);
      // FIXME: We do not have an implementation for L1 and HLT prescales.
      // That is because our NanoAOD genuises are incapable of thinking to write floats (yes, has to be floats if you have multiple L1 prescales and therefore need to take their geometric average) instead of booleans.
      // Most of the time, it would be compressed since a lot of them will be exactly 0 or 1, but our genius physicists cannot conceptualize that.
      // We need to write a dedicate EDLooper over data to make a table.
      // Pre-UL tables would not work since lumi. genuises independently decided to make a "better" golden JSON, so lumi blocks are not exactly the same anymore.
      // Fun, right?
      // (I hate incomplete frameworks that tout completeness.)

      // Set list index as its unique identifier
      obj->setUniqueIdentifier(itrig);

      // No trigger objects at the moment
      //// Associate trigger objects
      //obj->setTriggerObjects(product_triggerobjects);

      bool isValid = true;
      HLTTriggerPathProperties const* hltprop = nullptr;
      bool hasRunRangeExclusions = checkHLTPathRunRanges && TriggerHelpers::hasRunRangeExclusions(obj->name, &hltprop);
      if (hasRunRangeExclusions){
        assert(hltprop!=nullptr);
        if (!isData && !set_RunNumber_sim){
          if (!simEventHandler){
            if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::constructHLTPaths: simEventHandler is needed to determine run range exclusions!" << endl;
            assert(0);
          }
          RunNumber_sim = simEventHandler->getChosenRunNumber();
          set_RunNumber_sim = true;
        }
        isValid = hltprop->testRun((isData ? *RunNumber : RunNumber_sim));
      }
      obj->setValid(isValid);

      itrig++;
    }
  }

  return true;
}

bool EventFilterHandler::constructTriggerObjects(){
  GlobalCollectionNames::collsize_t nProducts;
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) TYPE* const* arr_##NAME;
  TRIGGEROBJECT_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = this->getConsumedValue(Form("n%s", EventFilterHandler::colName_triggerobjects.data()), nProducts);
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) allVariablesPresent &= this->getConsumed<TYPE* const>(EventFilterHandler::colName_triggerobjects + "_" + #NAME, arr_##NAME);
  TRIGGEROBJECT_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::constructTriggerObjects: Not all variables are consumed properly!" << endl;
    assert(0);
  }

  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::constructTriggerObjects: All variables are set up!" << endl;

  if (nProducts==0) return true; // Construction is successful, it is just that no muons exist.

  product_triggerobjects.reserve(nProducts);
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) TYPE* it_##NAME = &((*arr_##NAME)[0]);
  TRIGGEROBJECT_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE
  {
    GlobalCollectionNames::collsize_t ip=0;
    while (ip != nProducts){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::constructTriggerObjects: Attempting trigger object " << ip << "..." << endl;

      ParticleObject::LorentzVector_t momentum;
      momentum = ParticleObject::PolarLorentzVector_t(*it_pt, *it_eta, *it_phi, *it_mass);
      product_triggerobjects.push_back(new TriggerObject(*it_id, momentum));
      TriggerObject* const& obj = product_triggerobjects.back();

#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) obj->extras.NAME = *it_##NAME;
      TRIGGEROBJECT_EXTRA_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE

      // Set particle index as its unique identifier
      obj->setUniqueIdentifier(ip);

      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "\t- Success!" << endl;

      ip++;
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) it_##NAME++;
      TRIGGEROBJECT_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE
    }
  }

  // Sort particles
  ParticleObjectHelpers::sortByGreaterPt(product_triggerobjects);

  return true;
}

bool EventFilterHandler::constructMETFilters(){
  auto strmetfilters = EventFilterHandler::acquireMETFilterFlags(currentTree);
  product_metfilters.clear();

  bool allVariablesPresent = true;
  for (auto const& strmetfilter:strmetfilters){
    product_metfilters[strmetfilter] = false;
    allVariablesPresent &= this->getConsumedValue<bool>(strmetfilter, product_metfilters.find(strmetfilter)->second);
  }
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::constructMETFilters: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::constructMETFilters: All variables are set up!" << endl;

  return allVariablesPresent;
}

bool EventFilterHandler::accumulateRunLumiEventBlock(){
  bool isData = SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier);
  if (!isData || !trackDataEvents || !checkUniqueDataEvent){
    product_uniqueEvent = true;
    if (!isData || !trackDataEvents) return true;
  }

  bool allVariablesPresent = true;
#define RUNLUMIEVENT_VARIABLE(TYPE, NAME, NANONAME) TYPE const* NAME = nullptr; allVariablesPresent &= this->getConsumed(#NANONAME, NAME);
  RUNLUMIEVENT_VARIABLES;
#undef RUNLUMIEVENT_VARIABLE
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::accumulateRunLumiEventBlock: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: All variables are set up!" << endl;

  auto it_run = era_dataeventblock_map.find(*RunNumber);
  if (it_run == era_dataeventblock_map.end()){
    if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: Run " << *RunNumber << " is new." << endl;
    era_dataeventblock_map[*RunNumber] = std::unordered_map<unsigned int, std::vector<unsigned long long>>();
    it_run = era_dataeventblock_map.find(*RunNumber);
  }
  else if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: Run " << *RunNumber << " is already in the tracked list." << endl;

  auto it_lumi = it_run->second.find(*LuminosityBlock);
  if (it_lumi == it_run->second.end()){
    if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: Lumi. section " << *LuminosityBlock << " is new." << endl;
    it_run->second[*LuminosityBlock] = std::vector<unsigned long long>();
    it_lumi = it_run->second.find(*LuminosityBlock);
  }
  else if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: Lumi. section " << *LuminosityBlock << " is already in the tracked list." << endl;

  if (checkUniqueDataEvent){
    if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: Checking if the event is unique..." << endl;
    auto it_event = std::find(it_lumi->second.begin(), it_lumi->second.end(), *EventNumber);
    if (it_event == it_lumi->second.end()){
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: Event " << *EventNumber << " is unique." << endl;
      it_lumi->second.push_back(*EventNumber);
      product_uniqueEvent = true;
    }
    else{
      if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: Event " << *EventNumber << " is already covered." << endl;
      product_uniqueEvent = false;
    }
  }
  else{
    if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::accumulateRunLumiEventBlock: No checking is performed for event uniquenes." << endl;
    it_lumi->second.push_back(*EventNumber);
  }

  return true;
}

bool EventFilterHandler::testDataCert(){
  if (!SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier)){
    product_passDataCert = true;
    return true;
  }

  bool allVariablesPresent = true;
#define RUNLUMIEVENT_VARIABLE(TYPE, NAME, NANONAME) TYPE const* NAME = nullptr; allVariablesPresent &= this->getConsumed(#NANONAME, NAME);
  RUNLUMIEVENT_VARIABLES;
#undef RUNLUMIEVENT_VARIABLE
  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::testDataCert: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "EventFilterHandler::testDataCert: All variables are set up!" << endl;

  product_passDataCert = false;
  auto it_run = datacert_run_lumirangelist_map.find(*RunNumber);
  if (it_run != datacert_run_lumirangelist_map.end()){
    for (auto const& lumi_range:it_run->second){
      if (*LuminosityBlock>=lumi_range.first && *LuminosityBlock<=lumi_range.second){
        product_passDataCert = true;
        break;
      }
    }
  }

  return true;
}

std::vector<std::string> EventFilterHandler::acquireMETFilterFlags(BaseTree* intree){
  std::vector<std::string> res;

  switch (SampleHelpers::theDataYear){
  case 2016:
  {
    res = std::vector<std::string>{
      "goodVertices",
      "HBHENoiseFilter",
      "HBHENoiseIsoFilter",
      "EcalDeadCellTriggerPrimitiveFilter",
      "BadPFMuonFilter",
      "BadPFMuonDzFilter",
      "eeBadScFilter"
    };
    if (!SampleHelpers::checkSampleIsFastSim(intree->sampleIdentifier)){ // For data or non-FS MC
      res.push_back("globalSuperTightHalo2016Filter");
    }
    break;
  }
  case 2017:
  case 2018:
  {
    res = std::vector<std::string>{
      "goodVertices",
      "HBHENoiseFilter",
      "HBHENoiseIsoFilter",
      "EcalDeadCellTriggerPrimitiveFilter",
      "BadPFMuonFilter",
      "BadPFMuonDzFilter",
      "ecalBadCalibFilter",
      "eeBadScFilter"
    };
    if (!SampleHelpers::checkSampleIsFastSim(intree->sampleIdentifier)){ // For data or non-FS MC
      res.push_back("globalSuperTightHalo2016Filter");
    }
    break;
  }
  default:
    IVYerr << "EventFilterHandler::acquireMETFilterFlags: Data year " << SampleHelpers::theDataYear << " is not implemented!" << endl;
    assert(0);
  }

  for (auto& strmetfilter:res) strmetfilter = EventFilterHandler::colName_metfilters + "_" + strmetfilter;
  return res;
}

void EventFilterHandler::bookBranches(BaseTree* tree){
  if (!tree) return;

  // Book HLT paths
  for (auto const& trigtype:requestedTriggers){
    auto hltnames = TriggerHelpers::getHLTMenus(trigtype);
    for (auto hltname:hltnames){
      auto ipos = hltname.find_last_of("_v");
      if (ipos!=std::string::npos){
        hltname = hltname.substr(0, ipos-1);
      }
      tree->bookBranch<bool>(hltname, false);
    }
  }

  // Trigger objects
  if (trackTriggerObjects){
    tree->bookBranch<GlobalCollectionNames::collsize_t>(Form("n%s", EventFilterHandler::colName_triggerobjects.data()), 0);
    this->addConsumed<GlobalCollectionNames::collsize_t>(Form("n%s", EventFilterHandler::colName_triggerobjects.data()));
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) \
    tree->bookArrayBranch<TYPE>(EventFilterHandler::colName_triggerobjects + "_" + #NAME, 0, GlobalCollectionNames::colMaxSize_triggerObjects); \
    this->addConsumed<TYPE* const>(EventFilterHandler::colName_triggerobjects + "_" + #NAME);
    TRIGGEROBJECT_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE
  }

  // Book MET filters
  auto strmetfilters = EventFilterHandler::acquireMETFilterFlags(tree);
  for (auto const& strmetfilter:strmetfilters){
    tree->bookBranch<bool>(strmetfilter, true); // Default should be true to avoid non-existing branches
    this->addConsumed<bool>(strmetfilter);
    this->defineConsumedSloppy(strmetfilter); // Define as sloppy so that different samples from different years/versions can be processed.
  }

  // Do these for data trees
  if (SampleHelpers::checkSampleIsData(tree->sampleIdentifier)){
#define RUNLUMIEVENT_VARIABLE(TYPE, NAME, NANONAME) tree->bookBranch<TYPE>(#NANONAME, 0); this->addConsumed<TYPE>(#NANONAME); this->defineConsumedSloppy(#NANONAME);
    RUNLUMIEVENT_VARIABLES;
#undef RUNLUMIEVENT_VARIABLE
  }
}

void EventFilterHandler::loadGoldenJSON(std::string strjsonfname){
  strjsonfname = std::string(ANALYSISPKGDATAPATH.Data()) + "LumiJSON/" + strjsonfname;
  HostHelpers::ExpandEnvironmentVariables(strjsonfname);
  if (!HostHelpers::FileReadable(strjsonfname.data())){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::loadGoldenJSON: JSON file " << strjsonfname << " is not readable." << endl;
    assert(0);
  }

  std::vector<std::string> strlines;
  ifstream fin(strjsonfname.data(), ios_base::in);
  while (!fin.eof()){
    std::string strline;
    std::getline(fin, strline);
    HelperFunctions::lrstrip(strline, " \n\t{}");
    if (strline.empty()) continue;

    if (strline.find(":")!=std::string::npos){
      strlines.push_back(strline);
      continue;
    }

    if (strlines.empty()){
      if (this->verbosity>=MiscUtils::ERROR) IVYerr << "EventFilterHandler::loadGoldenJSON: Encountered an empty line collection while processing line '" << strline << "'." << endl;
      assert(0);
    }

    strlines.back() += strline;
  }
  fin.close();

  const char* chars_rm=" [\"";
  for (auto& strline:strlines){
    // Postprocess strings before interpreting them
    for (int ic=0; ic<strlen(chars_rm); ic++){ while (strline.find(chars_rm[ic])!=std::string::npos) HelperFunctions::replaceString<std::string, std::string const>(strline, std::string(1, chars_rm[ic]), ""); }
    HelperFunctions::replaceString<std::string, const char*>(strline, "]]", "]");
    while (strline.find("],")!=std::string::npos) HelperFunctions::replaceString<std::string, const char*>(strline, "],", " ");
    // Record to map
    std::string strrun, strlumipairlist_flat;
    HelperFunctions::splitOption(strline, strrun, strlumipairlist_flat, ':');
    unsigned int runval = std::stoi(strrun);
    std::vector<std::string> strlumipairlist;
    HelperFunctions::splitOptionRecursive(strlumipairlist_flat, strlumipairlist, ' ', false);
    std::vector< std::pair<unsigned int, unsigned int> > lumirangelist; lumirangelist.reserve(strlumipairlist.size());
    for (auto const& strlumipair:strlumipairlist){
      std::vector<std::string> tmp_lumipair;
      HelperFunctions::splitOptionRecursive(strlumipair, tmp_lumipair, ',', false);
      if (tmp_lumipair.size()!=2){
        if (this->verbosity>=MiscUtils::ERROR) IVYerr << "Lumi pair " << strlumipair << " is not parsed correctly!" << endl;
        assert(0);
      }
      else{
        unsigned int ilumi = std::stoi(tmp_lumipair.front());
        unsigned int jlumi = std::stoi(tmp_lumipair.back());
        lumirangelist.emplace_back(ilumi, jlumi);
      }
    }
    datacert_run_lumirangelist_map[runval] = lumirangelist;
  }
  if (this->verbosity>=MiscUtils::INFO){
    IVYout << "EventFilterHandler::loadGoldenJSON: The loaded Golden JSON run/lumi filter is as follows:" << endl;
    for (auto const& pp:datacert_run_lumirangelist_map) IVYout << "\t- " << pp.first << ": " << pp.second << endl;
  }
}
