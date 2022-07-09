#ifndef EVENTFILTERHANDLER_H
#define EVENTFILTERHANDLER_H

#include <vector>
#include <unordered_map>
#include "IvyBase.h"
#include "SimEventHandler.h"
#include "MuonObject.h"
#include "ElectronObject.h"
#include "PhotonObject.h"
#include "AK4JetObject.h"
#include "METObject.h"
#include "HLTTriggerPathObject.h"
#include "TriggerObject.h"
#include "TriggerHelpersCore.h"
#include "SystematicVariations.h"


class EventFilterHandler : public IvyBase{
public:
  static const std::string colName_triggerobjects;
  static const std::string colName_metfilters;

protected:
  std::vector<TriggerHelpers::TriggerType> const& requestedTriggers;

  bool trackDataEvents;
  bool checkUniqueDataEvent;
  bool checkHLTPathRunRanges;
  bool trackTriggerObjects;
  bool checkTriggerObjectsForHLTPaths;

  bool product_uniqueEvent;
  bool product_passDataCert;

  std::unordered_map<unsigned int, std::vector< std::pair<unsigned int, unsigned int> >> datacert_run_lumirangelist_map;

  std::vector<HLTTriggerPathObject*> product_HLTpaths;
  std::vector<TriggerObject*> product_triggerobjects;
  std::unordered_map<std::string, bool> product_metfilters;
  std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::vector<unsigned long long>> > era_dataeventblock_map;

  void clear();

  bool constructHLTPaths(SimEventHandler const* simEventHandler);
  bool constructTriggerObjects();
  bool constructMETFilters();
  bool accumulateRunLumiEventBlock();
  bool testDataCert();

public:
  // Constructors
  EventFilterHandler(std::vector<TriggerHelpers::TriggerType> const& requestedTriggers_);

  // Destructors
  ~EventFilterHandler(){ clear(); }

  bool constructFilters(SimEventHandler const* simEventHandler);

  bool hasMatchingTriggerPath(std::vector<std::string> const& hltpaths_) const;
  float getTriggerWeight(std::vector<std::string> const& hltpaths_) const;
  float getTriggerWeight(
    std::vector< std::unordered_map< TriggerHelpers::TriggerType, std::vector<HLTTriggerPathProperties> >::const_iterator > const& hltpathprops_,
    std::vector<MuonObject*> const* muons,
    std::vector<ElectronObject*> const* electrons,
    std::vector<PhotonObject*> const* photons,
    std::vector<AK4JetObject*> const* ak4jets,
    METObject const* pfmet,
    HLTTriggerPathObject const** firstPassingHLTPath = nullptr,
    std::vector<ParticleObject const*>* outparticles_TOmatched = nullptr
  ) const;
  float getTriggerWeight(
    std::vector< std::pair<TriggerHelpers::TriggerType, HLTTriggerPathProperties const*> > const& hltpathprops_,
    std::vector<MuonObject*> const* muons,
    std::vector<ElectronObject*> const* electrons,
    std::vector<PhotonObject*> const* photons,
    std::vector<AK4JetObject*> const* ak4jets,
    METObject const* pfmet,
    HLTTriggerPathObject const** firstPassingHLTPath = nullptr,
    std::vector<ParticleObject const*>* outparticles_TOmatched = nullptr
  ) const;
  bool passMETFilters() const;

  // Special event filters for various issues
  bool test2018HEMFilter(
    SimEventHandler const* simEventHandler,
    std::vector<ElectronObject*> const* electrons,
    std::vector<PhotonObject*> const* photons,
    std::vector<AK4JetObject*> const* ak4jets
  ) const;

  // For data trees. MC is always true
  bool const& isUniqueDataEvent() const{ return product_uniqueEvent; }
  bool const& passDataCert() const{ return product_passDataCert; }

  void setTrackDataEvents(bool flag){ this->trackDataEvents=flag; }
  void setCheckUniqueDataEvent(bool flag){ this->checkUniqueDataEvent=flag; }
  void setCheckHLTPathRunRanges(bool flag){ this->checkHLTPathRunRanges=flag; }
  void setTrackTriggerObjects(bool flag){ this->trackTriggerObjects=flag; }
  void setCheckTriggerObjectsForHLTPaths(bool flag){ this->checkTriggerObjectsForHLTPaths=flag; if (flag) this->trackTriggerObjects=flag; }

  std::vector<HLTTriggerPathObject*> const& getHLTPaths() const{ return this->product_HLTpaths; }
  std::vector<TriggerObject*> const& getTriggerObjects() const{ return this->product_triggerobjects; }
  std::unordered_map<std::string, bool> const& getMETFilters() const{ return this->product_metfilters; }

  void bookBranches(BaseTree* intree);

  void loadGoldenJSON(std::string strjsonfname);

  static std::vector<std::string> acquireMETFilterFlags(BaseTree* intree);
};


#endif
