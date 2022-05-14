#ifndef MUONSELECTIONHELPERS_H
#define MUONSELECTIONHELPERS_H

#include <vector>
#include <CMS3/Dictionaries/interface/MuonTriggerCutEnums.h>
#include "MuonObject.h"

namespace MuonSelectionHelpers{
  enum SelectionBits{
    // both loose bits have ID but isolated or non-isolated trigger
    kPreselection_loose_IsoTrig,
    kPreselection_loose_NoIsoTrig,
    kPreselection_tight,

    nSelectionBits;
  };

  // Kinematic pT thresholds
  constexpr float ptThr_cat0 = 5.; 
  constexpr float ptThr_cat1 = 10.;
  constexpr float ptThr_cat2 = 25.;

  // Kinematic eta thresholds
  // Last ECAL crystal in barrel is at |eta|=1.4442
  // Gap region is between 1.4442 and 1.56, crossing is at 1.479. See ECALGeometrySpecifications.h.
  constexpr float etaThr_cat0 = 0.8; 
  constexpr float etaThr_cat1 = 1.479;
  constexpr float etaThr_cat2 = 2.4; // line 316 in the AN2018_062_v17

  // Isolation thresholds
  constexpr float isoThr_loose_I1 = 0.4; 
  constexpr float isoThr_loose_I2 = 0.0; 
  constexpr float isoThr_loose_I3 = 0.0; 
  constexpr float isoThr_medium_I1 = 0.11; 
  constexpr float isoThr_medium_I2 = 0.74; 
  constexpr float isoThr_medium_I3 = 6.8; 
  constexpr float isoThr_tight_I1 = 0.07; 
  constexpr float isoThr_tight_I2 = 0.78; 
  constexpr float isoThr_tight_I3 = 8.0; 

  float getIsolationDRmax(MuonObject const& part);

  float relMiniIso(MuonObject const& part);

  float computeIso(MuonObject const& part);

  void setSelectionBits(MuonObject& part);
}


#endif
