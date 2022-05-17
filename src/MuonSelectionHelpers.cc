#include <cassert>
#include <cmath>

#include "MuonSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"

#include "selection_tools.h" // for raw_mvaFall17V2noIso

// These are functions hidden from the user
namespace MuonSelectionHelpers{

  bool testLooseId(MuonObject const& part);
  bool testLooseIso(MuonObject const& part);
  bool testLooseImpactParameter(MuonObject const& part);

  bool testFakableID(MuonObject const& part);
  bool testFakableIso(MuonObject const& part);
  bool testFakableImpactParameter(MuonObject const& part);

  bool testTightID(MuonObject const& part);
  bool testTightIso(MuonObject const& part);
  bool testTightImpactParameter(MuonObject const& part);

  bool testTightCharge(MuonObject const& part);
  bool testKin(MuonObject const& part);

  bool testPreselectionLoose(MuonObject const& part);
  bool testPreselectionFakable(MuonObject const& part);
  bool testPreselectionTight(MuonObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;

bool MuonSelectionHelpers::testLooseId(MuonObject const& part){
  // TODO: isn't this kinematic? why are we doing this here?
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());

  // if pt is too small or eta is too large for tracking
  if (part_pt < ptThr_cat0 || part_eta >= etaThr_cat2) return false;

  // TODO: is this it? why do we have stuff above?
  return part.extras.looseId();
}

bool MuonSelectionHelpers::testFakableId(MuonObject const& part){
  // TODO: isn't this kinematic? why are we doing this here?
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());

  // if pt is too small or eta is too large for tracking
  if (part_pt < ptThr_cat0 || part_eta >= etaThr_cat2) return false;

  // TODO: is this it? why do we have stuff above?
  return part.extras.mediumId();
}

bool MuonSelectionHelpers::testTightId(MuonObject const& part){
  // TODO: isn't this kinematic? why are we doing this here?
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());

  // if pt is too small or eta is too large for tracking
  if (part_pt < ptThr_cat0 || part_eta >= etaThr_cat2) return false;

  // TODO: is this it? why do we have stuff above?
  return part.extras.mediumId();
}

// TODO: is ptRatio and ptRel used for muons? see line 337 of AN2018_062_v17
bool MuonSelectionHelpers::testLooseIso(MuonObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_loose_I1) && ((part.extras.ptRatio > isoThr_loose_I2) || (part.extras.ptRel > isoThr_loose_I3)); 
}

// TODO: is ptRatio and ptRel used for muons? see line 337 of AN2018_062_v17
bool MuonSelectionHelpers::testFakableIso(MuonObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_loose_I1) && ((part.extras.ptRatio > isoThr_loose_I2) || (part.extras.ptRel > isoThr_loose_I3)); 
}

// TODO: is ptRatio and ptRel used for muons? see line 337 of AN2018_062_v17
bool MuonSelectionHelpers::testTightIso(MuonObject const& part){
  return (part.extras.miniPFRelIso_all < isoThr_medium_I1) && ((part.extras.ptRatio > isoThr_medium_I2) || (part.extras.ptRel > isoThr_medium_I3)); 
}

// TODO: rethink kinematic threshold
bool MuonSelectionHelpers::testKin(MuonObject const& part){
  return (part.pt() >= ptThr_cat0 && std::abs(part.eta()) < etaThr_cat2);
}

bool MuonSelectionHelpers::testLooseImpactParameter(MuonObject const& part){
  // TODO: fix the inequality
  return ( (part.extras.dxy() < dxyThr) && part.extras.dz() < dzThr);
}

bool MuonSelectionHelpers::testFakableImpactParameter(MuonObject const& part){
  return ( (part.extras.dxy() < dxyThr) && (part.extras.dz() < dzThr) && (part.extras.sip3d() < sip3dThr) );
}

bool MuonSelectionHelpers::testTightImpactParameter(MuonObject const& part){
  return ( (part.extras.dxy() < dxyThr) && (part.extras.dz() < dzThr) && (part.extras.sip3d() < sip3dThr) );
}

bool MuonSelectionHelpers::testTightCharge(MuonObject const& part){
  // from line 320 of AN2018_062_v17
  return ( (part.ptErr() / part.pt()) < track_reco_qualityThr )
}

bool MuonSelectionHelpers::testPreselectionLoose(MuonObject const& part){
  return (
    testLooseId(part) 
    && 
    testLooseIso(part) 
    &&
    testLooseImpactParameter(part)
    &&
    testKin(part) 
  )
}

bool MuonSelectionHelpers::testPreselectionFakable(MuonObject const& part){
  return (
    testFakableID(part) 
    && 
    testFakableIso(part) 
    &&
    testFakableImpactParameter(part)
    &&
    testFakableCharge(part) 
    &&
    testKin(part) 
  )
}

bool MuonSelectionHelpers::testPreselectionTight(MuonObject const& part){
  return (
    testTightID(part) 
    && 
    testTightIso(part) 
    &&
    testTightImpactParameter(part)
    &&
    testTightCharge(part) 
    &&
    testKin(part) 
  )
}

void MuonSelectionHelpers::setSelectionBits(MuonObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kPreselection_loose, testPreselectionLoose(part));
  part.setSelectionBit(kPreselection_fakable, testPreselectionFakable(part));
  part.setSelectionBit(kPreselection_tight, testPreselectionTight(part));
}
