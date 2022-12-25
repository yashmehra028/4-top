#include <cassert>
#include <cmath>
#include <limits>

#include "SamplesCore.h"
#include "MuonSelectionHelpers.h"
#include "AK4JetObject.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


namespace MuonSelectionHelpers{
  constexpr bool apply_multiiso = true;

  SelectionType selection_type = kCutbased_Run2;

  // Just test if this selection is cut- or MVA-based
  bool isMVASelection(SelectionType const& type);

  // MVA reader for MVA IDs
  std::unordered_map<SelectionType, std::shared_ptr<IvyXGBoostInterface> > seltype_mvareader_map;
  void loadMVA();

  // Sliding b-tagging WP
  float get_bscoreThr_TopMVAAny_Run2_UL_Fakeable_ALT(MuonObject const& part);

  // Common functions
  bool testLooseId(MuonObject const& part);
  bool testLooseIso(MuonObject const& part);

  bool testFakeableId(MuonObject const& part);
  bool testFakableIso(MuonObject const& part);

  bool testTightId(MuonObject const& part);
  bool testTightIso(MuonObject const& part);

  bool testKin_Skim(MuonObject const& part);
  bool testKin_Loose(MuonObject const& part);
  bool testKin_Fakeable(MuonObject const& part);
  bool testKin_Tight(MuonObject const& part);
  bool testKin(MuonObject const& part);

  bool testPreselectionLoose(MuonObject const& part);
  bool testPreselectionFakeable(MuonObject const& part);
  bool testPreselectionTight(MuonObject const& part);
}


using namespace std;
using namespace IvyStreamHelpers;


bool MuonSelectionHelpers::isMVASelection(SelectionType const& type){
  switch (type){
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return true;
  default:
    return false;
  }
}

