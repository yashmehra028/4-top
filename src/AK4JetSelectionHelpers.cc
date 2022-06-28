#include <cassert>
#include <cmath>
#include "AK4JetSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "SamplesCore.h"


namespace AK4JetSelectionHelpers{
  bool testPreselectionTight_JetIdOnly(AK4JetObject const& part);
  bool testPreselectionTight(AK4JetObject const& part);
  bool testPreselectionTightB(AK4JetObject const& part);

  bool testJetId(AK4JetObject const& part);
  bool testId(AK4JetObject const& part);
  bool testKin(AK4JetObject const& part);
  bool testB(AK4JetObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


bool AK4JetSelectionHelpers::testJetId(AK4JetObject const& part){
  return (part.extras.jetId>jetIdThr);
}
bool AK4JetSelectionHelpers::testId(AK4JetObject const& part){
  return AK4JetSelectionHelpers::testJetId(part);
}
bool AK4JetSelectionHelpers::testB(AK4JetObject const& part){
  auto const& dp = SampleHelpers::getDataPeriod();
  auto const& dy = SampleHelpers::getDataYear();
  float deepFlavThr = 1e9;
  if (dp=="2016_APV") deepFlavThr = deepFlavThr_2016_APV;
  else if (dp=="2016_NonAPV") deepFlavThr = deepFlavThr_2016_NonAPV;
  else if (dy==2017) deepFlavThr = deepFlavThr_2017;
  else if (dy==2018) deepFlavThr = deepFlavThr_2018;
  else{
    IVYerr << "AK4JetSelectionHelpers::testB: Data period " << dp << " is not defined." << endl;
    assert(0);
  }
  return (part.extras.btagDeepFlavB>deepFlavThr);
}
bool AK4JetSelectionHelpers::testKin(AK4JetObject const& part){
  float etaThr = -1;
  if (SampleHelpers::getDataYear()<=2016) etaThr = etaThr_btag_Phase0Tracker;
  else etaThr = etaThr_btag_Phase1Tracker;
  return (part.pt()>=ptThr && std::abs(part.eta())<etaThr);
}

bool AK4JetSelectionHelpers::testPreselectionTight_JetIdOnly(AK4JetObject const& part){ return testJetId(part); }
bool AK4JetSelectionHelpers::testPreselectionTight(AK4JetObject const& part){
  return (
    testId(part)
    &&
    testKin(part)
  );
}
bool AK4JetSelectionHelpers::testPreselectionTightB(AK4JetObject const& part){
  return (
    part.testSelectionBit(kPreselectionTight)
    &&
    testB(part)
  );
}

void AK4JetSelectionHelpers::setSelectionBits(AK4JetObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kPreselectionTight_JetIdOnly, testPreselectionTight_JetIdOnly(part));
  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
  part.setSelectionBit(kPreselectionTightB, testPreselectionTightB(part));
}
