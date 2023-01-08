#ifndef JESRHELPERS_H
#define JESRHELPERS_H

#include <vector>
#include "TString.h"


namespace JESRHelpers{
  enum JetType{
    kAK4PFCHS,
    kAK4PFPuppi,
    kAK8PFPuppi,
    nJetTypes
  };

  TString getJetTypeName(JetType type);

  TString getJESFilePath(bool isMC);
  std::vector<TString> getJESFileNames(JetType type, bool isMC);
  TString getJESUncertaintyFileName(JetType type, bool isMC);

  // JER only for the MC
  TString getJERFilePath();
  TString getJERPtFileName(JetType type);
  TString getJERPhiFileName(JetType type);
  TString getJERSFFileName(JetType type);

  enum JERVariation{
    kJERNominal = 0,
    kJERDown,
    kJERUp,
    nJERVariations
  };
  enum JERBinning{
    JetPt = 0,
    JetEta,
    JetAbsEta,
    JetE,
    JetArea,
    Mu,
    Rho,
    NPV
  };

}


#endif
