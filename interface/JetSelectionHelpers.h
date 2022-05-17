#ifndef JETSELECTIONHELPERS_H
#define JETSELECTIONHELPERS_H

#include <vector>

// TODO: does this file exist? why do we need it?
#include <CMS3/Dictionaries/interface/JetTriggerCutEnums.h> 

#include "JetObject.h"

namespace JetSelectionHelpers{
  enum SelectionBits{
    // both loose bits have ID but isolated or non-isolated trigger
    kPreselectionTight,
    kPreselectionTightB,

    nSelectionBits;
  };

  // Kinematic pT thresholds
  constexpr float ptThr = 40.; 

  // Kinematic eta thresholds
  // Last ECAL crystal in barrel is at |eta|=1.4442
  // Gap region is between 1.4442 and 1.56, crossing is at 1.479. See ECALGeometrySpecifications.h.
  constexpr float etaThr_2016 = 2.4;
  constexpr float etaThr_2017_2018 = 2.5;

  constexpr float deltaRThr = 0.4;

  constexpr int jetIDThr = 0;

  // deep flavor cutoffs - medium
  // found here: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation
  constexpr float deepFlavThr_2016_APV = 0.2598;
  constexpr float deepFlavThr_2016_NonAPV = 0.2489;
  constexpr float deepFlavThr_2017 = 0.3040;
  constexpr float deepFlavThr_2018 = 0.2783;

  void setSelectionBits(JetObject& part);
}


#endif
