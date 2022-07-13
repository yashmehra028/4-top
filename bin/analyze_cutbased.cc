#include <string>
#include <iostream>
#include <iomanip>
#include "TSystem.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TTree.h"
#include "TBranch.h"
#include "TChain.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "IvyFramework/IvyDataTools/interface/BaseTree.h"
#include "GlobalCollectionNames.h"
#include "SamplesCore.h"
#include "MuonSelectionHelpers.h"
#include "ElectronSelectionHelpers.h"
#include "AK4JetSelectionHelpers.h"
#include "IsotrackSelectionHelpers.h"
#include "ParticleSelectionHelpers.h"
#include "ParticleDisambiguator.h"
#include "MuonHandler.h"
#include "ElectronHandler.h"
#include "JetMETHandler.h"
#include "EventFilterHandler.h"
#include "SimEventHandler.h"
#include "GenInfoHandler.h"
#include "IsotrackHandler.h"
#include "SamplesCore.h"
#include "FourTopTriggerHelpers.h"
#include "DileptonHandler.h"
#include "SplitFileAndAddForTransfer.h"


using namespace std;
using namespace HelperFunctions;
using namespace IvyStreamHelpers;


struct SelectionTracker{
  std::vector<TString> ordered_reqs;
  std::unordered_map<TString, std::pair<double, double>> req_sumws_pair_map;

  void accumulate(TString const& strsel, double const& wgt);
  void print() const;
};
void SelectionTracker::accumulate(TString const& strsel, double const& wgt){
  if (!HelperFunctions::checkListVariable(ordered_reqs, strsel)){
    req_sumws_pair_map[strsel] = std::pair<double, double>(0, 0);
    ordered_reqs.push_back(strsel);
  }
  auto it_req_sumws_pair = req_sumws_pair_map.find(strsel);
  it_req_sumws_pair->second.first += wgt;
  it_req_sumws_pair->second.second += wgt*wgt;
}
void SelectionTracker::print() const{
  IVYout << "Selection summary:" << endl;
  for (auto const& strsel:ordered_reqs){
    auto it_req_sumws_pair = req_sumws_pair_map.find(strsel);
    IVYout << "\t- " << strsel << ": " << setprecision(15) << it_req_sumws_pair->second.first << " +- " << std::sqrt(it_req_sumws_pair->second.second) << endl;
  }
}


