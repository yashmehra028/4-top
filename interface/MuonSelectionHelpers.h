#ifndef MUONSELECTIONHELPERS_H
#define MUONSELECTIONHELPERS_H

#include <vector>
#include <memory>
#include "MuonObject.h"
#include "IvyFramework/IvyMLTools/interface/IvyXGBoostInterface.h"


namespace MuonSelectionHelpers{
  enum SelectionBits{
    kKinOnly_Skim,
    kKinOnly_Loose,
    kKinOnly_Fakeable,
    kKinOnly_Tight,

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


  /****************/
  /* Common stuff */
  /****************/
  constexpr float ptThr_skim = 5.;

  float getIsolationDRmax(MuonObject const& part);

  void setSelectionBits(MuonObject& part);


  /***********************************/
  /* Cut-based ID Run 2 requirements */
  /***********************************/

  // Kinematic pT thresholds
  constexpr float ptThr_loose = 5.;
  constexpr float ptThr_fakeable = 10.;
  constexpr float ptThr_tight = 10.;

  constexpr float ptThr_cat0 = 5.; 
  constexpr float ptThr_cat1 = 10.;
  constexpr float ptThr_cat2 = 25.;

  // Kinematic eta thresholds
  constexpr float etaThr_cat0 = 0.8; 
  constexpr float etaThr_cat1 = 1.479;
  constexpr float etaThr_cat2 = 2.4;

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

  // Impact parameter and track quality thresholds
  constexpr float dxyThr = 0.05; 
  constexpr float dzThr = 0.1; 
  constexpr float sip3dThr = 4.; 
  constexpr float track_reco_qualityThr = 0.2; // Line 320 of AN2018_062_v17, pTerr/pT


  /*****************************/
  /* MVA ID Run 2 requirements */
  /*****************************/
  constexpr float ptThr_TopMVAany_Run2_UL_loose = 10.;
  constexpr float ptThr_TopMVAany_Run2_UL_fakeable = 10.;
  constexpr float ptThr_TopMVAany_Run2_UL_tight = 10.;

  constexpr float isoThr_TopMVAany_Run2_UL_I1 = 0.4;
  constexpr float isoThr_TopMVAany_Run2_UL_Fakeable_ALT_I2 = 0.45;
  constexpr float bscoreThr_TopMVAAny_Run2_UL_Fakeable_ALT_fixed = 0.025;
  // A running bscoreThr could be computed using MuonSelectionHelpers.cc::get_bscoreThr_TopMVAAny_Run2_UL_Fakeable_ALT,
  // but we do not use it.

  constexpr float dxyThr_TopMVAany_Run2_UL = 0.05;
  constexpr float dzThr_TopMVAany_Run2_UL = 0.1;
  constexpr float sip3dThr_TopMVAany_Run2_UL = 8.;

  constexpr float wp_vloose_TopMVA_Run2_UL = 0.20;
  constexpr float wp_loose_TopMVA_Run2_UL = 0.41;
  constexpr float wp_medium_TopMVA_Run2_UL = 0.64;
  constexpr float wp_tight_TopMVA_Run2_UL = 0.81;
  constexpr float wp_mediumID_TopMVA_Run2_UL = 0.;

  constexpr float wp_vloose_TopMVAv2_Run2_UL = 0.59;
  constexpr float wp_loose_TopMVAv2_Run2_UL = 0.81;
  constexpr float wp_medium_TopMVAv2_Run2_UL = 0.90;
  constexpr float wp_tight_TopMVAv2_Run2_UL = 0.94;
  constexpr float wp_mediumID_TopMVAv2_Run2_UL = 0.;

  void storeMVAScores(MuonObject& part);
  float computeMVAScore(MuonObject const& part, SelectionType const& type);

}


#endif
