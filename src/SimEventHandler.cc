#include <cassert>
#include "TRandom3.h"
#include "TDirectory.h"

#include "GlobalCollectionNames.h"
#include "SimEventHandler.h"
#include "SamplesCore.h"
#include "HelperFunctions.h"


using namespace std;
using namespace IvyStreamHelpers;


#define SIMEVENT_RNDVARIABLES \
SIMEVENT_RNDVARIABLE(unsigned long long, event) \
SIMEVENT_RNDVARIABLE(float, GenMET_pt) \
SIMEVENT_RNDVARIABLE(float, GenMET_phi)

#define SIMEVENT_PUVARIABLES \
SIMEVENT_PUVARIABLE(float, nTrueInt)

#define SIMEVENT_L1PREFIRINGVARIABLES \
SIMEVENT_L1PREFIRINGVARIABLE(float, prefiringWeight_Nominal, Nom) \
SIMEVENT_L1PREFIRINGVARIABLE(float, prefiringWeight_Dn, Dn) \
SIMEVENT_L1PREFIRINGVARIABLE(float, prefiringWeight_Up, Up)

#define SIMEVENT_ALLVARIABLES \
SIMEVENT_RNDVARIABLES \
SIMEVENT_PUVARIABLES \
SIMEVENT_L1PREFIRINGVARIABLES


SimEventHandler::SimEventHandler() :
  IvyBase(),
  doPUReweighting(true),
  hasHEM2018Issue(false),
  pileupWeights(3, -1)
{
#define SIMEVENT_RNDVARIABLE(TYPE, NAME) this->addConsumed<TYPE>(#NAME); this->defineConsumedSloppy(#NAME);
#define SIMEVENT_PUVARIABLE(TYPE, NAME) this->addConsumed<TYPE>(GlobalCollectionNames::colName_pileup + "_" + #NAME); this->defineConsumedSloppy(GlobalCollectionNames::colName_pileup + "_" + #NAME);
#define SIMEVENT_L1PREFIRINGVARIABLE(TYPE, NAME, NANONAME) if (SampleHelpers::getDataYear() == 2016 || SampleHelpers::getDataYear() == 2017){ this->addConsumed<TYPE>(GlobalCollectionNames::colName_l1prefiring + "_" + #NANONAME); this->defineConsumedSloppy(GlobalCollectionNames::colName_l1prefiring + "_" + #NANONAME); }
  SIMEVENT_ALLVARIABLES;
#undef SIMEVENT_L1PREFIRINGVARIABLE
#undef SIMEVENT_PUVARIABLE
#undef SIMEVENT_RNDVARIABLE

  setupPUHistograms();
}
SimEventHandler::~SimEventHandler(){
  clear();
  clearPUHistograms();
}

void SimEventHandler::clear(){
  this->resetCache();

  product_rnds.clear();
  product_rndnums.clear();
  theChosenDataPeriod = "";
  hasHEM2018Issue = false;
  for (auto& v:pileupWeights) v = -1;
  l1prefiringWeights.clear();
}

