#include <cassert>
#include <cmath>

#include "SamplesCore.h"
#include "ElectronSelectionHelpers.h"
#include "AK4JetObject.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


// These are functions hidden from the user
namespace ElectronSelectionHelpers{
  constexpr bool apply_multiiso = true;
  constexpr bool apply_triggerSafety = true;
  constexpr bool apply_EBEEGapVeto = true;

  SelectionType selection_type = kCutbased_Run2;
  bool applyMVALooseFakeableNoIsoWPs = false; // For Cutbased_Run2 ID

  // Just test if this selection is cut- or MVA-based
  bool isMVASelection(SelectionType const& type);

  // MVA reader for MVA IDs
  std::unordered_map<SelectionType, std::shared_ptr<IvyXGBoostInterface> > seltype_mvareader_map;
  void loadMVA();

  // Common functions
  bool getNanoMVANoIsoBDTWPL(ElectronObject const& part);
  float getNanoMVANoIsoBDTScore(ElectronObject const& part);
  float getNanoMVAIsoBDTScore(ElectronObject const& part);
  float convert_BDTscore_raw(float const& mva); // This is for the MVA scores recorded in NanoAOD from e/gamma standard VIDs.

  bool testTriggerSafety(ElectronObject const& part);

  bool testLooseId_Common(ElectronObject const& part);
  bool testLooseId_NoIsoTrig(ElectronObject const& part);
  bool testLooseId_IsoTrig(ElectronObject const& part);
  bool testLooseIso(ElectronObject const& part);

  bool testFakeableId_Common(ElectronObject const& part);
  bool testFakeableId_NoIsoTrig(ElectronObject const& part);
  bool testFakeableId_IsoTrig(ElectronObject const& part);
  bool testFakeableIso(ElectronObject const& part);

  bool testTightId(ElectronObject const& part);
  bool testTightIso(ElectronObject const& part);

  bool testKin_Skim(ElectronObject const& part);
  bool testKin_Loose(ElectronObject const& part);
  bool testKin_Fakeable(ElectronObject const& part);
  bool testKin_Tight(ElectronObject const& part);
  bool testKin(ElectronObject const& part);

