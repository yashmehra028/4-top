#include <cassert>
#include <cmath>

#include "ElectronSelectionHelpers.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"

#include "selection_tools.h" // for raw_mvaFall17V2noIso

// These are functions hidden from the user
namespace ElectronSelectionHelpers{

  bool testLooseIdNoIsoTrig(ElectronObject const& part);
  bool testLooseIsoNoIsoTrig(ElectronObject const& part);

  bool testLooseIdIsoTrig(ElectronObject const& part);
  bool testLooseIsoIsoTrig(ElectronObject const& part);

  bool testLooseKin(ElectronObject const& part);

  bool testTightId(ElectronObject const& part);
  bool testTightIso(ElectronObject const& part);
  bool testTightKin(ElectronObject const& part);

  bool testPreselectionLooseNoIsoTrig(ElectronObject const& part);
  bool testPreselectionLooseIsoTrig(ElectronObject const& part);
  bool testPreselectionTight(ElectronObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


float ElectronSelectionHelpers::getIsolationDRmax(ElectronObject const& part){
  return (10. / std::min(std::max(part.uncorrected_pt(), 50.), 200.));
}

float ElectronSelectionHelpers::relMiniIso(ElectronObject const& part){ return part.extras.miniPFRelIso_all; }

float ElectronSelectionHelpers::computeIso(ElectronObject const& part){
	return ElectronSelectionHelpers::relMiniIso(part);
}

bool ElectronSelectionHelpers::testConversionSafe(ElectronObject const& part){ return part.extras.conv_vtx_flag; }

bool ElectronSelectionHelpers::testLooseIdIsoTrig(ElectronObject const& part){
  // TODO: Check AN and implement based on the year
}

bool ElectronSelectionHelpers::testLooseIdNoIsoTrig(ElectronObject const& part){
  // TODO: Check AN and implement based on the year
}

bool ElectronSelectionHelpers::testTightId(ElectronObject const& part){
  // TODO: Check AN and implement based on the year
}

bool ElectronSelectionHelpers::testLooseIsoNoIsoTrig(ElectronObject const& part){
  // TODO: MVA does not have iso, apply a light isolation cut
}

bool ElectronSelectionHelpers::testLooseIsoIsoTrig(ElectronObject const& part){
  // TODO: MVA does not have iso, apply loose isolation following the AN
}

bool ElectronSelectionHelpers::testTightIso(ElectronObject const& part){
  // TODO: MVA does not have iso, apply tight isolation following the AN
}

bool ElectronSelectionHelpers::testLooseKin(ElectronObject const& part){
  return (part.pt()>=ptThr_skim_loose && std::abs(part.eta())<etaThr_skim_loose);
}

bool ElectronSelectionHelpers::testTightKin(ElectronObject const& part){
  return (part.pt()>=ptThr_skim_tight && std::abs(part.eta())<etaThr_skim_tight);
}

bool ElectronSelectionHelpers::testPreselectionLooseIsoTrig(ElectronObject const& part){
  return (
    testLooseIdIsoTrig(part) 
    && 
    testLooseIsoIsoTrig(part) 
    &&
    testLooseKin(part) 
  )
}

bool ElectronSelectionHelpers::testPreselectionLooseNoIsoTrig(ElectronObject const& part){
  return (
    testLooseIdNoIsoTrig(part) 
    && 
    testLooseIsoNoIsoTrig(part) 
    &&
    testLooseKin(part) 
  )
}

bool ElectronSelectionHelpers::testPreselectionTight(ElectronObject const& part){
  return (
    testTightID(part) 
    && 
    testTightIso(part) 
    &&
    testTightKin(part) 
  )
}

void ElectronSelectionHelpers::setSelectionBits(ElectronObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kPreselection_loose_NoIsoTrig, testPreselectionLooseNoIsoTrig(part));
  part.setSelectionBit(kPreselection_loose_IsoTrig, testPreselectionLooseIsoTrig(part));
  part.setSelectionBit(kPreselection_tight, testPreselectionTight(part));
}