/*
How to obtain the data PU histograms:
1) Set up a CMSSW area other than CMSSW_10_2_X (10.6.26 works)
2) Run
pulatest=/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions16/13TeV/PileUp/UltraLegacy/pileup_latest.txt
pulatest=/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/PileUp/UltraLegacy/pileup_latest.txt
pulatest=/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/13TeV/PileUp/UltraLegacy/pileup_latest.txt
pileupCalc.py -i NtupleMaker/data/LumiJSON/${jsonfile} --inputLumiJSON ${pulatest} --calcMode true --minBiasXsec 69200 --maxPileupBin 100 --numPileupBins 100 ${jsonfile}_PUnominal.root
pileupCalc.py -i NtupleMaker/data/LumiJSON/${jsonfile} --inputLumiJSON ${pulatest} --calcMode true --minBiasXsec 66016.8 --maxPileupBin 100 --numPileupBins 100 ${jsonfile}_PUdn.root
pileupCalc.py -i NtupleMaker/data/LumiJSON/${jsonfile} --inputLumiJSON ${pulatest} --calcMode true --minBiasXsec 72383.2 --maxPileupBin 100 --numPileupBins 100 ${jsonfile}_PUup.root
The minBiasXsec variations are for the 4.6% uncertainty recommended in https://twiki.cern.ch/twiki/bin/view/CMS/PileupJSONFileforData#Pileup_JSON_Files_For_Run_II.
More information can be found there as well.
*/
void SimEventHandler::setupPUHistograms(){
  TDirectory* curdir = gDirectory;
  TDirectory* uppermostdir = SampleHelpers::rootTDirectory;

  std::vector<TString> dataperiods = SampleHelpers::getValidDataPeriods();

  TString mcpufile = Form("PU_MC_%i.root", SampleHelpers::getDataYear());
  std::unordered_map<TString, TString> datapucores;
  switch (SampleHelpers::getDataYear()){
  case 2016:
    datapucores = std::unordered_map<TString, TString>(
      {
      { "2016B", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_272007_lastRun_275376" },
      { "2016C", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_275657_lastRun_276283" },
      { "2016D", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_276315_lastRun_276811" },
      { "2016E", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_276831_lastRun_277420" },
      { "2016F_APV", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_277772_lastRun_278768_inc_278770_278806_278807" },
      { "2016F_NonAPV", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_278771_lastRun_278805_inc_278769_278808" },
      { "2016G", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_278820_lastRun_280385" },
      { "2016H", "Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_firstRun_280919_lastRun_284044" }
      }
    );
    break;
  case 2017:
    datapucores = std::unordered_map<TString, TString>(
      {
      { "2017B", "Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON_firstRun_297046_lastRun_299329" },
      { "2017C", "Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON_firstRun_299368_lastRun_302029" },
      { "2017D", "Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON_firstRun_302030_lastRun_303434" },
      { "2017E", "Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON_firstRun_303824_lastRun_304797" },
      { "2017F", "Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON_firstRun_305040_lastRun_306462" }
      }
    );
    break;
  case 2018:
    datapucores = std::unordered_map<TString, TString>(
      {
      { "2018A", "Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON_firstRun_315252_lastRun_316995" },
      { "2018B", "Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON_firstRun_317080_lastRun_319310" },
      { "2018C", "Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON_firstRun_319337_lastRun_320065" },
      { "2018D", "Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON_firstRun_320673_lastRun_325175" }
      }
    );
    break;
  case 2022:
  {
    doPUReweighting = false;
    IVYout << "SimEventHandler::setupPUHistograms: WARNING! Data year " << SampleHelpers::getDataYear() << " does not have PU histograms defined yet." << endl;
    return;
  }
  default:
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::setupPUHistograms: Data year " << SampleHelpers::getDataYear() << " is not defined." << endl;
    assert(0);
    break;
  }

  for (auto const& dp:dataperiods){
    if (datapucores.find(dp)==datapucores.end()){
      if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::setupPUHistograms: Data period " << dp << " does not have a corresponding PU reference." << endl;
      assert(0);
    }
  }

  TString cinput_pufile_main = ANALYSISPKGDATAPATH + "PileUp/";
  TFile* finput_mc = TFile::Open(cinput_pufile_main + mcpufile, "read");
  TH1F* hmc = nullptr;
  if (finput_mc){
    if (finput_mc->IsOpen() && finput_mc->IsZombie()) finput_mc->Close();
    else if (finput_mc->IsOpen()) hmc = (TH1F*) finput_mc->Get("pileup");
  }
  assert(hmc!=nullptr);
  hmc->Scale(1.f/hmc->Integral(0, hmc->GetNbinsX()+1));
  curdir->cd();

  auto it_dataperiod = dataperiods.cbegin();
  while (hmc && it_dataperiod != dataperiods.cend()){
    auto const& datapucore = datapucores.find(*it_dataperiod)->second;

    std::vector<TString> datapufiles{
      datapucore + "_PUnominal.root",
      datapucore + "_PUdn.root",
      datapucore + "_PUup.root"
    };
    map_DataPeriod_PUHistList[*it_dataperiod] = std::vector<TH1F*>(datapufiles.size(), nullptr);

    for (unsigned int isyst=0; isyst<datapufiles.size(); isyst++){
      auto const& datapufile = datapufiles.at(isyst);
      TH1F*& hfill = map_DataPeriod_PUHistList[*it_dataperiod].at(isyst);

      TFile* finput_data = TFile::Open(cinput_pufile_main + datapufile, "read");
      TH1F* hdata = nullptr;
      if (finput_data){
        if (finput_data->IsOpen() && finput_data->IsZombie()) finput_data->Close();
        else if (finput_data->IsOpen()) hdata = (TH1F*) finput_data->Get("pileup");
      }
      assert(hdata!=nullptr);
      assert(hdata->GetNbinsX() == hmc->GetNbinsX());
      hdata->Scale(1.f/hdata->Integral(0, hdata->GetNbinsX()+1));

      uppermostdir->cd();
      hfill = (TH1F*) hdata->Clone();
      HelperFunctions::divideHistograms(hdata, hmc, hfill, false);

      finput_data->Close();
      curdir->cd();
    }

    it_dataperiod++;
  }

  curdir->cd();

  finput_mc->Close();

  curdir->cd();
}
void SimEventHandler::clearPUHistograms(){
  for (auto& pp:map_DataPeriod_PUHistList){
    for (TH1F*& hh:pp.second) delete hh;
    pp.second.clear();
  }
  map_DataPeriod_PUHistList.clear();

  for (auto& hh:map_exceptionalPUHistList) delete hh.second;
  map_exceptionalPUHistList.clear();
}

bool SimEventHandler::constructSimEvent(){
  if (this->isAlreadyCached()) return true;

  clear();
  if (!currentTree) return false;
  if (SampleHelpers::checkSampleIsData(currentTree->sampleIdentifier)) return true;

  bool res = this->constructRandomNumbers() && this->constructPUWeight() && this->constructL1PrefiringWeight();

  if (res) this->cacheEvent();
  return res;
}
bool SimEventHandler::constructRandomNumbers(){
#define SIMEVENT_RNDVARIABLE(TYPE, NAME) TYPE const* NAME = nullptr;
  SIMEVENT_RNDVARIABLES;
#undef SIMEVENT_RNDVARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = true;
#define SIMEVENT_RNDVARIABLE(TYPE, NAME) allVariablesPresent &= this->getConsumed(#NAME, NAME);
  SIMEVENT_RNDVARIABLES;
#undef SIMEVENT_RNDVARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::constructRandomNumbers: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "SimEventHandler::constructRandomNumbers: All variables are set up!" << endl;

  // Get random number seeds first
  unsigned long long const rndDataPeriod = static_cast<unsigned long long>(std::abs((*GenMET_pt)*1000.f)) + ((*event) % 1000000);
  product_rnds[kDataPeriod_global] = product_rnds[kDataPeriod_local] = rndDataPeriod; // Seeds are supposed to be the same because the random numbers just translate between each other.
  unsigned long long const rndGenMETSmear = static_cast<unsigned long long>(std::abs(std::sin(*GenMET_phi))*100000.);
  product_rnds[kGenMETSmear] = rndGenMETSmear;

  // Set the random number generator
  TRandom3 rand;

  // Determine the MET smearing random number
  rand.SetSeed(rndGenMETSmear);
  product_rndnums[kGenMETSmear] = rand.Uniform();

  // Determine the chosen data period, and the global and local version of the data period random number
  double rndnum_dataPeriod_global = -1;
  double rndnum_dataPeriod_local = -1;
  theChosenDataPeriod = SampleHelpers::getRandomDataPeriod(rndDataPeriod, &rndnum_dataPeriod_global, &rndnum_dataPeriod_local);
  // Determine if SampleHelpers ignored the random number assignment because theDataPeriod is a specific period instead of the year.
  // If so, get the random number here.
  bool isSelfRandomEra = (rndnum_dataPeriod_local<0.);
  if (isSelfRandomEra){
    rand.SetSeed(rndDataPeriod);
    rndnum_dataPeriod_local = rand.Uniform();

    // Calculate the global random number from luminosity fractions
    std::vector<TString> const valid_periods = SampleHelpers::getValidDataPeriods();
    std::vector<double> lumilist; lumilist.reserve(valid_periods.size());
    for (TString const& period:valid_periods) lumilist.push_back(SampleHelpers::getIntegratedLuminosity(period));
    for (size_t il=1; il<lumilist.size(); il++) lumilist.at(il) += lumilist.at(il-1);
    for (size_t il=0; il<lumilist.size(); il++) lumilist.at(il) /= lumilist.back();
    for (unsigned char i_era=0; i_era<valid_periods.size(); i_era++){
      if (theChosenDataPeriod == valid_periods.at(i_era)){
        rndnum_dataPeriod_global = rndnum_dataPeriod_local*lumilist.at(i_era);
        if (i_era>0) rndnum_dataPeriod_global += lumilist.at(i_era-1);
        break;
      }
    }

    assert(rndnum_dataPeriod_global>=0.);
  }
  product_rndnums[kDataPeriod_global] = rndnum_dataPeriod_global;
  product_rndnums[kDataPeriod_local] = rndnum_dataPeriod_local;

  if (theChosenDataPeriod == "2018C" || theChosenDataPeriod == "2018D") hasHEM2018Issue = true;
  else if (theChosenDataPeriod == "2018B"){
    double lumi_total = SampleHelpers::getIntegratedLuminosity(theChosenDataPeriod); // Use the chosen data period in order to be able to use the local random number
    double lumi_HEMaffected = SampleHelpers::getIntegratedLuminosity("2018B_HEMaffected");
    double lumi_nonHEM = lumi_total - lumi_HEMaffected;
    double rnd_thr = lumi_nonHEM/lumi_total;
    // The latter portion of the 2018B data set was affected by the HEM15/16 failure.
    hasHEM2018Issue = (rndnum_dataPeriod_local>rnd_thr); // Make sure > vs >= is consistent with SampleHelpers::translateRandomNumberToRunNumber
  }
  else hasHEM2018Issue = false;


  if (this->verbosity>=MiscUtils::DEBUG){
    IVYout << "SimEventHandler::constructRandomNumbers:\n\t- Random numbers used:" << endl;
    IVYout << "\t\t- Global data period = " << product_rndnums[kDataPeriod_global] << endl;
    IVYout << "\t\t- Local data period = " << product_rndnums[kDataPeriod_local] << endl;
    IVYout << "\t\t- MET smear = " << product_rndnums[kGenMETSmear] << endl;
    IVYout << "\t- Identified period: " << theChosenDataPeriod << endl;
    IVYout << "\t- Has HEM issue? " << (hasHEM2018Issue ? 'y' : 'n') << endl;
  }

  return true;
}
bool SimEventHandler::constructPUWeight(){
  if (!doPUReweighting){
    for (auto& pileupWeight:pileupWeights) pileupWeight = 1.f;
    return true;
  }

#define SIMEVENT_PUVARIABLE(TYPE, NAME) TYPE const* NAME = nullptr;
  SIMEVENT_PUVARIABLES;
#undef SIMEVENT_PUVARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = true;
#define SIMEVENT_PUVARIABLE(TYPE, NAME) allVariablesPresent &= this->getConsumed(GlobalCollectionNames::colName_pileup + "_" + #NAME, NAME);
  SIMEVENT_PUVARIABLES;
#undef SIMEVENT_PUVARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::constructPUWeight: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "SimEventHandler::constructPUWeight: All variables are set up!" << endl;

  if (!HelperFunctions::checkVarNanInf(*nTrueInt) || *nTrueInt<0){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::constructPUWeight: Number of true interactions (" << *nTrueInt << ") is invalid." << endl;
    for (auto& pileupWeight:pileupWeights) pileupWeight = 0.f;
    return true;
  }

  auto it = map_DataPeriod_PUHistList.find(theChosenDataPeriod);
  if (it == map_DataPeriod_PUHistList.cend()){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::constructPUWeight: Histogram map for period \'" << theChosenDataPeriod << "\' is not found!" << endl;
    assert(0);
    return false;
  }
  auto const& hlist = it->second;
  if (hlist.size()!=3){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::constructPUWeight: PU histogram list has size " << hlist.size() << ", but the expected size is 3." << endl;
    assert(0);
  }

  unsigned short isyst = 0;
  for (auto const& hpu_data_MC:hlist){
    if (this->verbosity>=MiscUtils::DEBUG) IVYout << "SimEventHandler::constructPUWeight: Systematics index = " << isyst << endl;
    float pileupWeight = hpu_data_MC->GetBinContent(hpu_data_MC->FindBin(*nTrueInt));
    if (this->verbosity>=MiscUtils::DEBUG) IVYout
      << "SimEventHandler::constructPUWeight: Extracted PU ratio " << pileupWeight
      << " from " << hpu_data_MC->GetName()
      << " evaluated at N_true^int = " << *nTrueInt
      << endl;
    if (pileupWeight == 0.f){
      if (this->verbosity>=MiscUtils::ERROR) IVYerr
        << "SimEventHandler::constructPUWeight: PU weight = 0 for nTrueInt = " << *nTrueInt
        << ", isyst = " << isyst
        << ", chosen data period = " << theChosenDataPeriod
        << "!" << endl;
    }
    pileupWeights.at(isyst) = pileupWeight;
    isyst++;
  }

  return true;
}
bool SimEventHandler::constructL1PrefiringWeight(){
  if (!(SampleHelpers::getDataYear() == 2016 || SampleHelpers::getDataYear() == 2017)) return true;

#define SIMEVENT_L1PREFIRINGVARIABLE(TYPE, NAME, NANONAME) TYPE const* NAME = nullptr;
  SIMEVENT_L1PREFIRINGVARIABLES;
#undef SIMEVENT_L1PREFIRINGVARIABLE

  // Beyond this point starts checks and selection
  bool allVariablesPresent = true;
#define SIMEVENT_L1PREFIRINGVARIABLE(TYPE, NAME, NANONAME) allVariablesPresent &= this->getConsumed(GlobalCollectionNames::colName_l1prefiring + "_" + #NANONAME, NAME);
  SIMEVENT_L1PREFIRINGVARIABLES;
#undef SIMEVENT_L1PREFIRINGVARIABLE

  if (!allVariablesPresent){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::constructL1PrefiringWeight: Not all variables are consumed properly!" << endl;
    assert(0);
  }
  if (this->verbosity>=MiscUtils::DEBUG) IVYout << "SimEventHandler::constructL1PrefiringWeight: All variables are set up!" << endl;

  l1prefiringWeights = std::vector<float const*>{
    prefiringWeight_Nominal,
    prefiringWeight_Dn,
    prefiringWeight_Up
  };

  return true;
}

void SimEventHandler::bookBranches(BaseTree* tree){
  if (!tree) return;
  if (SampleHelpers::checkSampleIsData(tree->sampleIdentifier)) return;

#define SIMEVENT_RNDVARIABLE(TYPE, NAME) tree->bookBranch<TYPE>(#NAME, 0);
#define SIMEVENT_PUVARIABLE(TYPE, NAME) tree->bookBranch<TYPE>(GlobalCollectionNames::colName_pileup + "_" + #NAME, 0);
#define SIMEVENT_L1PREFIRINGVARIABLE(TYPE, NAME, NANONAME) if (SampleHelpers::getDataYear() == 2016 || SampleHelpers::getDataYear() == 2017) tree->bookBranch<TYPE>(GlobalCollectionNames::colName_l1prefiring + "_" + #NANONAME, 1);
  SIMEVENT_ALLVARIABLES;
#undef SIMEVENT_L1PREFIRINGVARIABLE
#undef SIMEVENT_PUVARIABLE
#undef SIMEVENT_RNDVARIABLE
}

TString const& SimEventHandler::getChosenDataPeriod() const{
  if (theChosenDataPeriod == ""){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::getChosenDataPeriod: SimEventHandler::constructSimEvent() needs to be called first..." << endl;
    assert(0);
  }
  return theChosenDataPeriod;
}
int SimEventHandler::getChosenRunNumber() const{
  if (theChosenDataPeriod == ""){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::getChosenRunNumber: SimEventHandler::constructSimEvent() needs to be called first..." << endl;
    assert(0);
  }
  return SampleHelpers::translateRandomNumberToRunNumber(theChosenDataPeriod, product_rndnums.find(kDataPeriod_local)->second);
}

unsigned long long const& SimEventHandler::getRandomNumberSeed(SimEventHandler::EventRandomNumberType type) const{
  auto it = product_rnds.find(type);
  if (it == product_rnds.cend()){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::getRandomNumberSeed: SimEventHandler::constructSimEvent() needs to be called first..." << endl;
    assert(0);
  }
  return it->second;
}

double const& SimEventHandler::getRandomNumber(SimEventHandler::EventRandomNumberType type) const{
  auto it = product_rndnums.find(type);
  if (it == product_rndnums.cend()){
    if (this->verbosity>=MiscUtils::ERROR) IVYerr << "SimEventHandler::getRandomNumber: SimEventHandler::constructSimEvent() needs to be called first..." << endl;
    assert(0);
  }
  return it->second;
}

float const& SimEventHandler::getPileUpWeight(SystematicsHelpers::SystematicVariationTypes const& syst) const{
  unsigned short const isyst = (syst == SystematicsHelpers::ePUDn)*1 + (syst == SystematicsHelpers::ePUUp)*2;
  return pileupWeights.at(isyst);
}
float SimEventHandler::getL1PrefiringWeight(SystematicsHelpers::SystematicVariationTypes const& syst) const{
  if (l1prefiringWeights.empty()) return 1;
  unsigned short const isyst = (syst == SystematicsHelpers::eL1PrefiringDn)*1 + (syst == SystematicsHelpers::eL1PrefiringUp)*2;
  return *(l1prefiringWeights.at(isyst));
}


#undef SIMEVENT_ALLVARIABLES
#undef SIMEVENT_L1PREFIRINGVARIABLES
#undef SIMEVENT_PUVARIABLES
#undef SIMEVENT_RNDVARIABLES
