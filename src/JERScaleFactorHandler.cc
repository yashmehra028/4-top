#include <cassert>
#include "TRandom3.h"
#include "GenJetObject.h"
#include "JERScaleFactorHandler.h"
#include "IvyFramework/IvyDataTools/interface/HostHelpersCore.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace JESRHelpers;
using namespace IvyStreamHelpers;


JERScaleFactorHandler::JERScaleFactorHandler(JESRHelpers::JetType type_) :
  ScaleFactorHandlerBase(),
  type(type_)
{
  setup();
}
JERScaleFactorHandler::~JERScaleFactorHandler(){ this->reset(); }

bool JERScaleFactorHandler::setup(){
  bool res = true;
  this->reset();

  TDirectory* curdir = gDirectory;

  resolution_pt = JetResolution(getJERPtFileName(type).Data());
  //resolution_phi = JetResolution(getJERPhiFileName(type).Data());
  resolution_sf = JetResolutionScaleFactor(getJERSFFileName(type).Data());

  curdir->cd();

  return res;
}
void JERScaleFactorHandler::reset(){
  resolution_pt = JetResolution();
  //resolution_phi = JetResolution();
  resolution_sf = JetResolutionScaleFactor();
}

bool JERScaleFactorHandler::applyJER(AK4JetObject* jet, float const& rho) const{
  if (!(type==kAK4PFCHS || type==kAK4PFPuppi)){
    IVYerr << "JERScaleFactorHandler::applyJER: Mismatch between object class and correction type " << type << ". Aborting..." << endl;
    assert(0);
    return false;
  }

  IvyParticle::LorentzVector_t p4_JECNominal = jet->uncorrected_p4() * jet->extras.JECNominal;
  float jpt = p4_JECNominal.Pt();
  float jeta = p4_JECNominal.Eta();
  float jphi = p4_JECNominal.Phi();

  JetParameters const jet_parameters{
    { JERBinning::JetPt, jpt },
    { JERBinning::JetEta, jeta },
    //{ JERBinning::JetPhi, jphi },
    { JERBinning::Rho, rho }
  };
  float res_pt  = resolution_pt.getResolution(jet_parameters);

  float sf    = resolution_sf.getScaleFactor(jet_parameters);
  float sf_dn = resolution_sf.getScaleFactor(jet_parameters, kJERDown);
  float sf_up = resolution_sf.getScaleFactor(jet_parameters, kJERUp);

  GenJetObject* genjet = nullptr;
  for (auto const& part:jet->getMothers()){
    genjet = dynamic_cast<GenJetObject*>(part);
    if (genjet) break;
  }
  bool hasMatched = (genjet!=nullptr);
  bool isMatched = hasMatched;
  float gen_pt=0;
  if (hasMatched){
    double deltaR = jet->deltaR(genjet);
    gen_pt = genjet->pt();
    float diff_pt = std::abs(jpt - gen_pt);
    isMatched = (deltaR < jet->ConeRadiusConstant/2. && diff_pt < 3.*res_pt*jpt);
  }

  float pt_jer=jpt, pt_jerdn=jpt, pt_jerup=jpt;
  if (isMatched){
    pt_jer   = std::max(0.f, gen_pt + sf   *(jpt-gen_pt));
    pt_jerdn = std::max(0.f, gen_pt + sf_dn*(jpt-gen_pt));
    pt_jerup = std::max(0.f, gen_pt + sf_up*(jpt-gen_pt));
  }
  else{
    TRandom3 rand;
    rand.SetSeed(std::abs(static_cast<int>(std::sin(jphi)*100000)));
    float smear = rand.Gaus(0, 1.);
    float sigma   = std::sqrt(sf   *sf   -1.) * res_pt*jpt;
    float sigmadn = std::sqrt(sf_dn*sf_dn-1.) * res_pt*jpt;
    float sigmaup = std::sqrt(sf_up*sf_up-1.) * res_pt*jpt;
    pt_jer   = std::max(0.f, smear*sigma   + jpt);
    pt_jerdn = std::max(0.f, smear*sigmadn + jpt);
    pt_jerup = std::max(0.f, smear*sigmaup + jpt);
  }
  jet->extras.JERNominal = (pt_jer/jpt);
  jet->extras.JERDn = (pt_jerdn/jpt);
  jet->extras.JERUp = (pt_jerup/jpt);

  return true;
}
