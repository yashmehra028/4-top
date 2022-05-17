#include <cassert>
#include <cmath>

#include "JetSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"

#include "selection_tools.h" // for raw_mvaFall17V2noIso

#include "NanoTools/NanoCORE/Nano.h" // for year

// These are functions hidden from the user
namespace JetSelectionHelpers{
  bool testPreselectionTight(JetObject const& part);
  bool testPreselectionTightB(JetObject const& part);

  bool testID(JetObject const& part);
  bool testKin(JetObject const& part);
  bool testB(JetObject const& part);
}

using namespace std;
using namespace IvyStreamHelpers;

bool JetSelectionHelpers::testB(JetObject const& part){
  float deepFlavThr = 0;
  switch(dataPeriod){
    case "2016_APV": 
      deepFlavThr = deepFlavThr_2016_APV;
      break;
    case "2016": 
      deepFlavThr = deepFlavThr_2016_NonAPV;
      break;
    case "2017":
      deepFlavThr = deepFlavThr_2017;
      break;
    case "2018":
      deepFlavThr = deepFlavThr_2018;
      break;
  }
  return (part.Jet_btagDeepFlavB()>deepFlavThr);
}

bool JetSelectionHelpers::testID(JetObject const& part){
  return (part.Jet_jetId()>jetIDThr);
}

bool JetSelectionHelpers::testKin(JetObject const& part){
  float etaThr = 0;
  switch(year){
    case 2016:
      etaThr = etaThr_2016;
      break;
    case 2017:
      etaThr = etaThr_2017_2018;
      break;
    case 2018:
      etaThr = etaThr_2017_2018;
      break;
  }
  return (part.pt()>=ptThr && std::abs(part.eta())<etaThr);
}

bool JetSelectionHelpers::testPreselectionTight(JetObject const& part){
  return (
    // TODO:
  )
}

bool JetSelectionHelpers::testPreselectionTightB(JetObject const& part){
  return (
    // TODO:
  )
}

void JetSelectionHelpers::setSelectionBits(JetObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
  part.setSelectionBit(kPreselectionTightB, testPreselectionTightB(part));
}
