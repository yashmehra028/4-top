#include <cassert>
#include <cstdio>
#include "JECScaleFactorHandler.h"
#include "JetCorrectorParameters.h"
#include "AK4JetObject.h"
//#include "AK8JetObject.h"
#include "IvyFramework/IvyDataTools/interface/HostHelpersCore.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace JESRHelpers;
using namespace IvyStreamHelpers;


JECScaleFactorHandler::JECScaleFactorHandler(JESRHelpers::JetType type_) :
  ScaleFactorHandlerBase(),
  type(type_),
  corrector_data(nullptr),
  corrector_MC(nullptr),
  uncertaintyEstimator_MC(nullptr)
{
  setup();
}

JECScaleFactorHandler::~JECScaleFactorHandler(){ this->reset(); }

FactorizedJetCorrector* JECScaleFactorHandler::makeCorrector(std::vector<TString> const& fnames){
  if (fnames.empty()) return nullptr;

  for (TString const& fname:fnames){
    if (!HostHelpers::FileExists(fname)){
      IVYerr << "JECScaleFactorHandler::makeCorrector: File " << fname << " does not exists! Aborting..." << endl;
      assert(0);
    }
  }

  std::vector<JetCorrectorParameters> vParam; vParam.reserve(fnames.size());
  for (TString const& fname:fnames) vParam.emplace_back(fname.Data());

  return new FactorizedJetCorrector(vParam);
}
JetCorrectionUncertainty* JECScaleFactorHandler::makeUncertaintyEstimator(TString const& fname){ return new JetCorrectionUncertainty(fname.Data()); }


bool JECScaleFactorHandler::setup(){
  bool res = true;
  this->reset();

  TDirectory* curdir = gDirectory;

  std::vector<TString> correctornames_data = getJESFileNames(type, false);
  if (!correctornames_data.empty()) corrector_data = makeCorrector(correctornames_data);

  std::vector<TString> correctornames_MC = getJESFileNames(type, true);
  if (!correctornames_MC.empty()) corrector_MC = makeCorrector(correctornames_MC);

  TString uncname_MC = getJESUncertaintyFileName(type, true);
  if (uncname_MC!="") uncertaintyEstimator_MC = makeUncertaintyEstimator(uncname_MC);

  curdir->cd();

  return res;
}
void JECScaleFactorHandler::reset(){
  delete corrector_data; corrector_data = nullptr;
  delete corrector_MC; corrector_MC = nullptr;
  delete uncertaintyEstimator_MC; uncertaintyEstimator_MC = nullptr;
}

void JECScaleFactorHandler::applyJEC(ParticleObject* obj, float const& rho, bool isMC){
  static bool printFirstWarning = false;

  AK4JetObject* ak4jet = dynamic_cast<AK4JetObject*>(obj);
  if (
    !(
      (type==kAK4PFCHS || type==kAK4PFPuppi) && ak4jet
      )
    ){
    IVYerr << "JECScaleFactorHandler::applyJEC: Mismatch between object class and correction type " << type << ". Aborting..." << endl;
    assert(0);
  }

  FactorizedJetCorrector* corrector = nullptr;
  if (!isMC) corrector = corrector_data;
  else corrector = corrector_MC;

  if (!corrector) return;

  JetCorrectionUncertainty* uncEst = nullptr;
  if (isMC) uncEst = uncertaintyEstimator_MC;

  auto const& jet_p4_uncor = ak4jet->uncorrected_p4();
  double jpt_unc = jet_p4_uncor.Pt();
  double jeta_unc = jet_p4_uncor.Eta();
  double jphi_unc = jet_p4_uncor.Phi();

  // Get nominal JEC references
  auto& JEC = ak4jet->extras.JECNominal;
  auto& JEC_L1 = ak4jet->extras.JECL1Nominal;
  if (corrector){
    corrector->setRho(rho);
    corrector->setJetA(ak4jet->extras.area);
    corrector->setJetPt(jpt_unc);
    corrector->setJetEta(jeta_unc);
    std::vector<float> const corr_vals = corrector->getSubCorrections(); // Subcorrections are stored with corr_vals(N) = corr(N)*corr(N-1)*...*corr(1)
    auto const& JEC_L1L2L3_comp = corr_vals.back();
    JEC_L1 = corr_vals.front();
    if (!printFirstWarning && std::abs(JEC_L1L2L3_comp - JEC)>JEC*0.01){
      printFirstWarning = true;
      IVYout << "JECScaleFactorHandler::applyJEC: WARNING! Nominal JEC value " << JEC << " != " << JEC_L1L2L3_comp << "." << endl;
    }
    JEC = JEC_L1L2L3_comp;
  }

  if (JEC<0.f){
    IVYerr << "JECScaleFactorHandler::applyJEC: Nominal JEC value " << JEC << " < 0." << endl;
    JEC=0.;
  }
  if (JEC_L1<0.f){
    IVYerr << "JECScaleFactorHandler::applyJEC: Nominal JEC_L1 value " << JEC_L1 << " < 0." << endl;
    JEC_L1=0.;
  }

  // Get up/dn variations
  auto& JECunc = ak4jet->extras.relJECUnc;
  if (uncEst){
    uncEst->setJetPt(jpt_unc*JEC); // Must use corrected pT
    uncEst->setJetEta(jeta_unc);
    uncEst->setJetPhi(jphi_unc);
    JECunc = uncEst->getUncertainty(true);
  }
}
