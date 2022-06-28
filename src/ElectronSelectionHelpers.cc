#include <cassert>
#include <cmath>

#include "ElectronSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"


// These are functions hidden from the user
namespace ElectronSelectionHelpers{
  float convert_BDTscore_raw(float const& mva);

  bool testLooseId_NoIsoTrig(ElectronObject const& part);
  bool testLooseId_IsoTrig(ElectronObject const& part);
  bool testLooseIso(ElectronObject const& part);

  bool testFakeableId(ElectronObject const& part);
  bool testFakeableIso(ElectronObject const& part);

  bool testTightId(ElectronObject const& part);
  bool testTightIso(ElectronObject const& part);

  bool testKin(ElectronObject const& part);

  bool testPreselectionLoose_NoIsoTrig(ElectronObject const& part);
  bool testPreselectionLoose_IsoTrig(ElectronObject const& part);
  bool testPreselectionLoose(ElectronObject const& part);
  bool testPreselectionFakeable(ElectronObject const& part);
  bool testPreselectionTight(ElectronObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


float ElectronSelectionHelpers::convert_BDTscore_raw(float const& mva){
  return 0.5 * std::log((1. + mva)/(1. - mva)); // Unsquashed MVA value
}


float ElectronSelectionHelpers::getIsolationDRmax(ElectronObject const& part){
  return (10. / std::min(std::max(part.uncorrected_pt(), 50.), 200.));
}

float ElectronSelectionHelpers::relMiniIso(ElectronObject const& part){ return part.extras.miniPFRelIso_all; }

float ElectronSelectionHelpers::computeIso(ElectronObject const& part){
	return ElectronSelectionHelpers::relMiniIso(part);
}


bool ElectronSelectionHelpers::testLooseId_IsoTrig(ElectronObject const& part){
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());

  // if pt is too small or eta is too large for tracking
  if (part_pt < ptThr_cat0 || part_eta >= etaThr_cat2) return false;

  // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
  unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
  unsigned char etacat = 1*(part_eta >= etaThr_cat0) + 1*(part_eta >= etaThr_cat1);

  // TODO: potential bug.. WP does not match for pt == 10 on lower and upper bounds. 
  double wpcutCoeffA = 99;
  double wpcutCoeffB = 0;
  switch (ptcat){
  case 0:
    wpcutCoeffA = (etacat == 0 ? 1.320 : (etacat == 1 ? 0.192 : 0.362));
    break;
  case 1:
    wpcutCoeffB = (etacat == 0 ? 0.066 : (etacat == 1 ? 0.033 : 0.053));
  case 2: // will run even for case 1
    wpcutCoeffA = (etacat == 0 ? 1.204 : (etacat == 1 ? 0.084 : -0.123));
    break;
  default:
    return false;
  }
  double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

  float const raw_mva = convert_BDTscore_raw(part.extras.mvaFall17V2noIso);
  return raw_mva >= wpcut;
}
bool ElectronSelectionHelpers::testLooseId_NoIsoTrig(ElectronObject const& part){
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());

  // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
  // if pt is too small or eta is too large for tracking
  if (part_pt < ptThr_cat0 || part_eta >= etaThr_cat2) return false;

  unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
  unsigned char etacat = 1*(part_eta >= etaThr_cat0) + 1*(part_eta >= etaThr_cat1);

  // TODO: potential bug.. WP does not match for pt == 10 on lower and upper bounds. 
  // see AN2018_062_v17 lines 298-303 for description of mva discriminant
  double wpcutCoeffA = 99;
  double wpcutCoeffB = 0;
  switch (ptcat){
  case 0:
    wpcutCoeffA = (etacat == 0 ? 0.053 : (etacat == 1 ? -0.434 : -0.956));
    break;
  case 1:
    wpcutCoeffB = (etacat == 0 ? 0.062 : (etacat == 1 ? 0.038 : 0.042));
  case 2: // will run even for case 1
    wpcutCoeffA = (etacat == 0 ? -0.106 : (etacat == 1 ? -0.769 : -1.461));
    break;
  default:
    return false;
  }
  double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

