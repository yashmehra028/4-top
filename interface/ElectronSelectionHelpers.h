#ifndef ELECTRONSELECTIONHELPERS_H
#define ELECTRONSELECTIONHELPERS_H

#include <vector>
#include "ElectronObject.h"


namespace ElectronSelectionHelpers{
  enum SelectionBits{
    kKinOnly,

    kPreselectionLoose_IsoTrig,
    kPreselectionLoose_NoIsoTrig,
    kPreselectionLoose,
    kPreselectionFakeable,
    kPreselectionTight,

    nSelectionBits
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
  constexpr float etaThr_cat2 = 2.5;

  // Isolation thresholds
  constexpr float isoThr_loose_I1 = 0.4;
  constexpr float isoThr_loose_I2 = 0.0;
  constexpr float isoThr_loose_I3 = 0.0;
  constexpr float isoThr_tight_I1 = 0.07;
  constexpr float isoThr_tight_I2 = 0.78;
  constexpr float isoThr_tight_I3 = 8.0;

  float getIsolationDRmax(ElectronObject const& part);

  void setSelectionBits(ElectronObject& part);
}


#endif
