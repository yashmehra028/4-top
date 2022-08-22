#ifndef AK4JETSELECTIONHELPERS_H
#define AK4JETSELECTIONHELPERS_H

#include <vector>
#include "AK4JetObject.h"


namespace AK4JetSelectionHelpers{
  enum SelectionBits{
    kJetIdOnly,
    kBTagOnly,
    kKinOnly,

    kPreselectionTight,
    kPreselectionTight_BTagged,

    nSelectionBits
  };

  // Kinematic pT thresholds
  constexpr float ptThr = 25.;
  constexpr float ptThr_HEMVeto = 30.;

  // Kinematic eta thresholds
  constexpr float etaThr_common = 4.7;
  constexpr float etaThr_btag_Phase0Tracker = 2.4;
  constexpr float etaThr_btag_Phase1Tracker = 2.5;

  constexpr float deltaRThr = 0.4;

  constexpr int jetIdBitPos = 1;

  // Deep flavor cutoffs - medium
  // Found here: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation
  constexpr float deepFlavThr_2016_APV = 0.2598;
  constexpr float deepFlavThr_2016_NonAPV = 0.2489;
  constexpr float deepFlavThr_2017 = 0.3040;
  constexpr float deepFlavThr_2018 = 0.2783;

  float getBtaggingWP();

  void setSelectionBits(AK4JetObject& part);
}


#endif
