#ifndef ELECTRONSELECTIONHELPERS_H
#define ELECTRONSELECTIONHELPERS_H

#include <vector>
#include <CMS3/Dictionaries/interface/ElectronTriggerCutEnums.h>
#include "ElectronObject.h"

namespace ElectronSelectionHelpers{
  enum SelectionBits{
    // both loose bits have ID but isolated or non-isolated trigger
    kPreselection_loose_IsoTrig,
    kPreselection_loose_NoIsoTrig,
    kPreselection_tight,

    nSelectionBits;
  };

  // TODO: table 5 in AN has the squshed iso variable values
  // add the unsquash function in the implementation
  // consider the raw working points for:
  // iso https://github.com/cms-sw/cmssw/blob/master/RecoEgamma/ElectronIdentification/python/Identification/mvaElectronID_Fall17_iso_V2_cff.py#L31
  // noIso https://github.com/cms-sw/cmssw/blob/master/RecoEgamma/ElectronIdentification/python/Identification/mvaElectronID_Fall17_noIso_V2_cff.py

  // Kinematic pT thresholds
  constexpr float ptThr_skim_loose = 5.; // TODO: check AN
  constexpr float ptThr_skim_tight = 5.;

  // Kinematic eta thresholds
  // Last ECAL crystal in barrel is at |eta|=1.4442
  // Gap region is between 1.4442 and 1.56, crossing is at 1.479. See ECALGeometrySpecifications.h.
  constexpr float etaThr_skim_loose = 2.5;// TODO: check AN
  constexpr float etaThr_skim_tight = 2.5;

  // Isolation thresholds
  constexpr float isoThr_loose_IsoTrig = 0.1; // TODO: check AN
  constexpr float isoThr_loose_NoIsoTrig = 0.1; // TODO: check AN
  constexpr float isoThr_tight = 0.1;

  float getIsolationDRmax(ElectronObject const& part);

  float relMiniIso(ElectronObject const& part);

  float computeIso(ElectronObject const& part);

  void setSelectionBits(ElectronObject& part);
}


#endif
