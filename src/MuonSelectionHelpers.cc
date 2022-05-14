#include <cassert>
#include <cmath>

#include "MuonSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"

#include "selection_tools.h" // for raw_mvaFall17V2noIso

// These are functions hidden from the user
namespace MuonSelectionHelpers{

  bool testLooseIdNoIsoTrig(MuonObject const& part);
  bool testLooseIdIsoTrig(MuonObject const& part);
  bool testLooseIso(MuonObject const& part);
  bool testLooseKin(MuonObject const& part);

  bool testMediumIso(MuonObject const& part);

  bool testTightId(MuonObject const& part);
  bool testTightIso(MuonObject const& part);
  bool testTightKin(MuonObject const& part);

  bool testPreselectionLooseNoIsoTrig(MuonObject const& part);
  bool testPreselectionLooseIsoTrig(MuonObject const& part);
  bool testPreselectionMediumNoIsoTrig(MuonObject const& part);
  bool testPreselectionMediumIsoTrig(MuonObject const& part);
  bool testPreselectionTight(MuonObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


float MuonSelectionHelpers::getIsolationDRmax(MuonObject const& part){
  return (10. / std::min(std::max(part.uncorrected_pt(), 50.), 200.));
}

// TODO: why do we need this?
float MuonSelectionHelpers::relMiniIso(MuonObject const& part){ return part.extras.miniPFRelIso_all; }

// TODO: why do we need this?
float MuonSelectionHelpers::computeIso(MuonObject const& part){
	return MuonSelectionHelpers::relMiniIso(part);
}

bool MuonSelectionHelpers::testLooseIdIsoTrig(MuonObject const& part){
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
  switch(ptcat) {
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

  float const raw_mva = raw_mvaFall17V2noIso(part.extras.mvaFall17V2noIso);
  return raw_mva >= wpcut;
}

bool MuonSelectionHelpers::testLooseIdNoIsoTrig(MuonObject const& part){
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
  switch(ptcat) {
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

  float const raw_mva = raw_mvaFall17V2noIso(part.extras.mvaFall17V2noIso);
  return raw_mva >= wpcut;
}

bool MuonSelectionHelpers::testTightId(MuonObject const& part){
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
  switch(ptcat) {
    case 1:
      wpcutCoeffB = (etacat == 0 ? 0.112 : (etacat == 1 ? 0.060 : 0.087)); 
    case 2: // will run even for case 1
      wpcutCoeffA = (etacat == 0 ? 4.277 : (etacat == 1 ? 3.152 : 2.359)); 
      break;

    default:
      return false;
  }
  double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

  float const raw_mva = raw_mvaFall17V2noIso(part.extras.mvaFall17V2noIso);
  return raw_mva >= wpcut;
}


bool MuonSelectionHelpers::testLooseIso(MuonObject const& part){
  return (part.extras.Muon_miniPFRelIso_all < isoThr_loose_I1) && ((part.extras.ptRatio > isoThr_loose_I2) || (part.extras.ptRel > isoThr_loose_I3)); 
}

bool MuonSelectionHelpers::testMediumIso(MuonObject const& part){
  return (part.extras.Muon_miniPFRelIso_all < isoThr_medium_I1) && ((part.extras.ptRatio > isoThr_medium_I2) || (part.extras.ptRel > isoThr_medium_I3)); 
}

bool MuonSelectionHelpers::testTightIso(MuonObject const& part){
  return (part.extras.Muon_miniPFRelIso_all < isoThr_tight_I1) && ((part.extras.ptRatio > isoThr_tight_I2) || (part.extras.ptRel > isoThr_tight_I3)); 
}

// TODO: rethink kinematic threshold
bool MuonSelectionHelpers::testLooseKin(MuonObject const& part){
  return (part.pt() >= ptThr_cat0 && std::abs(part.eta()) < etaThr_cat2);
}

// TODO: rethink kinematic threshold
bool MuonSelectionHelpers::testMediumKin(MuonObject const& part){
  return (part.pt() >= ptThr_cat0 && std::abs(part.eta()) < etaThr_cat2);
}

// TODO: rethink kinematic threshold
bool MuonSelectionHelpers::testTightKin(MuonObject const& part){
  return (part.pt() >= ptThr_cat0 && std::abs(part.eta()) < etaThr_cat2);
}

bool MuonSelectionHelpers::testPreselectionLooseIsoTrig(MuonObject const& part){
  return (
    testLooseIdIsoTrig(part) 
    && 
    testLooseIso(part) 
    &&
    testLooseKin(part) 
  )
}

bool MuonSelectionHelpers::testPreselectionLooseNoIsoTrig(MuonObject const& part){
  return (
    testLooseIdNoIsoTrig(part) 
    && 
    testLooseIso(part) 
    &&
    testLooseKin(part) 
  )
}

bool MuonSelectionHelpers::testPreselectionMediumNoIsoTrig(MuonObject const& part){
  return (
    testMediumIdNoIsoTrig(part) 
    && 
    testMediumIso(part) 
    &&
    testMediumKin(part) 
  )
}

bool MuonSelectionHelpers::testPreselectionMediumIsoTrig(MuonObject const& part){
  return (
    testMediumIdIsoTrig(part) 
    && 
    testMediumIso(part) 
    &&
    testMediumKin(part) 
  )
}

bool MuonSelectionHelpers::testPreselectionTight(MuonObject const& part){
  return (
    testTightID(part) 
    && 
    testTightIso(part) 
    &&
    testTightKin(part) 
  )
}

void MuonSelectionHelpers::setSelectionBits(MuonObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kPreselection_loose_NoIsoTrig, testPreselectionLooseNoIsoTrig(part));
  part.setSelectionBit(kPreselection_loose_IsoTrig, testPreselectionLooseIsoTrig(part));
  part.setSelectionBit(kPreselection_medium_IsoTrig, testPreselectionMediumIsoTrig(part));
  part.setSelectionBit(kPreselection_medium_NoIsoTrig, testPreselectionMediumNoIsoTrig(part));
  part.setSelectionBit(kPreselection_tight, testPreselectionTight(part));
}
