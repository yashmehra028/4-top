#ifndef TRIGGERHELPERSCORE_H
#define TRIGGERHELPERSCORE_H

#include <vector>
#include <unordered_map>
#include <utility>
#include "HLTTriggerPathProperties.h"
#include "HLTTriggerPathObject.h"
#include "TriggerObject.h"


namespace TriggerHelpers{
  enum TriggerType{
    kTripleLep=0,

    kDoubleMu,
    kDoubleMu_Extra,
    kDoubleMu_PFHT,
    kDoubleMu_Prescaled,

    kDoubleEle,
    kDoubleEle_Extra,
    kDoubleEle_HighPt,
    kDoubleEle_PFHT,

    kMuEle,
    kMuEle_Extra,
    kMuEle_PFHT,

    kSingleMu,
    kSingleMu_Prescaled,
    kSingleMu_Eta2p1_Prescaled,
    kSingleMu_HighPt,
    kSingleMu_HighPt_Extra,
    kSingleMu_HighPt_Extra_Prescaled,
    kSingleMu_Control_NoIso,
    kSingleMu_Control_Iso,

    kSingleEle,
    kSingleEle_Prescaled,
    kSingleEle_HighPt,
    kSingleEle_HighPt_Extra,
    kSingleEle_HighPt_Extra_Prescaled,
    kSingleEle_Control_NoIso,
    kSingleEle_Control_Iso,

    kSinglePho,

    kAK8PFJet_Control,
    kVBFJets_Control,
    kPFHT_Control,
    kMET_Control,
    kPFMET_Control,
    kPFHT_PFMET_Control,
    kPFMET_MHT_Control,
    kPFHT_PFMET_MHT_Control,

    // Any other trigger that probably does not make sense and/or needs investigation
    kAuxiliary,

    nTriggerTypes
  };

  bool hasRunRangeExclusions(std::string const& name, HLTTriggerPathProperties const** out_hltprop = nullptr); // This is to allow a string-based recognition of run range exclusions.

}

#endif