void MuonSelectionHelpers::setSelectionTypeByName(TString stname){
  SelectionType type = kCutbased_Run2;
  if (stname=="Cutbased_Run2") type = kCutbased_Run2;
  else if (stname=="TopMVA_Run2") type = kTopMVA_Run2;
  else if (stname=="TopMVAv2_Run2") type = kTopMVAv2_Run2;
  else{
    IVYerr << "MuonSelectionHelpers::setSelectionTypeByName: Type name " << stname << " is not recognized." << endl;
    assert(0);
  }
  setSelectionType(type);
}
void MuonSelectionHelpers::setSelectionType(SelectionType const& type){
  if (!SampleHelpers::runConfigure && isMVASelection(type)){
    IVYerr << "MuonSelectionHelpers::setSelectionType: SampleHelpers::configure needs to be called first when setting ML-based selection." << endl;
    assert(0);
  }
  selection_type = type;
  loadMVA();
}
void MuonSelectionHelpers::loadMVA(){
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
    fname = "mu_TOP";
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
      IVYout << "MuonSelectionHelpers::loadMVA: WARNING! Using the MVA for year 2018 in place of " << dy << "." << endl;
    }
    else{
      IVYerr << "MuonSelectionHelpers::loadMVA: Year " << dy << " is not implemented for type " << selection_type << "." << endl;
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
      "log_abs_dz", // = log(|dz|)
      "segmentComp"
    };
    // No need to set missing_entry_val
  }
  else{
    IVYerr << "MuonSelectionHelpers::loadMVA: Selection " << selection_type << " is not implemented." << endl;
    assert(0);
  }

  mvareader_xgb = std::make_shared<IvyXGBoostInterface>();
  mvareader_xgb->build(fname, varnames, missing_entry_val);
}
void MuonSelectionHelpers::storeMVAScores(MuonObject& part){
  for (auto const& pp:seltype_mvareader_map){ if (isMVASelection(pp.first)) part.setExternalMVAScore(static_cast<int>(pp.first), computeMVAScore(part, pp.first)); }
}
float MuonSelectionHelpers::computeMVAScore(MuonObject const& part, SelectionType const& type){
  float res = -999;
  if (!isMVASelection(type) || !part.testSelectionBit(kKinOnly_Loose) || part.getExternalMVAScore(static_cast<int>(type), res)) return res;

  auto it_seltype_mvareader_map = seltype_mvareader_map.find(type);
  if (it_seltype_mvareader_map==seltype_mvareader_map.end()){
    IVYerr << "MuonSelectionHelpers::computeMVAScore: External MVA for selection type " << type << " is not set up." << endl;
    assert(0);
  }
  auto& mvareader_xgb = seltype_mvareader_map.find(type)->second;
  if (mvareader_xgb){
    std::unordered_map<TString, IvyMLWrapper::IvyMLDataType_t> input_vars;

    AK4JetObject* mother = nullptr;
    for (auto const& mom:part.getMothers()){
      mother = dynamic_cast<AK4JetObject*>(mom);
      if (mother) break;
    }

    auto const& vnames = mvareader_xgb->getVariableNames();
    for (auto const& vname:vnames){
      if (vname=="pt") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.pt());
      else if (vname=="eta") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.eta());
      else if (vname=="miniPFRelIso_diff_all_chg") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.extras.miniPFRelIso_all - part.extras.miniPFRelIso_chg);
      else if (vname=="jetPtRatio") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(1./(part.extras.jetRelIso+1.));
      else if (vname=="log_abs_dxy") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(std::log(std::abs(part.extras.dxy)));
      else if (vname=="log_abs_dz") input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(std::log(std::abs(part.extras.dz)));
      else if (vname=="ak4jet:btagDeepFlavB") input_vars[vname] = (mother ? static_cast<IvyMLWrapper::IvyMLDataType_t>(mother->extras.btagDeepFlavB) : IvyMLWrapper::IvyMLDataType_t(0));
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) else if (vname==#NAME) input_vars[vname] = static_cast<IvyMLWrapper::IvyMLDataType_t>(part.extras.NAME);
      MUON_EXTRA_VARIABLES
#undef MUON_VARIABLE
      else{
        IVYerr << "MuonSelectionHelpers::computeMVAScore: Input variable name " << vname << " does not match to a corresponding variable. Please fix the implementation." << endl;
        assert(0);
      }
    }

    mvareader_xgb->eval(input_vars, res);
  }
  return res;
}


float MuonSelectionHelpers::getIsolationDRmax(MuonObject const& part){
  return (10. / std::min(std::max(part.uncorrected_pt(), 50.), 200.));
}

float MuonSelectionHelpers::get_bscoreThr_TopMVAAny_Run2_UL_Fakeable_ALT(MuonObject const& part){
  constexpr double cA = 0.02;
  constexpr double cB = 0.015;
  constexpr double ptA = 20.;
  constexpr double ptB = 40.;
  double const part_pt = part.pt();

  if (part_pt<=ptA) return cA;
  else if (part_pt>=ptB) return cB;
  else return (cA + (cB - cA)*(part_pt - ptA)/(ptB - ptA));
}