int ScanChain(std::string const& strdate, ::string const& dset, std::string const& proc, double const& xsec){
  if (!SampleHelpers::checkRunOnCondor()) std::signal(SIGINT, SampleHelpers::setSignalInterrupt);

  TDirectory* curdir = gDirectory;

  float const absEtaThr_ak4jets = (SampleHelpers::getDataYear()<=2016 ? AK4JetSelectionHelpers::etaThr_btag_Phase0Tracker : AK4JetSelectionHelpers::etaThr_btag_Phase1Tracker);

  TString coutput_main = ANALYSISPKGPATH + "test/output/Analysis_CutBased/" + strdate.data() + "/" + SampleHelpers::getDataPeriod();
  HostHelpers::ExpandEnvironmentVariables(coutput_main);
  gSystem->mkdir(coutput_main, true);
  TString stroutput = coutput_main + "/" + proc.data() + ".root";

  std::vector<TriggerHelpers::TriggerType> requiredTriggers_Dilepton{
    TriggerHelpers::kDoubleMu,
    TriggerHelpers::kDoubleEle,
    TriggerHelpers::kMuEle
  };
  // These PFHT triggers were used in the 2016 analysis. We keep them for now, but we could drop them later.
  if (SampleHelpers::getDataYear()==2016) requiredTriggers_Dilepton = std::vector<TriggerHelpers::TriggerType>{
      TriggerHelpers::kDoubleMu_PFHT,
      TriggerHelpers::kDoubleEle_PFHT,
      TriggerHelpers::kMuEle_PFHT
  };
  std::vector<std::string> const hltnames_Dilepton = TriggerHelpers::getHLTMenus(requiredTriggers_Dilepton);

  // Declare handlers
  GenInfoHandler genInfoHandler;
  SimEventHandler simEventHandler;
  EventFilterHandler eventFilter(requiredTriggers_Dilepton);
  MuonHandler muonHandler;
  ElectronHandler electronHandler;
  JetMETHandler jetHandler;
  IsotrackHandler isotrackHandler;

  // These are called handlers, but they are more like helpers.
  DileptonHandler dileptonHandler;
  ParticleDisambiguator particleDisambiguator;

  //dileptonHandler.setVerbosity(MiscUtils::DEBUG);
  // Disable some advanced event tracking for skims
  eventFilter.setTrackDataEvents(true);
  eventFilter.setCheckUniqueDataEvent(true);
  eventFilter.setCheckHLTPathRunRanges(true);

  curdir->cd();

  // Acquire input tree/chain
  TString strinput = SampleHelpers::getInputDirectory() + "/" + SampleHelpers::getDataPeriod() + "/" + proc.data();
  TString cinput = strinput + "/*.root";
  BaseTree* tin = new BaseTree(cinput, "Events", "", "");
  tin->sampleIdentifier = SampleHelpers::getSampleIdentifier(dset);
  bool const isData = SampleHelpers::checkSampleIsData(tin->sampleIdentifier);
  if (!isData && xsec<0.){
    IVYerr << "xsec = " << xsec << " is not valid." << endl;
    assert(0);
  }

  double sum_wgts = (isData ? 1 : 0);
  for (auto const& fname:SampleHelpers::lsdir(strinput.Data())){
    if (fname.EndsWith(".root")){
      TFile* ftmp = TFile::Open(strinput + "/" + fname, "read");
      TH2D* hCounters = (TH2D*) ftmp->Get("Counters");
      sum_wgts += hCounters->GetBinContent(1, 1);
      ftmp->Close();
    }
  }
  if (sum_wgts==0.){
    IVYerr << "Sum of pre-recorded weights cannot be zero." << endl;
    assert(0);
  }

  curdir->cd();

  double const lumi = SampleHelpers::getIntegratedLuminosity(SampleHelpers::getDataPeriod());
  double norm_scale = (isData ? 1. : xsec * xsecScale * lumi)/sum_wgts;

  IVYout << "Valid data periods for " << SampleHelpers::getDataPeriod() << ": " << SampleHelpers::getValidDataPeriods() << endl;
  IVYout << "Integrated luminosity: " << lumi << endl;
  IVYout << "Acquired a sum of weights of " << sum_wgts << ". Overall normalization will be " << norm_scale << "." << endl;

  curdir->cd();

  // Wrap the ivies around the input tree
  genInfoHandler.bookBranches(tin);
  genInfoHandler.wrapTree(tin);

  simEventHandler.bookBranches(tin);
  simEventHandler.wrapTree(tin);

  eventFilter.bookBranches(tin);
  eventFilter.wrapTree(tin);

  muonHandler.bookBranches(tin);
  muonHandler.wrapTree(tin);

  electronHandler.bookBranches(tin);
  electronHandler.wrapTree(tin);

  jetHandler.bookBranches(tin);
  jetHandler.wrapTree(tin);

  isotrackHandler.bookBranches(tin);
  isotrackHandler.wrapTree(tin);

  TFile* foutput = TFile::Open(stroutput, "recreate");
  foutput->cd();
  TH2D* hCat = new TH2D("hCat", "", 15, 0, 15, 2, 0, 2); hCat->Sumw2();
  for (int ix=1; ix<=15; ix++){
    if (ix<15) hCat->GetXaxis()->SetBinLabel(ix, Form("SR%i", ix));
    else hCat->GetXaxis()->SetBinLabel(ix, "CRW");
  }
  hCat->GetYaxis()->SetBinLabel(1, "SR");
  hCat->GetYaxis()->SetBinLabel(2, "CRZ");

  curdir->cd();

  // Keep track of sums of weights
  SelectionTracker seltracker;

  bool firstOutputEvent = true;
  unsigned int n_traversed = 0;
  unsigned int n_recorded = 0;
  int nEntries = tin->getNEvents();
  IVYout << "Looping over " << nEntries << " events..." << endl;
  for (int ev=0; ev<nEntries; ev++){
    if (SampleHelpers::doSignalInterrupt==1) break;

    tin->getEvent(ev);
    HelperFunctions::progressbar(ev, nEntries);
    n_traversed++;

    genInfoHandler.constructGenInfo();
    auto const& genInfo = genInfoHandler.getGenInfo();

    simEventHandler.constructSimEvent();

    double wgt = 1;
    if (!isData){
      double genwgt = 1;
      genwgt = genInfo->getGenWeight(SystematicsHelpers::sNominal);

      double puwgt = 1;
      puwgt = simEventHandler.getPileUpWeight(SystematicsHelpers::sNominal);

      wgt = genwgt * puwgt;
    }

    muonHandler.constructMuons();
    electronHandler.constructElectrons();
    jetHandler.constructJetMET(&simEventHandler);
    particleDisambiguator.disambiguateParticles(&muonHandler, &electronHandler, nullptr, &jetHandler);

    auto const& muons = muonHandler.getProducts();
    std::vector<MuonObject*> muons_selected;
    std::vector<MuonObject*> muons_tight;
    std::vector<MuonObject*> muons_loose;
    for (auto const& part:muons){
      if (part->pt()<5.) continue;
      if (ParticleSelectionHelpers::isTightParticle(part)) muons_tight.push_back(part);
      else if (ParticleSelectionHelpers::isLooseParticle(part)) muons_loose.push_back(part);
    }
    HelperFunctions::appendVector(muons_selected, muons_tight);
    HelperFunctions::appendVector(muons_selected, muons_loose);

    auto const& electrons = electronHandler.getProducts();
    std::vector<ElectronObject*> electrons_selected;
    std::vector<ElectronObject*> electrons_tight;
    std::vector<ElectronObject*> electrons_loose;
    for (auto const& part:electrons){
      if (part->pt()<7.) continue;
      if (ParticleSelectionHelpers::isTightParticle(part)) electrons_tight.push_back(part);
      else if (ParticleSelectionHelpers::isLooseParticle(part)) electrons_loose.push_back(part);
    }
    HelperFunctions::appendVector(electrons_selected, electrons_tight);
    HelperFunctions::appendVector(electrons_selected, electrons_loose);

    unsigned int const nleptons_tight = muons_tight.size() + electrons_tight.size();
    unsigned int const nleptons_loose = muons_loose.size() + electrons_loose.size();
    unsigned int const nleptons_selected = nleptons_tight + nleptons_loose;

    std::vector<ParticleObject*> leptons_selected; leptons_selected.reserve(nleptons_selected);
    for (auto const& part:muons_selected) leptons_selected.push_back(dynamic_cast<ParticleObject*>(part));
    for (auto const& part:electrons_selected) leptons_selected.push_back(dynamic_cast<ParticleObject*>(part));
    ParticleObjectHelpers::sortByGreaterPt(leptons_selected);

    std::vector<ParticleObject*> leptons_tight; leptons_tight.reserve(nleptons_tight);
    for (auto const& part:muons_tight) leptons_tight.push_back(dynamic_cast<ParticleObject*>(part));
    for (auto const& part:electrons_tight) leptons_tight.push_back(dynamic_cast<ParticleObject*>(part));
    ParticleObjectHelpers::sortByGreaterPt(leptons_tight);

    double ak4jets_pt4_HT=0;
    auto const& ak4jets = jetHandler.getAK4Jets();
    std::vector<AK4JetObject*> ak4jets_tight_pt40;
    std::vector<AK4JetObject*> ak4jets_tight_pt40_btagged;
    for (auto const& jet:ak4jets){
      if (ParticleSelectionHelpers::isTightJet(jet) && jet->pt()>=40. && std::abs(jet->eta())<absEtaThr_ak4jets){
        ak4jets_tight_pt40.push_back(jet);
        ak4jets_pt4_HT += jet->pt();
        if (jet->testSelectionBit(AK4JetSelectionHelpers::kPreselectionTight_BTagged)) ak4jets_tight_pt40_btagged.push_back(jet);
      }
    }
    unsigned int const nak4jets_tight_pt40 = ak4jets_tight_pt40.size();
    unsigned int const nak4jets_tight_pt40_btagged = ak4jets_tight_pt40_btagged.size();

    auto const& eventmet = jetHandler.getPFMET();

    // BEGIN PRESELECTION
    seltracker.accumulate("Full sample", wgt);

    if (nak4jets_tight_pt40_btagged<2) continue;
    seltracker.accumulate("Pass Nj and Nb", wgt);

    if (eventmet->pt()<50.) continue;
    seltracker.accumulate("Pass pTmiss", wgt);

    if (ak4jets_pt4_HT<300.) continue;
    seltracker.accumulate("Pass HT", wgt);

    if (nleptons_selected<2 || nleptons_selected>=5 || nleptons_loose>1) continue;
    seltracker.accumulate("Has >=2 and <=4 leptons, >=2 of which are tight", wgt);

    if (nleptons_tight>=2 && (leptons_tight.front()->pt()<25. || leptons_tight.at(1)->pt()<20.)) continue;
    seltracker.accumulate("Pass pT1 and pT2", wgt);

    if (nleptons_tight>=3 && leptons_tight.at(2)->pt()<20.) continue;
    seltracker.accumulate("Pass pT3 if >=3 tight leptons", wgt);

    // Construct all possible dilepton pairs
    int nQ = 0;
    for (auto const& part:leptons_tight) nQ += (part->pdgId()>0 ? -1 : 1);
    if (std::abs(nQ)>=(6-static_cast<int>(nleptons_tight))) continue; // This req. necessarily vetoes Nleps>=5 because the actual number of same-sign leptons will always be >=3.
    seltracker.accumulate("Pass 3-lepton same charge veto", wgt);

    dileptonHandler.constructDileptons(&muons_selected, &electrons_selected);
    auto const& dileptons = dileptonHandler.getProducts();
    unsigned int ndileptons_SS = 0;
    unsigned int ndileptons_OS = 0;
    for (auto const& dilepton:dileptons){
      if (dilepton->isOS()) ndileptons_OS++;
      else ndileptons_SS++;
    }
    seltracker.accumulate("nOS", wgt*static_cast<double>(ndileptons_OS));
    seltracker.accumulate("nSS", wgt*static_cast<double>(ndileptons_SS));

    bool fail_vetos = false;
    DileptonObject* dilepton_SS_tight = nullptr;
    DileptonObject* dilepton_OS_DYCand_tight = nullptr;
    for (auto const& dilepton:dileptons){
      bool isSS = !dilepton->isOS();
      bool isTight = dilepton->nTightDaughters()==2;
      bool isSF = dilepton->isSF();
      bool is_DYClose = std::abs(dilepton->m()-91.2)<15. || dilepton->m()<12.;
      if (isSS && isSF && is_DYClose && std::abs(dilepton->getDaughter(0)->pdgId())==11){
        fail_vetos = true;
        break;
      }
      if (isSS && isTight && !dilepton_SS_tight) dilepton_SS_tight = dilepton;
      if (!isSS && isSF && isTight && is_DYClose && !dilepton_OS_DYCand_tight) dilepton_OS_DYCand_tight = dilepton;
    }

    if (fail_vetos) continue;
    seltracker.accumulate("Pass dilepton vetos", wgt);

    if (!dilepton_SS_tight) continue;
    seltracker.accumulate("Has at least one tight SS dilepton", wgt);

    // Put event filters to the last because data has unique event tracking enabled.
    eventFilter.constructFilters(&simEventHandler);
    if (!eventFilter.test2018HEMFilter(&simEventHandler, nullptr, nullptr, &ak4jets)) continue; // Test for 2018 partial HEM failure
    seltracker.accumulate("Pass HEM veto", wgt);
    if (!eventFilter.passMETFilters()) continue; // Test for MET filters
    seltracker.accumulate("Pass MET filters", wgt);
    if (!eventFilter.isUniqueDataEvent()) continue;
    seltracker.accumulate("Pass unique event check", wgt);

    float event_weight_triggers_dilepton = eventFilter.getTriggerWeight(hltnames_Dilepton);
    if (event_weight_triggers_dilepton==0.) continue; // Test if any triggers passed at all
    seltracker.accumulate("Pass any trigger", wgt);

    // Accumulate any ME weights and K factors that might be present

    /*************************************************/
    /* NO MORE CALLS TO SELECTION BEYOND THIS POINT! */
    /*************************************************/
    int iCRZ = (dilepton_OS_DYCand_tight ? 1 : 0);
    int icat = -1;
    if (nleptons_tight==2){
      switch (nak4jets_tight_pt40_btagged){
      case 2:
        if (nak4jets_tight_pt40<6) icat=15;
        else icat = 1 + std::min(nak4jets_tight_pt40, static_cast<unsigned int>(8))-6;
        break;
      case 3:
        if (nak4jets_tight_pt40>=5) icat = 4 + std::min(nak4jets_tight_pt40, static_cast<unsigned int>(8))-5;
        break;
      default: // Nb>=4
        if (nak4jets_tight_pt40>=5) icat = 8;
        break;
      }
    }
    else{
      switch (nak4jets_tight_pt40_btagged){
      case 2:
        if (nak4jets_tight_pt40>=5) icat = 9 + std::min(nak4jets_tight_pt40, static_cast<unsigned int>(7))-5;
        break;
      default: // Nb>=3
        if (nak4jets_tight_pt40>=4) icat = 12 + std::min(nak4jets_tight_pt40, static_cast<unsigned int>(6))-4;
        break;
      }
    }
    if (icat>=0) hCat->Fill(static_cast<double>(icat-1)+0.5, static_cast<double>(iCRZ)+0.5, wgt);

    n_recorded++;

    if (firstOutputEvent) firstOutputEvent = false;
  }

  IVYout << "Number of events recorded: " << n_recorded << " / " << n_traversed << " / " << nEntries << endl;
  seltracker.print();

  if (n_traversed>0) hCat->Scale(norm_scale * static_cast<double>(nEntries) / static_cast<double>(n_traversed));

  IVYout << "Event counts for the SR:" << endl;
  for (int ix=1; ix<=hCat->GetNbinsX(); ix++) IVYout << "\t- " << hCat->GetXaxis()->GetBinLabel(ix) << ": " << hCat->GetBinContent(ix, 1) << " +- " << hCat->GetBinError(ix, 1) << endl;
  IVYout << "Event counts for the CRZ:" << endl;
  for (int ix=1; ix<=hCat->GetNbinsX(); ix++) IVYout << "\t- " << hCat->GetXaxis()->GetBinLabel(ix) << ": " << hCat->GetBinContent(ix, 2) << " +- " << hCat->GetBinError(ix, 2) << endl;

  foutput->WriteTObject(hCat);
  delete hCat;
  foutput->Close();

  curdir->cd();
  delete tin;

  SampleHelpers::splitFileAndAddForTransfer(stroutput);
  return 0;
}

