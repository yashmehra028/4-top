#ifndef HADRONICTAUSELECTIONHELPERS_H
#define HADRONICTAUSELECTIONHELPERS_H

#include <vector>
#include <memory>
#include "HadronicTauObject.h"


namespace HadronicTauSelectionHelpers{
  enum SelectionBits{
    kKinOnly_Skim,
    kKinOnly,

    kPreselectionLoose,
    kPreselectionTight,

    nSelectionBits
  };
  enum SelectionType{
    kDeepTau_v2p1_Run2 = 0
  };


  extern SelectionType selection_type;
  void setSelectionType(SelectionType const& type);
  void setSelectionTypeByName(TString stname);


  /****************/
  /* Common stuff */
  /****************/
  constexpr float ptThr_skim = 18.; // Threshold at MiniAOD level
  constexpr float etaThr = 2.3;

  float getIsolationDRmax(HadronicTauObject const& part);

  void setSelectionBits(HadronicTauObject& part);

}


#endif
