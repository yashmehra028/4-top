#include <cassert>
#include <cmath>

#include "PhotonSelectionHelpers.h"


namespace PhotonSelectionHelpers{
  bool testLooseId(PhotonObject const& part);
  bool testTightId(PhotonObject const& part);

  bool testLooseIso(PhotonObject const& part);
  bool testTightIso(PhotonObject const& part);

  bool testKin(PhotonObject const& part);

  bool testPreselectionLoose(PhotonObject const& part);
  bool testPreselectionTight(PhotonObject const& part);
}



float PhotonSelectionHelpers::getIsolationDRmax(PhotonObject const& /*part*/){ return 0.3; }

bool PhotonSelectionHelpers::testLooseId(PhotonObject const& part){ return part.extras.cutBased; }
bool PhotonSelectionHelpers::testTightId(PhotonObject const& part){ return testLooseId(part); }

bool PhotonSelectionHelpers::testLooseIso(PhotonObject const& part){ return true; }
bool PhotonSelectionHelpers::testTightIso(PhotonObject const& part){ return testLooseIso(part); }

bool PhotonSelectionHelpers::testKin(PhotonObject const& part){
  return (part.pt()>=ptThr && std::abs(part.eta())<etaThr);
}

bool PhotonSelectionHelpers::testPreselectionLoose(PhotonObject const& part){
  return (
    part.testSelectionBit(kKinOnly)
    &&
    testLooseId(part)
    &&
    testLooseIso(part)
    );
}
bool PhotonSelectionHelpers::testPreselectionTight(PhotonObject const& part){
  return (
    part.testSelectionBit(kKinOnly)
    &&
    testTightId(part)
    &&
    testTightIso(part)
    );
}
void PhotonSelectionHelpers::setSelectionBits(PhotonObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kKinOnly, testKin(part));

  part.setSelectionBit(kPreselectionLoose, testPreselectionLoose(part));
  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
}
