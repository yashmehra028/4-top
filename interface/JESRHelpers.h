#ifndef JESRHELPERS_H
#define JESRHELPERS_H

#include <vector>
#include "TString.h"


namespace JESRHelpers{
  constexpr bool doReapplyJES = false;

  enum JetType{
    kAK4PFCHS,
    kAK8PFPuppi
  };
  enum JetSystematicType{
    sNominal,
    sJESUp, sJESDn,
    sJERUp, sJERDn
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
}


#endif
