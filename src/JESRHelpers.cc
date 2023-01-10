#include <cassert>
#include <unordered_map>
#include "JESRHelpers.h"
#include "SamplesCore.h"
#include "IvyFramework/IvyDataTools/interface/HostHelpersCore.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


TString JESRHelpers::getJetTypeName(JetType type){
  if (type==kAK4PFCHS) return "AK4PFchs";
  if (type==kAK4PFPuppi) return "AK4PFPuppi";
  else if (type==kAK8PFPuppi) return "AK8PFPuppi";
  else{ IVYerr << "JESRHelpers::getJetTypeName: JetType " << type << " is not implemented!" << endl; assert(0); return ""; }
}

TString JESRHelpers::getJESFilePath(bool isMC){
  std::unordered_map<TString, TString> eraMap;
  auto const& dy = SampleHelpers::getDataYear();
  auto const& dp = SampleHelpers::getDataPeriod();
  switch (dy){
  case 2016:
    eraMap["2016B"] = eraMap["2016C"] = eraMap["2016D"] = "Summer19UL16APV_RunBCD_V7_DATA";
    eraMap["2016E"] = eraMap["2016F_APV"] = "Summer19UL16APV_RunEF_V7_DATA";
    eraMap["2016F_NonAPV"] = eraMap["2016G"] = eraMap["2016H"] = "Summer19UL16_RunFGH_V7_DATA";
    eraMap["MC_2016_APV"] = "Summer19UL16APV_V7_MC";
    eraMap["MC_2016_NonAPV"] = "Summer19UL16_V7_MC";
    break;
  case 2017:
    eraMap["2017B"] = "Summer19UL17_RunB_V5_DATA";
    eraMap["2017C"] = "Summer19UL17_RunC_V5_DATA";
    eraMap["2017D"] = "Summer19UL17_RunD_V5_DATA";
    eraMap["2017E"] = "Summer19UL17_RunE_V5_DATA";
    eraMap["2017F"] = "Summer19UL17_RunF_V5_DATA";
    eraMap["MC_2017"] = "Summer19UL17_V5_MC";
    break;
  case 2018:
    eraMap["2018A"] = "Summer19UL18_RunA_V5_DATA";
    eraMap["2018B"] = "Summer19UL18_RunB_V5_DATA";
    eraMap["2018C"] = "Summer19UL18_RunC_V5_DATA";
    eraMap["2018D"] = "Summer19UL18_RunD_V5_DATA";
    eraMap["MC_2018"] = "Summer19UL18_V5_MC";
    break;
  default:
    IVYerr << "JESRHelpers::getJESFilePath: Data year " << dy << " is not implemented. Aborting..." << endl;
    assert(0);
  }

  TString res = ANALYSISPKGDATAPATH + "external/JetMET/JES/";
  if (isMC){
    if (dy==2016) res += eraMap[(SampleHelpers::isAPV2016Affected(dp) ? "MC_2016_APV" : "MC_2016_NonAPV")];
    else res += eraMap[Form("MC_%i", dy)];
  }
  else res += eraMap[dp];

  HostHelpers::ExpandEnvironmentVariables(res);

  return res;
}

std::vector<TString> JESRHelpers::getJESFileNames(JetType type, bool isMC){
  std::vector<TString> res; res.reserve((isMC ? 3 : 4));

  TString stype = getJetTypeName(type);
  TString jespath = getJESFilePath(isMC);

  res.push_back(jespath+"_L1FastJet_"+stype+".txt");
  res.push_back(jespath+"_L2Relative_"+stype+".txt");
  res.push_back(jespath+"_L3Absolute_"+stype+".txt");
  if (!isMC) res.push_back(jespath+"_L2L3Residual_"+stype+".txt");
  return res;
}
TString JESRHelpers::getJESUncertaintyFileName(JetType type, bool isMC){
  TString stype = getJetTypeName(type);
  TString jespath = getJESFilePath(isMC);
  TString res = jespath+"_Uncertainty_"+stype+".txt";
  return res;
}

TString JESRHelpers::getJERFilePath(){
  TString res = ANALYSISPKGDATAPATH + "external/JetMET/JER/";
  auto const& year = SampleHelpers::getDataYear();
  auto const& dp = SampleHelpers::getDataPeriod();
  switch (year){
  case 2016:
    if (SampleHelpers::isAPV2016Affected(dp)) res += "Summer20UL16APV_JRV3_MC";
    else res += "Summer20UL16_JRV3_MC";
    break;
  case 2017:
    res += "Summer19UL17_JRV3_MC";
    break;
  case 2018:
    res += "Summer19UL18_JRV2_MC";
    break;
  default:
    IVYerr << "JESRHelpers::getJERFilePath: Data year " << year << " is not implemented. Aborting..." << endl;
    assert(0);
    res = "";
  }

  HostHelpers::ExpandEnvironmentVariables(res);

  return res;
}
TString JESRHelpers::getJERPtFileName(JetType type){
  TString res = getJERFilePath() + "_PtResolution_" + getJetTypeName(type) + ".txt";
  return res;
}
TString JESRHelpers::getJERPhiFileName(JetType type){
  TString res = getJERFilePath() + "_PhiResolution_" + getJetTypeName(type) + ".txt";
  return res;
}
TString JESRHelpers::getJERSFFileName(JetType type){
  TString res = getJERFilePath() + "_SF_" + getJetTypeName(type) + ".txt";
  return res;
}
