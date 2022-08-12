#ifndef MUONSELECTIONHELPERS_H
#define MUONSELECTIONHELPERS_H

#include <vector>
#include <memory>
#include "MuonObject.h"
#include "IvyXGBoostInterface.h"


namespace MuonSelectionHelpers{
  enum SelectionBits{
    kKinOnly,

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
  constexpr float track_reco_qualityThr = 0.2; // Line 320 of AN2018_062_v17, pTerr/pT

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
  constexpr float isoThr_medium_I1_Phase0Tracker = 0.16;
  constexpr float isoThr_medium_I2_Phase0Tracker = 0.76;
  constexpr float isoThr_medium_I3_Phase0Tracker = 7.2;
  constexpr float isoThr_medium_I1_Phase1Tracker = 0.11;
  constexpr float isoThr_medium_I2_Phase1Tracker = 0.74;
  constexpr float isoThr_medium_I3_Phase1Tracker = 6.8;

  // Impact parameter thresholds
  constexpr float dxyThr = 0.05; 
  constexpr float dzThr = 0.1; 
  constexpr float sip3dThr = 4.; 

  /*****************************/
  /* MVA ID Run 2 requirements */
  /*****************************/

  constexpr float isoThr_TopMVAany_I1 = 0.4;

  constexpr float dxyThr_TopMVAany_Run2_UL = 0.05;
  constexpr float dzThr_TopMVAany_Run2_UL = 0.1;
  constexpr float sip3dThr_TopMVAany_Run2_UL = 8.;

  constexpr float wp_vloose_TopMVA_Run2_UL = 0.20;
  constexpr float wp_loose_TopMVA_Run2_UL = 0.41;
  constexpr float wp_medium_TopMVA_Run2_UL = 0.64;
  constexpr float wp_tight_TopMVA_Run2_UL = 0.81;

  constexpr float wp_vloose_TopMVAv2_Run2_UL = 0.59;
  constexpr float wp_loose_TopMVAv2_Run2_UL = 0.81;
  constexpr float wp_medium_TopMVAv2_Run2_UL = 0.90;
  constexpr float wp_tight_TopMVAv2_Run2_UL = 0.94;

  void storeMVAScores(MuonObject& part);
  float computeMVAScore(MuonObject& part, SelectionType const& type);

  /****************/
  /* Common stuff */
  /****************/

  float getIsolationDRmax(MuonObject const& part);

  void setSelectionBits(MuonObject& part);
}


#endif
