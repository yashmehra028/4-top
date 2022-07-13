#include <cassert>
#include <cmath>
#include "AK4JetSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "SamplesCore.h"


namespace AK4JetSelectionHelpers{
  bool testJetId(AK4JetObject const& part);
  bool testId(AK4JetObject const& part);
  bool testBTag(AK4JetObject const& part);
  bool testKin(AK4JetObject const& part);

  bool testPreselectionTight(AK4JetObject const& part);
  bool testPreselectionTight_BTagged(AK4JetObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


bool AK4JetSelectionHelpers::testJetId(AK4JetObject const& part){
  return (part.extras.jetId>jetIdThr);
}
bool AK4JetSelectionHelpers::testId(AK4JetObject const& part){
  return part.testSelectionBit(kJetIdOnly); // Could add PU jet ID here as well...
}
bool AK4JetSelectionHelpers::testBTag(AK4JetObject const& part){
  auto const& dp = SampleHelpers::getDataPeriod();
  auto const& dy = SampleHelpers::getDataYear();

  float etaThr = -1;
  if (dy<=2016) etaThr = etaThr_btag_Phase0Tracker;
  else etaThr = etaThr_btag_Phase1Tracker;

  float deepFlavThr = 1e9;
  if (dy==2016) deepFlavThr = (SampleHelpers::isAPV2016Affected(dp) ? deepFlavThr_2016_APV : deepFlavThr_2016_NonAPV);
  else if (dy==2017) deepFlavThr = deepFlavThr_2017;
  else if (dy==2018) deepFlavThr = deepFlavThr_2018;
  else{
    IVYerr << "AK4JetSelectionHelpers::testB: Data period " << dp << " is not defined." << endl;
    assert(0);
  }
  return (part.extras.btagDeepFlavB>deepFlavThr && std::abs(part.eta())<etaThr);
}
bool AK4JetSelectionHelpers::testKin(AK4JetObject const& part){
  return (part.pt()>=ptThr && std::abs(part.eta())<etaThr_common);
}

bool AK4JetSelectionHelpers::testPreselectionTight(AK4JetObject const& part){
  return (
    testId(part)
    &&
    part.testSelectionBit(kKinOnly)
    );
}
bool AK4JetSelectionHelpers::testPreselectionTight_BTagged(AK4JetObject const& part){
  return (
    part.testSelectionBit(kPreselectionTight)
    &&
    part.testSelectionBit(kBTagOnly)
    );
}

void AK4JetSelectionHelpers::setSelectionBits(AK4JetObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kJetIdOnly, testJetId(part));
  part.setSelectionBit(kBTagOnly, testBTag(part));
  part.setSelectionBit(kKinOnly, testKin(part));

  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
  part.setSelectionBit(kPreselectionTight_BTagged, testPreselectionTight_BTagged(part));
}