  float const raw_mva = convert_BDTscore_raw(part.extras.mvaFall17V2noIso);
  return raw_mva >= wpcut;
}
bool ElectronSelectionHelpers::testFakeableId(ElectronObject const& part){
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());

  // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
  // if pt is too small or eta is too large for tracking
  if (part_pt < ptThr_cat1 || part_eta >= etaThr_cat2) return false;

  unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
  unsigned char etacat = 1*(part_eta >= etaThr_cat0) + 1*(part_eta >= etaThr_cat1);

  // see AN2018_062_v17 lines 298-303 for description of mva discriminant
  double wpcutCoeffA = 99;
  double wpcutCoeffB = 0;
  switch (ptcat){
  case 1:
    wpcutCoeffB = (etacat == 0 ? 0.112 : (etacat == 1 ? 0.060 : 0.087));
  case 2: // will run even for case 1
    wpcutCoeffA = (etacat == 0 ? 4.277 : (etacat == 1 ? 3.152 : 2.359));
    break;
  default:
    return false;
  }
  double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

  float const raw_mva = convert_BDTscore_raw(part.extras.mvaFall17V2noIso);
  return raw_mva >= wpcut;
}
bool ElectronSelectionHelpers::testTightId(ElectronObject const& part){
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());

  // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
  // if pt is too small or eta is too large for tracking
  if (part_pt < ptThr_cat1 || part_eta >= etaThr_cat2) return false;

  unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
  unsigned char etacat = 1*(part_eta >= etaThr_cat0) + 1*(part_eta >= etaThr_cat1);

  // see AN2018_062_v17 lines 298-303 for description of mva discriminant
  double wpcutCoeffA = 99;
  double wpcutCoeffB = 0;
  switch (ptcat){
  case 1:
    wpcutCoeffB = (etacat == 0 ? 0.112 : (etacat == 1 ? 0.060 : 0.087));
  case 2: // will run even for case 1
    wpcutCoeffA = (etacat == 0 ? 4.277 : (etacat == 1 ? 3.152 : 2.359));
    break;
  default:
    return false;
  }
  double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

  float const raw_mva = convert_BDTscore_raw(part.extras.mvaFall17V2noIso);
  return raw_mva >= wpcut;
}


bool ElectronSelectionHelpers::testLooseIso(ElectronObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
}
bool ElectronSelectionHelpers::testFakeableIso(ElectronObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
}
bool ElectronSelectionHelpers::testTightIso(ElectronObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_tight_I1);
}

bool ElectronSelectionHelpers::testKin(ElectronObject const& part){
  return (part.pt()>=ptThr_cat0 && std::abs(part.eta())<etaThr_cat2);
}

bool ElectronSelectionHelpers::testPreselectionLoose_IsoTrig(ElectronObject const& part){
  return (
    testLooseId_IsoTrig(part)
    &&
    testLooseIso(part)
    &&
    testKin(part)
    );
}
bool ElectronSelectionHelpers::testPreselectionLoose_NoIsoTrig(ElectronObject const& part){
  return (
    testLooseId_NoIsoTrig(part)
    &&
    testLooseIso(part)
    &&
    testKin(part)
    );
}
bool ElectronSelectionHelpers::testPreselectionLoose(ElectronObject const& part){
  return (
    part.testSelectionBit(kPreselectionLoose_NoIsoTrig)
    ||
    part.testSelectionBit(kPreselectionLoose_IsoTrig)
    );
}
bool ElectronSelectionHelpers::testPreselectionFakeable(ElectronObject const& part){
  return (
    testFakeableId(part)
    &&
    testFakeableIso(part)
    &&
    testKin(part)
    );
}
bool ElectronSelectionHelpers::testPreselectionTight(ElectronObject const& part){
  return (
    testTightId(part)
    &&
    testTightIso(part)
    &&
    testKin(part)
    );
}

void ElectronSelectionHelpers::setSelectionBits(ElectronObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kPreselectionLoose_NoIsoTrig, testPreselectionLoose_NoIsoTrig(part));
  part.setSelectionBit(kPreselectionLoose_IsoTrig, testPreselectionLoose_IsoTrig(part));
  part.setSelectionBit(kPreselectionLoose, testPreselectionLoose(part));
  part.setSelectionBit(kPreselectionFakeable, testPreselectionFakeable(part));
  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
}
