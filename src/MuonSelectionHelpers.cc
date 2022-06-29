#include <cassert>
#include <cmath>

#include "MuonSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"


namespace MuonSelectionHelpers{
  bool testLooseId(MuonObject const& part);
  bool testLooseIso(MuonObject const& part);

  bool testFakableId(MuonObject const& part);
  bool testFakableIso(MuonObject const& part);

  bool testTightId(MuonObject const& part);
  bool testTightIso(MuonObject const& part);

  bool testTightCharge(MuonObject const& part);
  bool testKin(MuonObject const& part);

  bool testPreselectionLoose(MuonObject const& part);
  bool testPreselectionFakeable(MuonObject const& part);
  bool testPreselectionTight(MuonObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


float MuonSelectionHelpers::getIsolationDRmax(MuonObject const& part){
  return (10. / std::min(std::max(part.uncorrected_pt(), 50.), 200.));
}

bool MuonSelectionHelpers::testLooseId(MuonObject const& part){
  return part.extras.looseId && part.extras.dxy<dxyThr && part.extras.dz<dzThr;
}
bool MuonSelectionHelpers::testFakableId(MuonObject const& part){
  double const part_pt = part.pt();
  return part.extras.mediumId && part.extras.dxy<dxyThr && part.extras.dz<dzThr && part.extras.sip3d<sip3dThr && part.extras.ptErr/part_pt<track_reco_qualityThr;
}
bool MuonSelectionHelpers::testTightId(MuonObject const& part){
  double const part_pt = part.pt();
  return part.extras.mediumId && part.extras.dxy<dxyThr && part.extras.dz<dzThr && part.extras.sip3d<sip3dThr && part.extras.ptErr/part_pt<track_reco_qualityThr;
}

bool MuonSelectionHelpers::testLooseIso(MuonObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
}
bool MuonSelectionHelpers::testFakableIso(MuonObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
}
bool MuonSelectionHelpers::testTightIso(MuonObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_medium_I1);
}

bool MuonSelectionHelpers::testKin(MuonObject const& part){
  return (part.pt() >= ptThr_cat0 && std::abs(part.eta()) < etaThr_cat2);
}

bool MuonSelectionHelpers::testPreselectionLoose(MuonObject const& part){
  return (
    testLooseId(part)
    &&
    testLooseIso(part)
    &&
    testKin(part)
    );
}
bool MuonSelectionHelpers::testPreselectionFakeable(MuonObject const& part){
  return (
    testFakableId(part)
    &&
    testFakableIso(part)
    &&
    testKin(part)
    );
}
bool MuonSelectionHelpers::testPreselectionTight(MuonObject const& part){
  return (
    testTightId(part)
    &&
    testTightIso(part)
    &&
    testKin(part)
    );
}

void MuonSelectionHelpers::setSelectionBits(MuonObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kPreselectionLoose, testPreselectionLoose(part));
  part.setSelectionBit(kPreselectionFakeable, testPreselectionFakeable(part));
  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
}
