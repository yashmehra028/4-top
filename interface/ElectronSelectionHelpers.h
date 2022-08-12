#ifndef ELECTRONSELECTIONHELPERS_H
#define ELECTRONSELECTIONHELPERS_H

#include <vector>
#include "ElectronObject.h"
#include "IvyXGBoostInterface.h"


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
  enum SelectionType{
    kCutbased_Run2 = 0,
    kTopMVA_Run2,
    kTopMVAv2_Run2
  };

  extern SelectionType selection_type;
  void setSelectionType(SelectionType const& type);
  void setSelectionTypeByName(TString stname);


  /***********************************/
  /* Cut-based ID Run 2 requirements */
  /***********************************/

  // Kinematic pT thresholds
  constexpr float ptThr_cat0 = 5.;
  constexpr float ptThr_cat1 = 10.;
  constexpr float ptThr_cat2 = 25.;

  // Kinematic eta thresholds
  // Last ECAL crystal in barrel is at |eta|=1.4442
  // Gap region is between 1.4442 and 1.566, crossing is at 1.479. See ECALGeometrySpecifications.h.
  constexpr float etaThr_cat0 = 0.8;
  constexpr float etaThr_cat1 = 1.479;
  constexpr float etaThr_cat2 = 2.5;

  // Isolation thresholds
  constexpr float isoThr_loose_I1 = 0.4;
  constexpr float isoThr_loose_I2 = 0.0;
  constexpr float isoThr_loose_I3 = 0.0;
  constexpr float isoThr_tight_I1_Phase0Tracker = 0.12;
  constexpr float isoThr_tight_I2_Phase0Tracker = 0.80;
  constexpr float isoThr_tight_I3_Phase0Tracker = 7.2;
  constexpr float isoThr_tight_I1_Phase1Tracker = 0.07;
  constexpr float isoThr_tight_I2_Phase1Tracker = 0.78;
  constexpr float isoThr_tight_I3_Phase1Tracker = 8.0;

  // Other ID requirements
  constexpr int maxMissingHits_loose = 1;
  constexpr int maxMissingHits_fakeable_tight = 0;
  constexpr float dxyThr = 0.05;
  constexpr float dzThr = 0.1;
  constexpr float sip3dThr = 4.;
  // Trigger emulation
  // Note: This is only the remnant of what can be applied from AN-16-228 on NanoAOD.
  // Besides, it is really only valid for 2016 only.
  // These requirements are totally different in 2017, and again in 2018, and in fact, even in the latter half of 2016!
  constexpr float sieieThr_barrel_TriggerSafety = 0.011;
  constexpr float sieieThr_endcap_TriggerSafety = 0.031;
  constexpr float hoverEThr_TriggerSafety = 0.08;
  constexpr float absEinvminusPinvThr_TriggerSafety = 0.01; // BE CAREFUL! THIS IS UNSIGNED AND COMPARISON IS <!

  /*****************************/
  /* MVA ID Run 2 requirements */
  /*****************************/

  constexpr float etaThrLow_Gap = 1.4442;
  constexpr float etaThrHigh_Gap = 1.566;

  constexpr float isoThr_TopMVAany_I1 = 0.4;

  constexpr int maxMissingHits_TopMVAany_Run2_UL = 1;
  constexpr float dxyThr_TopMVAany_Run2_UL = 0.05;
  constexpr float dzThr_TopMVAany_Run2_UL = 0.1;
  constexpr float sip3dThr_TopMVAany_Run2_UL = 8.;
  constexpr float sieieThr_barrel_TopMVAany_Run2_UL = 0.011;
  constexpr float sieieThr_endcap_TopMVAany_Run2_UL = 0.03;
  constexpr float hoverEThr_TopMVAany_Run2_UL = 0.1;
  constexpr float min_EinvminusPinvThr_TopMVAany_Run2_UL = -0.04; // BE CAREFUL! THIS IS SIGNED AND COMPARISON IS >!

  constexpr float wp_vloose_TopMVA_Run2_UL = 0.20;
  constexpr float wp_loose_TopMVA_Run2_UL = 0.41;
  constexpr float wp_medium_TopMVA_Run2_UL = 0.64;
  constexpr float wp_tight_TopMVA_Run2_UL = 0.81;

  constexpr float wp_vloose_TopMVAv2_Run2_UL = 0.59;
  constexpr float wp_loose_TopMVAv2_Run2_UL = 0.81;
  constexpr float wp_medium_TopMVAv2_Run2_UL = 0.90;
  constexpr float wp_tight_TopMVAv2_Run2_UL = 0.94;

  void storeMVAScores(ElectronObject& part);
  float computeMVAScore(ElectronObject& part, SelectionType const& type);

  /****************/
  /* Common stuff */
  /****************/

  float getIsolationDRmax(ElectronObject const& part);

  void setSelectionBits(ElectronObject& part);
}


#endif
