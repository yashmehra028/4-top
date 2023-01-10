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


JECScaleFactorHandler::JECScaleFactorHandler(JESRHelpers::JetType type_, bool isMC_) :
  ScaleFactorHandlerBase(),
  type(type_),
  isMC(isMC_),
  corrector(nullptr),
  uncertaintyEstimator(nullptr)
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

  std::vector<TString> correctornames = getJESFileNames(type, isMC);
  if (!correctornames.empty()) corrector = makeCorrector(correctornames);

  TString uncname = getJESUncertaintyFileName(type, isMC);
  if (uncname!="") uncertaintyEstimator = makeUncertaintyEstimator(uncname);

  curdir->cd();

  return res;
}
void JECScaleFactorHandler::reset(){
  delete corrector; corrector = nullptr;
  delete uncertaintyEstimator; uncertaintyEstimator = nullptr;
}

void JECScaleFactorHandler::applyJEC(ParticleObject* obj, float const& rho){
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

  if (!corrector) return;
  if (isMC && !uncertaintyEstimator) return;

  //static unsigned long long njets = 0;
  //static unsigned long long njets_warning = 0;

  auto const& jet_p4_uncor = ak4jet->uncorrected_p4();
  double jpt_unc = jet_p4_uncor.Pt();
  double jeta_unc = jet_p4_uncor.Eta();
  double jphi_unc = jet_p4_uncor.Phi();

  // Get nominal JEC references
  auto& JEC = ak4jet->extras.JECNominal;
  auto& JEC_L1 = ak4jet->extras.JECL1Nominal;
  if (corrector){
    //njets++;

    corrector->setRho(rho);
    corrector->setJetA(ak4jet->extras.area);
    corrector->setJetPt(jpt_unc);
    corrector->setJetEta(jeta_unc);
    std::vector<float> const corr_vals = corrector->getSubCorrections(); // Subcorrections are stored with corr_vals(N) = corr(N)*corr(N-1)*...*corr(1)
    auto const& JEC_L1L2L3_comp = corr_vals.back();
    JEC_L1 = corr_vals.front();
    if (!printFirstWarning && !ak4jet->extras.isLowPtJet && std::abs(JEC_L1L2L3_comp - JEC)>JEC*0.01){
      printFirstWarning = true;
      //njets_warning++;

      IVYout
        << "JECScaleFactorHandler::applyJEC: WARNING"/* << " (" << njets_warning << "/" << njets << ")"*/ << "! Nominal JEC value " << JEC << " (recorded) != " << JEC_L1L2L3_comp << " (computed). The properties of the jet are as follows:"
        << "\n\t- Uncorrected pT = " << jpt_unc
        << "\n\t- eta = " << jeta_unc
        << "\n\t- phi = " << jphi_unc
        << "\n\t- rho = " << rho
        << "\n\t- area = " << ak4jet->extras.area
        << "\n\t- rawFactor = " << ak4jet->extras.rawFactor
        << endl;
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
  if (uncertaintyEstimator){
    uncertaintyEstimator->setJetPt(jpt_unc*JEC); // Must use corrected pT
    uncertaintyEstimator->setJetEta(jeta_unc);
    uncertaintyEstimator->setJetPhi(jphi_unc);
    JECunc = uncertaintyEstimator->getUncertainty(true);
  }
}