bool MuonSelectionHelpers::testLooseId(MuonObject const& part){
  switch (selection_type){
  case kCutbased_Run2:
    return (
      part.extras.looseId
      &&
      std::abs(part.extras.dxy)<dxyThr
      &&
      std::abs(part.extras.dz)<dzThr
      );
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (
      part.extras.mediumId
      &&
      std::abs(part.extras.dxy)<dxyThr_TopMVAany_Run2_UL
      &&
      std::abs(part.extras.dz)<dzThr_TopMVAany_Run2_UL
      &&
      std::abs(part.extras.sip3d)<sip3dThr_TopMVAany_Run2_UL
      );
  default:
    IVYerr << "MuonSelectionHelpers::testLooseId: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool MuonSelectionHelpers::testFakeableId(MuonObject const& part){
  double const part_pt = part.pt();

  switch (selection_type){
  case kCutbased_Run2:
    return (
      part.extras.mediumId
      &&
      std::abs(part.extras.dxy)<dxyThr
      &&
      std::abs(part.extras.dz)<dzThr
      &&
      std::abs(part.extras.sip3d)<sip3dThr
      &&
      part.extras.ptErr/part_pt<track_reco_qualityThr
      );
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
  {
    float mvascore = -999;
    if (!part.getExternalMVAScore(selection_type, mvascore)){
      IVYerr << "MuonSelectionHelpers::testFakeableId: MVA score is not yet computed for selection type " << selection_type << "." << endl;
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

    return (
      part.extras.mediumId
      &&
      std::abs(part.extras.dxy)<dxyThr_TopMVAany_Run2_UL
      &&
      std::abs(part.extras.dz)<dzThr_TopMVAany_Run2_UL
      &&
      std::abs(part.extras.sip3d)<sip3dThr_TopMVAany_Run2_UL
      && (
        mvascore>(selection_type==kTopMVA_Run2 ? wp_medium_TopMVA_Run2_UL : wp_medium_TopMVAv2_Run2_UL)
        ||
        (bscore<bscoreThr_TopMVAAny_Run2_UL_Fakeable_ALT_fixed && ptratio>=isoThr_TopMVAany_Run2_UL_Fakeable_ALT_I2)
        )
      );
  }
  default:
    IVYerr << "MuonSelectionHelpers::testFakeableId: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool MuonSelectionHelpers::testTightId(MuonObject const& part){
  double const part_pt = part.pt();

  switch (selection_type){
  case kCutbased_Run2:
    return (
      part.extras.mediumId
      &&
      std::abs(part.extras.dxy)<dxyThr
      &&
      std::abs(part.extras.dz)<dzThr
      &&
      std::abs(part.extras.sip3d)<sip3dThr
      &&
      part.extras.ptErr/part_pt<track_reco_qualityThr
      );
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
  {
    float mvascore = -999;
    if (!part.getExternalMVAScore(selection_type, mvascore)){
      IVYerr << "MuonSelectionHelpers::testFakeableId: MVA score is not yet computed for selection type " << selection_type << "." << endl;
      assert(0);
    }

    return (
      part.extras.mediumId
      &&
      std::abs(part.extras.dxy)<dxyThr_TopMVAany_Run2_UL
      &&
      std::abs(part.extras.dz)<dzThr_TopMVAany_Run2_UL
      &&
      std::abs(part.extras.sip3d)<sip3dThr_TopMVAany_Run2_UL
      &&
      mvascore>(selection_type==kTopMVA_Run2 ? wp_medium_TopMVA_Run2_UL : wp_medium_TopMVAv2_Run2_UL)
      );
  }
  default:
    IVYerr << "MuonSelectionHelpers::testTightId: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}

bool MuonSelectionHelpers::testLooseIso(MuonObject const& part){
  switch (selection_type){
  case kCutbased_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_TopMVAany_Run2_UL_I1);
  default:
    IVYerr << "MuonSelectionHelpers::testLooseIso: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool MuonSelectionHelpers::testFakableIso(MuonObject const& part){
  double const ptratio = part.ptratio();
  double const ptrel = part.ptrel();

  switch (selection_type){
  case kCutbased_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_loose_I1);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_TopMVAany_Run2_UL_I1);
  default:
    IVYerr << "MuonSelectionHelpers::testFakableIso: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool MuonSelectionHelpers::testTightIso(MuonObject const& part){
  double const ptratio = part.ptratio();
  double const ptrel = part.ptrel();

  switch (selection_type){
  case kCutbased_Run2:
  {
    auto const& isoThr_medium_I1 = (SampleHelpers::getDataYear()<=2016 ? isoThr_medium_I1_Phase0Tracker : isoThr_medium_I1_Phase1Tracker);
    auto const& isoThr_medium_I2 = (SampleHelpers::getDataYear()<=2016 ? isoThr_medium_I2_Phase0Tracker : isoThr_medium_I2_Phase1Tracker);
    auto const& isoThr_medium_I3 = (SampleHelpers::getDataYear()<=2016 ? isoThr_medium_I3_Phase0Tracker : isoThr_medium_I3_Phase1Tracker);

    return (
      part.extras.miniPFRelIso_all < isoThr_medium_I1
      &&
      (!apply_multiiso || ptratio>=isoThr_medium_I2 || ptrel>=isoThr_medium_I3)
      );
  }
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.extras.miniPFRelIso_all < isoThr_TopMVAany_Run2_UL_I1);
  default:
    IVYerr << "MuonSelectionHelpers::testTightIso: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}

bool MuonSelectionHelpers::testKin_Skim(MuonObject const& part){
  return (part.pt() >= ptThr_skim && std::abs(part.eta()) < etaThr_cat2);
}
bool MuonSelectionHelpers::testKin_Loose(MuonObject const& part){
  bool const pass_eta = (std::abs(part.eta()) < etaThr_cat2);
  switch (selection_type){
  case kCutbased_Run2:
    return (part.pt() >= ptThr_loose && pass_eta);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.pt() >= ptThr_TopMVAany_Run2_UL_loose && pass_eta);
  default:
    IVYerr << "MuonSelectionHelpers::testKin_Loose: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool MuonSelectionHelpers::testKin_Fakeable(MuonObject const& part){
  bool const pass_eta = (std::abs(part.eta()) < etaThr_cat2);
  switch (selection_type){
  case kCutbased_Run2:
    return (part.pt() >= ptThr_fakeable && pass_eta);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.pt() >= ptThr_TopMVAany_Run2_UL_fakeable && pass_eta);
  default:
    IVYerr << "MuonSelectionHelpers::testKin_Fakeable: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool MuonSelectionHelpers::testKin_Tight(MuonObject const& part){
  bool const pass_eta = (std::abs(part.eta()) < etaThr_cat2);
  switch (selection_type){
  case kCutbased_Run2:
    return (part.pt() >= ptThr_tight && pass_eta);
  case kTopMVA_Run2:
  case kTopMVAv2_Run2:
    return (part.pt() >= ptThr_TopMVAany_Run2_UL_tight && pass_eta);
  default:
    IVYerr << "MuonSelectionHelpers::testKin_Tight: Selection type " << selection_type << " is not implemented." << endl;
    assert(0);
    return false;
  }
}
bool MuonSelectionHelpers::testKin(MuonObject const& part){
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

bool MuonSelectionHelpers::testPreselectionLoose(MuonObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Loose)
    &&
    testLooseId(part)
    &&
    testLooseIso(part)
    );
}
bool MuonSelectionHelpers::testPreselectionFakeable(MuonObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Fakeable)
    &&
    testFakeableId(part)
    &&
    testFakableIso(part)
    );
}
bool MuonSelectionHelpers::testPreselectionTight(MuonObject const& part){
  return (
    part.testSelectionBit(kKinOnly_Tight)
    &&
    testTightId(part)
    &&
    testTightIso(part)
    );
}

void MuonSelectionHelpers::setSelectionBits(MuonObject& part){
  static_assert(std::numeric_limits<ParticleObject::SelectionBitsType_t>::digits >= nSelectionBits);

  part.setSelectionBit(kKinOnly_Skim, testKin_Skim(part));
  part.setSelectionBit(kKinOnly_Loose, testKin_Loose(part));
  part.setSelectionBit(kKinOnly_Fakeable, testKin_Fakeable(part));
  part.setSelectionBit(kKinOnly_Tight, testKin_Tight(part));
  part.setSelectionBit(kKinOnly, testKin(part));

  // Store MVA scores if particle passes kinematic selections
  storeMVAScores(part);

  part.setSelectionBit(kPreselectionLoose, testPreselectionLoose(part));
  part.setSelectionBit(kPreselectionFakeable, testPreselectionFakeable(part));
  part.setSelectionBit(kPreselectionTight, testPreselectionTight(part));
}