  bool testPreselectionLoose_NoIsoTrig(ElectronObject const& part);
  bool testPreselectionLoose_IsoTrig(ElectronObject const& part);
  bool testPreselectionLoose(ElectronObject const& part);
  bool testPreselectionFakeable_NoIsoTrig(ElectronObject const& part);
  bool testPreselectionFakeable_IsoTrig(ElectronObject const& part);
  bool testPreselectionFakeable(ElectronObject const& part);
  bool testPreselectionTight(ElectronObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


void ElectronSelectionHelpers::setApplyMVALooseFakeableNoIsoWPs(bool flag){ applyMVALooseFakeableNoIsoWPs = flag; }

bool ElectronSelectionHelpers::isMVASelection(SelectionType const& type){
  switch (type){
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return true;
  default:
    return false;
  }
}

void ElectronSelectionHelpers::setSelectionTypeByName(TString stname){
  SelectionType type = kCutbased_Run2;
  if (stname=="Cutbased_Run2") type = kCutbased_Run2;
  else if (stname=="TopMVA_Run2") type = kTopMVA_Run2;
  else if (stname=="TopMVAv2_Run2") type = kTopMVAv2_Run2;
  else{
    IVYerr << "ElectronSelectionHelpers::setSelectionTypeByName: Type name " << stname << " is not recognized." << endl;
    assert(0);
  }
  setSelectionType(type);
}
void ElectronSelectionHelpers::setSelectionType(SelectionType const& type){
  if (!SampleHelpers::runConfigure && isMVASelection(type)){
    IVYerr << "ElectronSelectionHelpers::setSelectionType: SampleHelpers::configure needs to be called first when setting ML-based selection." << endl;
    assert(0);
  }
  selection_type = type;
  loadMVA();
}
void ElectronSelectionHelpers::loadMVA(){
  if (!isMVASelection(selection_type)) return;

  auto it_seltype_mvareader_map = seltype_mvareader_map.find(selection_type);
  if (it_seltype_mvareader_map==seltype_mvareader_map.end()) seltype_mvareader_map[selection_type] = std::shared_ptr<IvyXGBoostInterface>();
  auto& mvareader_xgb = seltype_mvareader_map.find(selection_type)->second;

  if (mvareader_xgb) mvareader_xgb.reset();

  TString fname;
  std::vector<TString> varnames;
  IvyMLWrapper::IvyMLDataType_t missing_entry_val = std::numeric_limits<IvyMLWrapper::IvyMLDataType_t>::quiet_NaN();
  auto const& dy = SampleHelpers::getDataYear();
  if (selection_type==kTopMVA_Run2 || selection_type==kTopMVAv2_Run2){
    fname = "el_TOP";
    if (selection_type==kTopMVAv2_Run2) fname += "v2";
    fname += "UL";
    if (dy==2016){
      auto const& dp = SampleHelpers::getDataPeriod();
      if (SampleHelpers::isAPV2016Affected(dp)) fname += "16APV";
      else fname += "16";
    }
    else if (dy==2017) fname += "17";
    else if (dy==2018) fname += "18";
    else if (dy==2022){
      fname += "18"; // FIXME: Need to be changed in the future.
      IVYout << "ElectronSelectionHelpers::loadMVA: WARNING! Using the MVA for year 2018 in place of " << dy << "." << endl;
    }
    else{
      IVYerr << "ElectronSelectionHelpers::loadMVA: Year " << dy << " is not implemented for type " << selection_type << "." << endl;
      assert(0);
    }
    fname += "_XGB.weights.bin";

    fname = ANALYSISPKGDATAPATH + "external/TopLeptonMVA/" + fname;
    varnames = std::vector<TString>{
      "pt",
      "eta",
      "jetNDauCharged",
      "miniPFRelIso_chg",
      "miniPFRelIso_diff_all_chg", // = miniPFRelIso_all - miniPFRelIso_chg
      "jetPtRelv2",
      "jetPtRatio", // = 1/(jetRelIso+1)
      "pfRelIso03_all",
      "ak4jet:btagDeepFlavB", // B-tagging discriminant score
      "sip3d",
      "log_abs_dxy", // = log(|dxy|)
      "log_abs_dz" // = log(|dz|)
    };
    if (dy>=2016 && dy<=2018) varnames.push_back("mvaFall17V2noIso");
    else if (dy==2022) varnames.push_back("mvaNoIso");
    else{
      IVYerr << "ElectronSelectionHelpers::loadMVA: Could not determine the name of the no-iso. MVA variable name for year " << dy << "." << endl;
      assert(0);
    }
    if (selection_type==kTopMVAv2_Run2) varnames.push_back("lostHits");
    // No need to set missing_entry_val
  }
  else{
    IVYerr << "ElectronSelectionHelpers::loadMVA: Selection " << selection_type << " is not implemented." << endl;
    assert(0);
  }

  mvareader_xgb = std::make_shared<IvyXGBoostInterface>();
  mvareader_xgb->build(fname, varnames, missing_entry_val, 1);
}
void ElectronSelectionHelpers::storeMVAScores(ElectronObject& part){
  for (auto const& pp:seltype_mvareader_map){ if (isMVASelection(pp.first)) part.setExternalMVAScore(static_cast<int>(pp.first), computeMVAScore(part, pp.first)); }
}
float ElectronSelectionHelpers::computeMVAScore(ElectronObject const& part, SelectionType const& type){
  float res = -999;
  if (!isMVASelection(type) || !part.testSelectionBit(kKinOnly_Loose) || part.getExternalMVAScore(static_cast<int>(type), res)) return res;

  auto it_seltype_mvareader_map = seltype_mvareader_map.find(type);
  if (it_seltype_mvareader_map==seltype_mvareader_map.end()){
    IVYerr << "ElectronSelectionHelpers::computeMVAScore: External MVA for selection type " << type << " is not set up." << endl;
    assert(0);
  }
  auto& mvareader_xgb = seltype_mvareader_map.find(type)->second;
  if (mvareader_xgb){
    std::unordered_map<TString, IvyMLWrapper::IvyMLDataType_t> input_vars;

    auto const& vnames = mvareader_xgb->getVariableNames();
    for (auto const& vname:vnames){
      if (vname=="pt") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.pt());
      else if (vname=="eta") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.eta());
      else if (vname=="miniPFRelIso_diff_all_chg") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.extras.miniPFRelIso_all - part.extras.miniPFRelIso_chg);
      else if (vname=="jetPtRatio") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(1./(part.extras.jetRelIso+1.));
      else if (vname=="log_abs_dxy") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(std::log(std::abs(part.extras.dxy)));
      else if (vname=="log_abs_dz") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(std::log(std::abs(part.extras.dz)));
      else if (vname=="ak4jet:btagDeepFlavB"){
        IvyMLWrapper::IvyMLDataType_t score = 0;
        AK4JetObject* mother = nullptr;
        for (auto const& mom:part.getMothers()){
          mother = dynamic_cast<AK4JetObject*>(mom);
          if (mother) break;
        }
        if (mother) score = static_cast<IvyMLWrapper::IvyMLDataType_t>(mother->extras.btagDeepFlavB);
        input_vars[vname] = score;
      }
#define ELECTRON_VARIABLE(TYPE, NAME, DEFVAL) else if (vname==#NAME) input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.extras.NAME);
      ELECTRON_EXTRA_VARIABLES
#undef ELECTRON_VARIABLE
      else{
        IVYerr << "ElectronSelectionHelpers::computeMVAScore: Input variable name " << vname << " does not match to a corresponding variable. Please fix the implementation." << endl;
        assert(0);
      }
    }

