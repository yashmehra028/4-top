#include <cassert>
#include <cmath>
#include <limits>

#include "SamplesCore.h"
#include "HadronicTauSelectionHelpers.h"
#include "AK4JetObject.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


namespace HadronicTauSelectionHelpers{
  SelectionType selection_type = kDeepTau_v2p1_Run2;

  bool testKin_Skim(HadronicTauObject const& part);
  bool testKin(HadronicTauObject const& part);

  bool testPreselectionLoose(HadronicTauObject const& part);
  bool testPreselectionTight(HadronicTauObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


void HadronicTauSelectionHelpers::setSelectionTypeByName(TString stname){
  SelectionType type = kDeepTau_v2p1_Run2;
  if (stname=="DeepTau_v2p1") type = kDeepTau_v2p1_Run2;
  else{
    IVYerr << "HadronicTauSelectionHelpers::setSelectionTypeByName: Type name " << stname << " is not recognized." << endl;
    assert(0);
  }
  setSelectionType(type);
}
void HadronicTauSelectionHelpers::setSelectionType(SelectionType const& type){ selection_type = type; }

float HadronicTauSelectionHelpers::getIsolationDRmax(HadronicTauObject const& part){
  return 0.5;
}

bool HadronicTauSelectionHelpers::testKin_Skim(HadronicTauObject const& part){
  return (part.pt() >= ptThr_skim && std::abs(part.eta()) < etaThr);
}
bool HadronicTauSelectionHelpers::testKin(HadronicTauObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Skim)
    );
}

// FIXME: Placeholders for now
bool HadronicTauSelectionHelpers::testPreselectionLoose(HadronicTauObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Skim)
    );
}
bool HadronicTauSelectionHelpers::testPreselectionTight(HadronicTauObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Skim)
    );
}

void HadronicTauSelectionHelpers::setSelectionBits(HadronicTauObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kKinOnly_Skim, testKin_Skim(part));
  part.setSelectionBit(kKinOnly, testKin(part));

  part.setSelectionBit(kPreselectionLoose, testPreselectionLoose(part));
  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
}