int main(int argc, char** argv){
  constexpr int iarg_offset=1; // argv[0]==[Executable name]

  bool print_help=false, has_help=false;
  std::string str_dset;
  std::string str_proc;
  std::string str_period;
  std::string str_tag;
  std::string str_outtag;
  double xsec = -1;
  for (int iarg=iarg_offset; iarg<argc; iarg++){
    std::string strarg = argv[iarg];
    std::string wish, value;
    splitOption(strarg, wish, value, '=');

    if (wish.empty()){
      if (value=="help"){ print_help=has_help=true; }
      else{
        IVYerr << "ERROR: Unknown argument " << value << endl;
        print_help=true;
      }
    }
    else if (wish=="dataset") str_dset = value;
    else if (wish=="short_name") str_proc = value;
    else if (wish=="period") str_period = value;
    else if (wish=="input_tag") str_tag = value;
    else if (wish=="output_tag") str_outtag = value;
    else if (wish=="xsec"){
      if (xsec<0.) xsec = 1;
      xsec *= std::stod(value);
    }
    else if (wish=="BR"){
      if (xsec<0.) xsec = 1;
      xsec *= std::stod(value);
    }
    else{
      IVYerr << "ERROR: Unknown argument " << wish << "=" << value << endl;
      print_help=true;
    }
  }

  if (!print_help && (str_proc=="" || str_dset=="" || str_period=="" || str_tag=="" || str_outtag=="")){
    IVYerr << "ERROR: Not all mandatory inputs are present." << endl;
    print_help=true;
  }

  if (print_help){
    IVYout << "skim_UL options:\n\n";
    IVYout << "- help: Prints this help message.\n";
    IVYout << "- dataset: Data set name. Mandatory.\n";
    IVYout << "- short_name: Process short name. Mandatory.\n";
    IVYout << "- period: Data period. Mandatory.\n";
    IVYout << "- input_tag: Version of the input skims. Mandatory.\n";
    IVYout << "- output_tag: Version of the output. Mandatory.\n";
    IVYout << "- xsec: Cross section value. Mandatory in the MC.\n";
    IVYout << "- BR: BR value. Mandatory in the MC.\n";

    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  SampleHelpers::configure(str_period, Form("skims:%s", str_tag.data()), HostHelpers::kUCSDT2);

  return ScanChain(str_outtag, str_dset, str_proc, xsec);
}