    mvareader_xgb->eval(input_vars, res);
  }
  return res;
}


bool ElectronSelectionHelpers::getNanoMVANoIsoBDTWPL(ElectronObject const& part){
  auto const& dy = SampleHelpers::getDataYear();
  if (dy>=2015 && dy<=2018) return part.extras.mvaFall17V2noIso_WPL;
  else if (dy==2022) return part.extras.mvaNoIso_WPL;
  else{
    IVYerr << "ElectronSelectionHelpers::getNanoMVANoIsoBDTWPL: Could not determine the name of the no-iso. MVA variable for year " << dy << "." << endl;
    assert(0);
  }
  return -99;
}
float ElectronSelectionHelpers::getNanoMVANoIsoBDTScore(ElectronObject const& part){
  auto const& dy = SampleHelpers::getDataYear();
  if (dy>=2015 && dy<=2018) return part.extras.mvaFall17V2noIso;
  else if (dy==2022) return part.extras.mvaNoIso;
  else{
    IVYerr << "ElectronSelectionHelpers::getNanoMVANoIsoBDTScore: Could not determine the name of the no-iso. MVA variable for year " << dy << "." << endl;
    assert(0);
  }
  return -99;
}
float ElectronSelectionHelpers::getNanoMVAIsoBDTScore(ElectronObject const& part){
  auto const& dy = SampleHelpers::getDataYear();
  if (dy>=2015 && dy<=2018) return part.extras.mvaFall17V2Iso;
  else if (dy==2022) return part.extras.mvaIso;
  else{
    IVYerr << "ElectronSelectionHelpers::getNanoMVAIsoBDTScore: Could not determine the name of the iso. MVA variable for year " << dy << "." << endl;
    assert(0);
  }
  return -99;
}


float ElectronSelectionHelpers::convert_BDTscore_raw(float const& mva){
  return 0.5 * std::log((1. + mva)/(1. - mva)); // Unsquashed MVA value
}

float ElectronSelectionHelpers::getIsolationDRmax(ElectronObject const& part){
  return (10. / std::min(std::max(part.uncorrected_pt(), 50.), 200.));
}


bool ElectronSelectionHelpers::testTriggerSafety(ElectronObject const& part){
  if (!apply_triggerSafety) return true;

  double const part_etaSC = std::abs(part.etaSC());

  switch (selection_type){
  case kCutbased_Run2:
    return (
      part.extras.hoe<hoverEThr_TriggerSafety
      &&
      std::abs(part.extras.eInvMinusPInv)<absEinvminusPinvThr_TriggerSafety
      &&
      part.extras.sieie<(part_etaSC<etaThr_cat1 ? sieieThr_barrel_TriggerSafety : sieieThr_endcap_TriggerSafety)
      );
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return true;
  default:
    IVYerr << "ElectronSelectionHelpers::testTriggerSafety: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testLooseId_Common(ElectronObject const& part){
  switch (selection_type){
  case kCutbased_Run2:
    return (
      part.extras.lostHits<=maxMissingHits_loose
      &&
      std::abs(part.extras.dxy)<dxyThr
      &&
      std::abs(part.extras.dz)<dzThr
      &&
      part.extras.convVeto
      );
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (
      part.extras.lostHits<=maxMissingHits_TopMVAany_Run2_UL
      && std::abs(part.extras.dxy)<dxyThr_TopMVAany_Run2_UL
      && std::abs(part.extras.dz)<dzThr_TopMVAany_Run2_UL
      && std::abs(part.extras.sip3d)<sip3dThr_TopMVAany_Run2_UL
      );
  default:
    IVYerr << "ElectronSelectionHelpers::testLooseId_Common: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testLooseId_IsoTrig(ElectronObject const& part){
  if (!testLooseId_Common(part)) return false;

  double const part_pt = part.pt();
  double const part_etaSC = std::abs(part.etaSC());

  switch (selection_type){
  case kCutbased_Run2:
  {
    // if pt is too small or eta is too large for tracking
    if (part_pt < ptThr_cat0 || part_etaSC >= etaThr_cat2) return false;

    // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
    unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
    unsigned char etacat = 1*(part_etaSC >= etaThr_cat0) + 1*(part_etaSC >= etaThr_cat1);

    // TODO: Potential bug. WP does not match for pt == 10 on lower and upper bounds. 
    double wpcutCoeffA = 99;
    double wpcutCoeffB = 0;
    switch (ptcat){
    case 0:
      wpcutCoeffA = (etacat == 0 ? 1.320 : (etacat == 1 ? 0.192 : 0.362));
      break;
    case 1:
      wpcutCoeffB = (etacat == 0 ? 0.066 : (etacat == 1 ? 0.033 : 0.053));
    case 2:
      wpcutCoeffA = (etacat == 0 ? 1.204 : (etacat == 1 ? 0.084 : -0.123));
      break;
    default:
      return false;
    }
    double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

    float const raw_mva = convert_BDTscore_raw(getNanoMVANoIsoBDTScore(part));
    return raw_mva >= wpcut;
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return true; // Already handled in the common function
  default:
    IVYerr << "ElectronSelectionHelpers::testLooseId_IsoTrig: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testLooseId_NoIsoTrig(ElectronObject const& part){
  if (!testLooseId_Common(part)) return false;

  double const part_pt = part.pt();
  double const part_etaSC = std::abs(part.etaSC());

  switch (selection_type){
  case kCutbased_Run2:
  {
    // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
    // if pt is too small or eta is too large for tracking
    if (part_pt < ptThr_cat0 || part_etaSC >= etaThr_cat2) return false;

    unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
    unsigned char etacat = 1*(part_etaSC >= etaThr_cat0) + 1*(part_etaSC >= etaThr_cat1);

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
    case 2:
      wpcutCoeffA = (etacat == 0 ? -0.106 : (etacat == 1 ? -0.769 : -1.461));
      break;
    default:
      return false;
    }
    double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

    float const raw_mva = convert_BDTscore_raw(getNanoMVANoIsoBDTScore(part));
    return raw_mva >= wpcut;
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return true; // Already handled in the common function
  default:
    IVYerr << "ElectronSelectionHelpers::testLooseId_NoIsoTrig: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testFakeableId_Common(ElectronObject const& part){
  double const part_pt = part.pt();
  double const part_etaSC = std::abs(part.etaSC());

  switch (selection_type){
  case kCutbased_Run2:
  {
    return (
      part.extras.lostHits<=maxMissingHits_fakeable_tight
      &&
      std::abs(part.extras.dxy)<dxyThr
      &&
      std::abs(part.extras.dz)<dzThr
      &&
      std::abs(part.extras.sip3d)<sip3dThr
      &&
      part.extras.convVeto
      &&
      part.extras.tightCharge==2
      );
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
  {
    float mvascore = -999;
    if (!part.getExternalMVAScore(selection_type, mvascore)){
      IVYerr << "ElectronSelectionHelpers::testFakeableId: MVA score is not yet computed for selection type " << selection_type << "." << endl;
      assert(0);
    }

    float const ptratio = part.ptratio();

    float bscore = 0;
    AK4JetObject* mother = nullptr;
    for (auto const& mom:part.getMothers()){
      mother = dynamic_cast<AK4JetObject*>(mom);
      if (mother) break;
    }
    if (mother) bscore = mother->extras.btagDeepFlavB;

    float const isoThr_TopMVAany_Run2_UL_Fakeable_ALT_I2 = (SampleHelpers::getDataYear()<=2016 ? isoThr_TopMVAany_Run2_UL_Fakeable_ALT_I2_Phase0 : isoThr_TopMVAany_Run2_UL_Fakeable_ALT_I2_Phase1);

    return (
      part.extras.lostHits<=maxMissingHits_TopMVAany_Run2_UL
      && std::abs(part.extras.dxy)<dxyThr_TopMVAany_Run2_UL
      && std::abs(part.extras.dz)<dzThr_TopMVAany_Run2_UL
      && std::abs(part.extras.sip3d)<sip3dThr_TopMVAany_Run2_UL
      //&& part.extras.hoe<hoverEThr_TopMVAany_Run2_UL
      //&& part.extras.eInvMinusPInv>min_EinvminusPinvThr_TopMVAany_Run2_UL
      //&& part.extras.sieie<(part_etaSC<etaThr_cat1 ? sieieThr_barrel_TopMVAany_Run2_UL : sieieThr_endcap_TopMVAany_Run2_UL)
      && part.extras.convVeto
      && part.extras.tightCharge==2
      && (
        mvascore>(selection_type==kTopMVA_Run2 ? wp_tight_TopMVA_Run2_UL : wp_tight_TopMVAv2_Run2_UL)
        ||
        (part.extras.mvaFall17V2noIso_WPL && ptratio>=isoThr_TopMVAany_Run2_UL_Fakeable_ALT_I2 && bscore<bscoreThr_TopMVAany_Run2_UL_Fakeable_ALT)
        )
      );
  }
  default:
    IVYerr << "ElectronSelectionHelpers::testFakeableId_Common: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testFakeableId_IsoTrig(ElectronObject const& part){
  if (!testFakeableId_Common(part)) return false;

  double const part_pt = part.pt();
  double const part_etaSC = std::abs(part.etaSC());

  switch (selection_type){
  case kCutbased_Run2:
  {
    // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
    // if pt is too small or eta is too large for tracking
    if (part_pt < ptThr_cat1 || part_etaSC >= etaThr_cat2) return false;

    unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
    unsigned char etacat = 1*(part_etaSC >= etaThr_cat0) + 1*(part_etaSC >= etaThr_cat1);

    // See AN2018_062_v17 lines 298-303 for description of mva discriminant
    // Note that for electrons, pt<etaThr_cat0 = 10 GeV always returns false.
    double wpcutCoeffA = 99;
    double wpcutCoeffB = 0;
    switch (ptcat){
    case 1:
      wpcutCoeffB = (etacat == 0 ? 0.066 : (etacat == 1 ? 0.033 : 0.053));
    case 2:
      wpcutCoeffA = (etacat == 0 ? 1.204 : (etacat == 1 ? 0.084 : -0.123));
      break;
    default:
      return false;
    }
    double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

    float const raw_mva = convert_BDTscore_raw(getNanoMVANoIsoBDTScore(part));
    return raw_mva >= wpcut;
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return true; // Already handled in the common function
  default:
    IVYerr << "ElectronSelectionHelpers::testFakeableId_IsoTrig: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testFakeableId_NoIsoTrig(ElectronObject const& part){
  if (!testFakeableId_Common(part)) return false;

  double const part_pt = part.pt();
  double const part_etaSC = std::abs(part.etaSC());

  switch (selection_type){
  case kCutbased_Run2:
  {
    // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
    // if pt is too small or eta is too large for tracking
    if (part_pt < ptThr_cat1 || part_etaSC >= etaThr_cat2) return false;

    unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
    unsigned char etacat = 1*(part_etaSC >= etaThr_cat0) + 1*(part_etaSC >= etaThr_cat1);

    // See AN2018_062_v17 lines 298-303 for description of mva discriminant
    // Note that for electrons, pt<etaThr_cat0 = 10 GeV always returns false.
    double wpcutCoeffA = 99;
    double wpcutCoeffB = 0;
    switch (ptcat){
    case 1:
      wpcutCoeffB = (etacat == 0 ? 0.062 : (etacat == 1 ? 0.038 : 0.042));
    case 2:
      wpcutCoeffA = (etacat == 0 ? -0.106 : (etacat == 1 ? -0.769 : -1.461));
      break;
    default:
      return false;
    }
    double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

    float const raw_mva = convert_BDTscore_raw(getNanoMVANoIsoBDTScore(part));
    return raw_mva >= wpcut;
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return true; // Already handled in the common function
  default:
    IVYerr << "ElectronSelectionHelpers::testFakeableId_IsoTrig: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testTightId(ElectronObject const& part){
  double const part_pt = part.pt();
  double const part_eta = std::abs(part.eta());
  double const part_etaSC = std::abs(part.etaSC());

  switch (selection_type){
  case kCutbased_Run2:
  {
    // have pt categories 0, 1, 2. unsigned char is 8 bit int. 
    // if pt is too small or eta is too large for tracking
    if (part_pt < ptThr_cat1 || part_etaSC >= etaThr_cat2) return false;
    if (
      !(
        part.extras.lostHits<=maxMissingHits_fakeable_tight
        &&
        std::abs(part.extras.dxy)<dxyThr
        &&
        std::abs(part.extras.dz)<dzThr
        &&
        std::abs(part.extras.sip3d)<sip3dThr
        &&
        part.extras.convVeto
        &&
        part.extras.tightCharge==2
        )
      ) return false;

    unsigned char ptcat = 1*(part_pt >= ptThr_cat1) + 1*(part_pt >= ptThr_cat2);
    unsigned char etacat = 1*(part_etaSC >= etaThr_cat0) + 1*(part_etaSC >= etaThr_cat1);

    // See AN2018_062_v17 lines 298-303 for description of mva discriminant
    // Note that for electrons, pt<etaThr_cat0 = 10 GeV always returns false.
    double wpcutCoeffA = 99;
    double wpcutCoeffB = 0;
    switch (ptcat){
    case 1:
      wpcutCoeffB = (etacat == 0 ? 0.112 : (etacat == 1 ? 0.060 : 0.087));
    case 2:
      wpcutCoeffA = (etacat == 0 ? 4.277 : (etacat == 1 ? 3.152 : 2.359));
      break;
    default:
      return false;
    }
    double wpcut = wpcutCoeffA + wpcutCoeffB * (part_pt - ptThr_cat2);

    float const raw_mva = convert_BDTscore_raw(getNanoMVANoIsoBDTScore(part));
    return raw_mva >= wpcut;
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
  {
    float mvascore = -999;
    if (!part.getExternalMVAScore(selection_type, mvascore)){
      IVYerr << "ElectronSelectionHelpers::testTightId: MVA score is not yet computed for selection type " << selection_type << "." << endl;
      assert(0);
    }

    return (
      part.extras.lostHits<=maxMissingHits_TopMVAany_Run2_UL
      && std::abs(part.extras.dxy)<dxyThr_TopMVAany_Run2_UL
      && std::abs(part.extras.dz)<dzThr_TopMVAany_Run2_UL
      && std::abs(part.extras.sip3d)<sip3dThr_TopMVAany_Run2_UL
      //&& part.extras.hoe<hoverEThr_TopMVAany_Run2_UL
      //&& part.extras.eInvMinusPInv>min_EinvminusPinvThr_TopMVAany_Run2_UL
      //&& part.extras.sieie<(part_etaSC<etaThr_cat1 ? sieieThr_barrel_TopMVAany_Run2_UL : sieieThr_endcap_TopMVAany_Run2_UL)
      && part.extras.convVeto
      && part.extras.tightCharge==2
      && mvascore>(selection_type==kTopMVA_Run2 ? wp_tight_TopMVA_Run2_UL : wp_tight_TopMVAv2_Run2_UL)
      );
  }
  default:
    IVYerr << "ElectronSelectionHelpers::testTightId: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}


bool ElectronSelectionHelpers::testLooseIso(ElectronObject const& part){
  switch (selection_type){
  case kCutbased_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_TopMVAany_Run2_UL_I1);
  default:
    IVYerr << "ElectronSelectionHelpers::testLooseIso: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testFakeableIso(ElectronObject const& part){
  double const ptratio = part.ptratio();
  double const ptrel = part.ptrel();

  switch (selection_type){
  case kCutbased_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_TopMVAany_Run2_UL_I1);
  default:
    IVYerr << "ElectronSelectionHelpers::testFakeableIso: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testTightIso(ElectronObject const& part){
  double const ptratio = part.ptratio();
  double const ptrel = part.ptrel();

  switch (selection_type){
  case kCutbased_Run2:
  {
    auto const& isoThr_tight_I1 = (SampleHelpers::getDataYear()<=2016 ? isoThr_tight_I1_Phase0Tracker : isoThr_tight_I1_Phase1Tracker);
    auto const& isoThr_tight_I2 = (SampleHelpers::getDataYear()<=2016 ? isoThr_tight_I2_Phase0Tracker : isoThr_tight_I2_Phase1Tracker);
    auto const& isoThr_tight_I3 = (SampleHelpers::getDataYear()<=2016 ? isoThr_tight_I3_Phase0Tracker : isoThr_tight_I3_Phase1Tracker);

    return (part.extras.miniPFRelIso_all < isoThr_tight_I1 && (!apply_multiiso || ptratio>=isoThr_tight_I2 || ptrel>=isoThr_tight_I3));
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_TopMVAany_Run2_UL_I1);
  default:
    IVYerr << "ElectronSelectionHelpers::testTightIso: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}

bool ElectronSelectionHelpers::testKin_Skim(ElectronObject const& part){
  return (part.pt() >= ptThr_skim && std::abs(part.eta()) < etaThr_cat2);
}
bool ElectronSelectionHelpers::testKin_Loose(ElectronObject const& part){
  double const part_etaSC = std::abs(part.etaSC());
  bool const pass_eta = (std::abs(part.eta()) < etaThr_cat2 && !(apply_EBEEGapVeto && part_etaSC>=etaThrLow_Gap && part_etaSC<=etaThrHigh_Gap));
  switch (selection_type){
  case kCutbased_Run2:
    return (part.pt() >= ptThr_loose && pass_eta);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.pt() >= ptThr_TopMVAany_Run2_UL_loose && pass_eta);
  default:
    IVYerr << "ElectronSelectionHelpers::testKin_Loose: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testKin_Fakeable(ElectronObject const& part){
  double const part_etaSC = std::abs(part.etaSC());
  bool const pass_eta = (std::abs(part.eta()) < etaThr_cat2 && !(apply_EBEEGapVeto && part_etaSC>=etaThrLow_Gap && part_etaSC<=etaThrHigh_Gap));
  switch (selection_type){
  case kCutbased_Run2:
    return (part.pt() >= ptThr_fakeable && pass_eta);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.pt() >= ptThr_TopMVAany_Run2_UL_fakeable && pass_eta);
  default:
    IVYerr << "ElectronSelectionHelpers::testKin_Fakeable: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testKin_Tight(ElectronObject const& part){
  double const part_etaSC = std::abs(part.etaSC());
  bool const pass_eta = (std::abs(part.eta()) < etaThr_cat2 && !(apply_EBEEGapVeto && part_etaSC>=etaThrLow_Gap && part_etaSC<=etaThrHigh_Gap));
  switch (selection_type){
  case kCutbased_Run2:
    return (part.pt() >= ptThr_tight && pass_eta);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.pt() >= ptThr_TopMVAany_Run2_UL_tight && pass_eta);
  default:
    IVYerr << "ElectronSelectionHelpers::testKin_Tight: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool ElectronSelectionHelpers::testKin(ElectronObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Skim)
    ||
    part.testSelectionBit(kKinOnly_Loose)
    ||
    part.testSelectionBit(kKinOnly_Fakeable)
    ||
    part.testSelectionBit(kKinOnly_Tight)
    );
}

bool ElectronSelectionHelpers::testPreselectionLoose_IsoTrig(ElectronObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Loose)
    &&
    testTriggerSafety(part)
    &&
    testLooseId_IsoTrig(part)
    &&
    testLooseIso(part)
    );
}
bool ElectronSelectionHelpers::testPreselectionLoose_NoIsoTrig(ElectronObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Loose)
    &&
    testTriggerSafety(part)
    &&
    testLooseId_NoIsoTrig(part)
    &&
    testLooseIso(part)
    );
}
bool ElectronSelectionHelpers::testPreselectionLoose(ElectronObject const& part){
  return (
    (applyMVALooseFakeableNoIsoWPs && part.testSelectionBit(kPreselectionLoose_NoIsoTrig))
    ||
    (!applyMVALooseFakeableNoIsoWPs && part.testSelectionBit(kPreselectionLoose_IsoTrig))
    );
}
bool ElectronSelectionHelpers::testPreselectionFakeable_IsoTrig(ElectronObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Fakeable)
    &&
    testTriggerSafety(part)
    &&
    testFakeableId_IsoTrig(part)
    &&
    testFakeableIso(part)
    );
}
bool ElectronSelectionHelpers::testPreselectionFakeable_NoIsoTrig(ElectronObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Fakeable)
    &&
    testTriggerSafety(part)
    &&
    testFakeableId_NoIsoTrig(part)
    &&
    testFakeableIso(part)
    );
}
bool ElectronSelectionHelpers::testPreselectionFakeable(ElectronObject const& part){
  return (
    (applyMVALooseFakeableNoIsoWPs && part.testSelectionBit(kPreselectionFakeable_NoIsoTrig))
    ||
    (!applyMVALooseFakeableNoIsoWPs && part.testSelectionBit(kPreselectionFakeable_IsoTrig))
    );
}
bool ElectronSelectionHelpers::testPreselectionTight(ElectronObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Tight)
    &&
    testTriggerSafety(part)
    &&
    testTightId(part)
    &&
    testTightIso(part)
    );
}

void ElectronSelectionHelpers::setSelectionBits(ElectronObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kKinOnly_Skim, testKin_Skim(part));
  part.setSelectionBit(kKinOnly_Loose, testKin_Loose(part));
  part.setSelectionBit(kKinOnly_Fakeable, testKin_Fakeable(part));
  part.setSelectionBit(kKinOnly_Tight, testKin_Tight(part));
  part.setSelectionBit(kKinOnly, testKin(part));

  // Store MVA scores if particle passes kinematic selections
  storeMVAScores(part);

  part.setSelectionBit(kPreselectionLoose_NoIsoTrig, testPreselectionLoose_NoIsoTrig(part));
  part.setSelectionBit(kPreselectionLoose_IsoTrig, testPreselectionLoose_IsoTrig(part));
  part.setSelectionBit(kPreselectionLoose, testPreselectionLoose(part));
  part.setSelectionBit(kPreselectionFakeable_NoIsoTrig, testPreselectionFakeable_NoIsoTrig(part));
  part.setSelectionBit(kPreselectionFakeable_IsoTrig, testPreselectionFakeable_IsoTrig(part));
  part.setSelectionBit(kPreselectionFakeable, testPreselectionFakeable(part));
  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
}
