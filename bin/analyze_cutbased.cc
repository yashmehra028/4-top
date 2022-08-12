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
#include "IvyFramework/IvyDataTools/interface/SimpleEntry.h"
#include "GlobalCollectionNames.h"
#include "RunLumiEventBlock.h"
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

int ScanChain(std::string const& strdate, std::string const& dset, std::string const& proc, double const& xsec, SimpleEntry const& extra_arguments){
  if (!SampleHelpers::checkRunOnCondor()) std::signal(SIGINT, SampleHelpers::setSignalInterrupt);

  TDirectory* curdir = gDirectory;

  float const absEtaThr_ak4jets = (SampleHelpers::getDataYear()<=2016 ? AK4JetSelectionHelpers::etaThr_btag_Phase0Tracker : AK4JetSelectionHelpers::etaThr_btag_Phase1Tracker);

  // Turn on synchronization exercise options
  std::string input_files;
  extra_arguments.getNamedVal("input_files", input_files);
  bool runSyncExercise = false;
  extra_arguments.getNamedVal("run_sync", runSyncExercise);

  std::string muon_id_name;
  extra_arguments.getNamedVal("muon_id", muon_id_name);
  if (muon_id_name!=""){
    IVYout << "Switching to muon id " << muon_id_name << endl;
    MuonSelectionHelpers::setSelectionTypeByName(muon_id_name);
  }
  std::string electron_id_name;
  extra_arguments.getNamedVal("electron_id", electron_id_name);
  if (electron_id_name!=""){
    IVYout << "Switching to electron id " << electron_id_name << endl;
    ElectronSelectionHelpers::setSelectionTypeByName(electron_id_name);
  }


  // Flag to control whether any preselection other than nleps>=2 to be applied
  bool const applyPreselection = !runSyncExercise;

  // This is the output directory.
  // Output should always be recorded as if you are running the job locally.
  // We will inform the Condor job later on that some files would need transfer if we are running on Condor.
  TString coutput_main = ANALYSISPKGPATH + "test/output/Analysis_CutBased/" + strdate.data() + "/" + SampleHelpers::getDataPeriod();
  HostHelpers::ExpandEnvironmentVariables(coutput_main);
  gSystem->mkdir(coutput_main, true);
  TString stroutput = coutput_main + "/" + proc.data() + ".root"; // This is the output file.

  // Trigger configuration
  std::vector<TriggerHelpers::TriggerType> requiredTriggers_Dilepton{
    TriggerHelpers::kDoubleMu,
    TriggerHelpers::kDoubleEle,
    TriggerHelpers::kMuEle
  };
  // These PFHT triggers were used in the 2016 analysis. They are a bit more efficient.
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

  // Some advanced event filters
  eventFilter.setTrackDataEvents(true);
  eventFilter.setCheckUniqueDataEvent(true);
  eventFilter.setCheckHLTPathRunRanges(true);

  curdir->cd();

  // Acquire input tree/chain
  TString strinput = SampleHelpers::getInputDirectory() + "/" + SampleHelpers::getDataPeriod() + "/" + proc.data();
  TString cinput = (input_files=="" ? strinput + "/*.root" : strinput + "/" + input_files.data());
  IVYout << "Accessing input files " << cinput << "..." << endl;
  BaseTree* tin = new BaseTree(cinput, "Events", "", "");
  tin->sampleIdentifier = SampleHelpers::getSampleIdentifier(dset);
  bool const isData = SampleHelpers::checkSampleIsData(tin->sampleIdentifier);
  if (!isData && xsec<0.){
    IVYerr << "xsec = " << xsec << " is not valid." << endl;
    assert(0);
  }

  double sum_wgts = (isData ? 1 : 0);
  if (!isData){
    for (auto const& fname:SampleHelpers::lsdir(strinput.Data())){
      if (runSyncExercise && fname!=input_files.data()) continue;
      if (fname.EndsWith(".root")){
        TFile* ftmp = TFile::Open(strinput + "/" + fname, "read");
        TH2D* hCounters = (TH2D*) ftmp->Get("Counters");
        sum_wgts += hCounters->GetBinContent(1, 1);
        ftmp->Close();
      }
    }
  }
  if (sum_wgts==0.){
    IVYerr << "Sum of pre-recorded weights cannot be zero." << endl;
    assert(0);
  }

  curdir->cd();

  // Calculate the overall normalization scale on the events.
  // Includes xsec (in fb), lumi (in fb-1), and 1/sum of weights in all of the MC.
  // Data normalizaion factor is always 1.
  double const lumi = SampleHelpers::getIntegratedLuminosity(SampleHelpers::getDataPeriod());
  double norm_scale = (isData ? 1. : xsec * xsecScale * lumi)/sum_wgts;

  IVYout << "Valid data periods for " << SampleHelpers::getDataPeriod() << ": " << SampleHelpers::getValidDataPeriods() << endl;
  IVYout << "Integrated luminosity: " << lumi << endl;
  IVYout << "Acquired a sum of weights of " << sum_wgts << ". Overall normalization will be " << norm_scale << "." << endl;

  curdir->cd();

  // Wrap the ivies around the input tree:
  // Booking is basically SetBranchStatus+SetBranchAddress. You can book for as many trees as you would like.
  // In some cases, bookBranches also informs the ivy dynamically that it is supposed to consume certain entries.
  // For entries common to all years or any data or MC, the consumption information is handled in the ivy constructor already.
  // None of these mean the ivy establishes its access to the input tree yet.
  // Wrapping a tree informs the ivy that it is supposed to consume the booked entries from that particular tree.
  // Without wrapping, you are not really accessing the entries from the input tree to construct the physics objects;
  // all you would get are 0 electrons, 0 jets, everything failing event filters etc.
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

  EventNumber_t* ptr_EventNumber = nullptr;
  if (runSyncExercise){
    tin->bookBranch<EventNumber_t>("event", 0);
    tin->getValRef("event", ptr_EventNumber);
  }

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

  // Create a sync file
  ofstream foutput_sync;
  TString stroutput_sync = coutput_main + "/" + Form("sync_%s.csv", proc.data());
  if (runSyncExercise){
    foutput_sync.open(stroutput_sync.Data(), std::ios_base::out);
    foutput_sync << "Event#,#inFile,MET,MET_phi,Nlooseleptons,Nfakeableleptons,Ntightleptons,Goodsspair?,HT,Njets,Nbjets,SR/CR" << endl;
  }

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

      // Add L1 prefiring weight for 2016 and 2017
      //wgt *= simEventHandler.getL1PrefiringWeight(SystematicsHelpers::sNominal);
    }

    muonHandler.constructMuons();
    electronHandler.constructElectrons();
    jetHandler.constructJetMET(&simEventHandler);

    // !!!IMPORTANT!!!
    // NEVER USE LEPTONS AND JETS IN AN ANALYSIS BEFORE SOME FORM OF DISAMBIGUATION BETWEEN THEM!
    // Muon and electron handlers only apply mini. iso. reqs.
    // In order to compute pTratio and pTrel, you need jets.
    // ParticleDisambiguator does the matching, and assigns the overlapping jets (or closest ones) as 'mothers' of the leptons.
    // Once mothers are assigned, ParticleObject::ptratio and ptrel functions work as intended,
    // and you can apply the additional selections on these variables this way.
    // ParticleDisambiguator then cleans all geometrically overlapping jets by resetting their selection bits, which makes them unusable.
    particleDisambiguator.disambiguateParticles(&muonHandler, &electronHandler, nullptr, &jetHandler);

    bool const printObjInfo = runSyncExercise
      &&
      HelperFunctions::checkListVariable(std::vector<int>{3, 15, 30, 31, 32, 41, 153, 154, 197, 215, 284, 615}, ev);

    if (printObjInfo) IVYout << "Lepton info for event " << ev << ":" << endl;

    auto const& muons = muonHandler.getProducts();
    std::vector<MuonObject*> muons_selected;
    std::vector<MuonObject*> muons_tight;
    std::vector<MuonObject*> muons_fakeable;
    std::vector<MuonObject*> muons_loose;
    for (auto const& part:muons){
      if (part->pt()<5.) continue;

      bool is_tight = false;
      bool is_fakeable = false;
      bool is_loose = false;

      if (ParticleSelectionHelpers::isTightParticle(part)){
        muons_tight.push_back(part);
        is_loose = is_fakeable = is_tight = true;
      }
      else if (ParticleSelectionHelpers::isFakeableParticle(part)){
        muons_fakeable.push_back(part);
        is_loose = is_fakeable = true;
      }
      else if (ParticleSelectionHelpers::isLooseParticle(part)){
        muons_loose.push_back(part);
        is_loose = true;
      }

      if (printObjInfo) IVYout
        << "\t- PDG id = " << part->pdgId()
        << ", pt = " << part->pt() << ", eta = " << part->eta() << ", phi = " << part->phi()
        << ", loose? " << is_loose
        << ", fakeable? " << is_fakeable
        << ", tight? " << is_tight
        << endl;
    }
    HelperFunctions::appendVector(muons_selected, muons_tight);
    HelperFunctions::appendVector(muons_selected, muons_fakeable);
    HelperFunctions::appendVector(muons_selected, muons_loose);

    auto const& electrons = electronHandler.getProducts();
    std::vector<ElectronObject*> electrons_selected;
    std::vector<ElectronObject*> electrons_tight;
    std::vector<ElectronObject*> electrons_fakeable;
    std::vector<ElectronObject*> electrons_loose;
    for (auto const& part:electrons){
      if (part->pt()<7.) continue;

      bool is_tight = false;
      bool is_fakeable = false;
      bool is_loose = false;

      if (ParticleSelectionHelpers::isTightParticle(part)){
        electrons_tight.push_back(part);
        is_loose = is_fakeable = is_tight = true;
      }
      else if (ParticleSelectionHelpers::isFakeableParticle(part)){
        electrons_fakeable.push_back(part);
        is_loose = is_fakeable = true;
      }
      else if (ParticleSelectionHelpers::isLooseParticle(part)){
        electrons_loose.push_back(part);
        is_loose = true;
      }

      if (printObjInfo) IVYout
        << "\t- PDG id = " << part->pdgId()
        << ", pt = " << part->pt() << ", eta = " << part->eta() << ", phi = " << part->phi()
        << ", loose? " << is_loose
        << ", fakeable? " << is_fakeable
        << ", tight? " << is_tight
        << endl;
    }
    HelperFunctions::appendVector(electrons_selected, electrons_tight);
    HelperFunctions::appendVector(electrons_selected, electrons_fakeable);
    HelperFunctions::appendVector(electrons_selected, electrons_loose);

    unsigned int const nleptons_tight = muons_tight.size() + electrons_tight.size();
    unsigned int const nleptons_fakeable = muons_fakeable.size() + electrons_fakeable.size();
    unsigned int const nleptons_loose = muons_loose.size() + electrons_loose.size();
    unsigned int const nleptons_selected = nleptons_tight + nleptons_fakeable + nleptons_loose;

    std::vector<ParticleObject*> leptons_selected; leptons_selected.reserve(nleptons_selected);
    for (auto const& part:muons_selected) leptons_selected.push_back(dynamic_cast<ParticleObject*>(part));
    for (auto const& part:electrons_selected) leptons_selected.push_back(dynamic_cast<ParticleObject*>(part));
    ParticleObjectHelpers::sortByGreaterPt(leptons_selected);

    std::vector<ParticleObject*> leptons_tight; leptons_tight.reserve(nleptons_tight);
    for (auto const& part:muons_tight) leptons_tight.push_back(dynamic_cast<ParticleObject*>(part));
    for (auto const& part:electrons_tight) leptons_tight.push_back(dynamic_cast<ParticleObject*>(part));
    ParticleObjectHelpers::sortByGreaterPt(leptons_tight);

    if (printObjInfo) IVYout << "Jet info for event " << ev << ":" << endl;
    double ak4jets_pt40_HT=0;
    auto const& ak4jets = jetHandler.getAK4Jets();
    std::vector<AK4JetObject*> ak4jets_tight_pt40;
    std::vector<AK4JetObject*> ak4jets_tight_pt25_btagged;
    for (auto const& jet:ak4jets){
      if (printObjInfo) IVYout
        << "\t- pt = " << jet->pt() << ", eta = " << jet->eta() << ", phi = " << jet->phi()
        << " | btagged? " << jet->testSelectionBit(AK4JetSelectionHelpers::kPreselectionTight_BTagged)
        << " | tight & clean? " << ParticleSelectionHelpers::isTightJet(jet)
        << endl;
      if (ParticleSelectionHelpers::isTightJet(jet) && jet->pt()>=25. && std::abs(jet->eta())<absEtaThr_ak4jets){
        if (jet->testSelectionBit(AK4JetSelectionHelpers::kPreselectionTight_BTagged)) ak4jets_tight_pt25_btagged.push_back(jet);
        if (jet->pt()>=40.){
          ak4jets_tight_pt40.push_back(jet);
          ak4jets_pt40_HT += jet->pt();
        }
      }
    }
    unsigned int const nak4jets_tight_pt40 = ak4jets_tight_pt40.size();
    unsigned int const nak4jets_tight_pt25_btagged = ak4jets_tight_pt25_btagged.size();

    auto const& eventmet = jetHandler.getPFMET();

    // BEGIN PRESELECTION
    seltracker.accumulate("Full sample", wgt);

    if (applyPreselection && (nak4jets_tight_pt25_btagged<2 || nak4jets_tight_pt40<2)) continue;
    seltracker.accumulate("Pass Nj and Nb", wgt);

    if (applyPreselection && eventmet->pt()<50.) continue;
    seltracker.accumulate("Pass pTmiss", wgt);

    if (applyPreselection && ak4jets_pt40_HT<300.) continue;
    seltracker.accumulate("Pass HT", wgt);

    if (nleptons_tight<2) continue; // Skims are required to apply this selection, so no additional test on applyPreselection.
    if (applyPreselection && (nleptons_selected<2 || nleptons_selected>=5)) continue;
    seltracker.accumulate("Has >=2 and <=4 leptons, >=2 of which are tight", wgt);

    if (applyPreselection && (leptons_tight.front()->pt()<25. || leptons_tight.at(1)->pt()<20.)) continue;
    seltracker.accumulate("Pass pT1 and pT2", wgt);

    if (applyPreselection && (nleptons_tight>=3 && leptons_tight.at(2)->pt()<20.)) continue;
    seltracker.accumulate("Pass pT3 if >=3 tight leptons", wgt);

    // Construct all possible dilepton pairs
    int nQ = 0;
    for (auto const& part:leptons_tight) nQ += (part->pdgId()>0 ? -1 : 1);
    if (applyPreselection && (std::abs(nQ)>=(6-static_cast<int>(nleptons_tight)))) continue; // This req. necessarily vetoes Nleps>=5 because the actual number of same-sign leptons will always be >=3.
    seltracker.accumulate("Pass 3-lepton same charge veto", wgt);

    dileptonHandler.constructDileptons(&muons_selected, &electrons_selected);
    auto const& dileptons = dileptonHandler.getProducts();
    unsigned int ndileptons_SS = 0;
    unsigned int ndileptons_OS = 0;
    for (auto const& dilepton:dileptons){
      if (dilepton->isOS()) ndileptons_OS++;
      else ndileptons_SS++;
    }
    // If uncommented, these numbers could later tell you the average number of OS and SS dileptons before the rest of the selections
    // once you divide the numbers by the previous gen. weight sum.
    //seltracker.accumulate("nOS", wgt*static_cast<double>(ndileptons_OS));
    //seltracker.accumulate("nSS", wgt*static_cast<double>(ndileptons_SS));

    bool fail_vetos = false;
    DileptonObject* dilepton_SS_tight = nullptr;
    DileptonObject* dilepton_OS_DYCand_tight = nullptr;
    for (auto const& dilepton:dileptons){
      bool isSS = !dilepton->isOS();
      bool isTight = dilepton->nTightDaughters()==2;
      bool isSF = dilepton->isSF();
      bool is_LowMass = dilepton->m()<12.;
      bool is_ZClose = std::abs(dilepton->m()-91.2)<15.;
      bool is_DYClose = std::abs(dilepton->m()-91.2)<15. || is_LowMass;
      if (isSS && isSF && is_LowMass && std::abs(dilepton->getDaughter(0)->pdgId())==11){
        fail_vetos = true;
        break;
      }
      if (isSS && isTight && !dilepton_SS_tight) dilepton_SS_tight = dilepton;
      if (!isSS && isSF && is_DYClose){
        if (isTight && is_ZClose && !dilepton_OS_DYCand_tight) dilepton_OS_DYCand_tight = dilepton;
        else{
          fail_vetos = true;
          break;
        }
      }
    }

    if (applyPreselection && fail_vetos) continue;
    seltracker.accumulate("Pass dilepton vetos", wgt);

    if (applyPreselection && !dilepton_SS_tight) continue;
    seltracker.accumulate("Has at least one tight SS dilepton", wgt);

    // Put event filters to the last because data has unique event tracking enabled.
    eventFilter.constructFilters(&simEventHandler);
    //if (!eventFilter.test2018HEMFilter(&simEventHandler, nullptr, nullptr, &ak4jets)) continue; // Test for 2018 partial HEM failure
    //if (!eventFilter.test2018HEMFilter(&simEventHandler, &electrons, nullptr, nullptr)) continue; // Test for 2018 partial HEM failure
    seltracker.accumulate("Pass HEM veto", wgt);
    if (applyPreselection && !eventFilter.passMETFilters()) continue; // Test for MET filters
    seltracker.accumulate("Pass MET filters", wgt);
    if (!eventFilter.isUniqueDataEvent()) continue; // Test if the data event is unique (i.e., dorky). Does not do anything in the MC.
    seltracker.accumulate("Pass unique event check", wgt);

    // Later on, an overload of getTriggerWeight could be used to match trigger objects.
    // That would mean, however, that trigger bits need to be interpreted from NanoAOD.
    // Well, good luck with juggling between different years...
    float event_weight_triggers_dilepton = eventFilter.getTriggerWeight(hltnames_Dilepton);
    if (applyPreselection && event_weight_triggers_dilepton==0.) continue; // Test if any triggers passed at all
    seltracker.accumulate("Pass any trigger", wgt);

    // Accumulate any ME weights and K factors that might be present

    /*************************************************/
    /* NO MORE CALLS TO SELECTION BEYOND THIS POINT! */
    /*************************************************/
    constexpr int idx_CRW = 15;
    int iCRZ = (dilepton_OS_DYCand_tight ? 1 : 0);
    int icat = 0;
    if (nleptons_tight==2){
      switch (nak4jets_tight_pt25_btagged){
      case 2:
        if (nak4jets_tight_pt40<6) icat = idx_CRW;
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
      switch (nak4jets_tight_pt25_btagged){
      case 2:
        if (nak4jets_tight_pt40>=5) icat = 9 + std::min(nak4jets_tight_pt40, static_cast<unsigned int>(7))-5;
        break;
      default: // Nb>=3
        if (nak4jets_tight_pt40>=4) icat = 12 + std::min(nak4jets_tight_pt40, static_cast<unsigned int>(6))-4;
        break;
      }
    }
    if (icat>=0) hCat->Fill(static_cast<double>(icat-1)+0.5, static_cast<double>(iCRZ)+0.5, wgt);

    if (runSyncExercise) foutput_sync
      << *ptr_EventNumber << ","
      << ev << ","
      << eventmet->pt() << ","
      << eventmet->phi() << ","
      << nleptons_selected << ","
      << nleptons_fakeable + nleptons_tight << ","
      << nleptons_tight << ","
      << (dilepton_SS_tight ? "SS" : "!SS") << ","
      << ak4jets_pt40_HT << ","
      << nak4jets_tight_pt40 << ","
      << nak4jets_tight_pt25_btagged << ","
      << (iCRZ ? "Z" : (icat==idx_CRW ? "W" : (icat==0 ? "N/A" : std::to_string(icat).data())))
      << endl;

    n_recorded++;

    if (firstOutputEvent) firstOutputEvent = false;
  }

  IVYout << "Number of events recorded: " << n_recorded << " / " << n_traversed << " / " << nEntries << endl;
  seltracker.print();

  if (n_traversed>0) hCat->Scale(norm_scale * static_cast<double>(nEntries) / static_cast<double>(n_traversed));

  {
    double integral_error;
    integral_error = 0;
    IVYout << "Event counts for the SR:" << endl;
    for (int ix=1; ix<=hCat->GetNbinsX(); ix++) IVYout << "\t- " << hCat->GetXaxis()->GetBinLabel(ix) << ": " << hCat->GetBinContent(ix, 1) << " +- " << hCat->GetBinError(ix, 1) << endl;
    IVYout << "\t- Total: " << HelperFunctions::getHistogramIntegralAndError(hCat, 1, hCat->GetNbinsX()-1, 1, 1, false, &integral_error) << " +- " << integral_error << endl;
    IVYout << "\t- Failed: " << hCat->GetBinContent(0, 1) << " +- " << hCat->GetBinError(0, 1) << endl;
    integral_error = 0;
    IVYout << "Event counts for the CRZ:" << endl;
    for (int ix=1; ix<=hCat->GetNbinsX(); ix++) IVYout << "\t- " << hCat->GetXaxis()->GetBinLabel(ix) << ": " << hCat->GetBinContent(ix, 2) << " +- " << hCat->GetBinError(ix, 2) << endl;
    IVYout << "\t- Total: " << HelperFunctions::getHistogramIntegralAndError(hCat, 1, hCat->GetNbinsX()-1, 2, 2, false, &integral_error) << " +- " << integral_error << endl;
    IVYout << "\t- Failed: " << hCat->GetBinContent(0, 2) << " +- " << hCat->GetBinError(0, 2) << endl;
  }

  if (runSyncExercise){
    foutput_sync.close();
    SampleHelpers::splitFileAndAddForTransfer(stroutput_sync);
  }

  foutput->WriteTObject(hCat);
  delete hCat;
  foutput->Close();

  curdir->cd();
  delete tin;

  // Split large files, and add them to the transfer queue from Condor to the target site
  // Does nothing if you are running the program locally because your output is already in the desired location.
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
  SimpleEntry extra_arguments;
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
    else if (wish=="input_files"){
      if (value.find("/")!=std::string::npos){
        IVYerr << "ERROR: Input file specification cannot contain directory structure." << endl;
        print_help=true;
      }
      extra_arguments.setNamedVal(wish, value);
    }
    else if (wish=="run_sync"){
      bool tmpval;
      HelperFunctions::castStringToValue(value, tmpval);
      extra_arguments.setNamedVal(wish, tmpval);
    }
    else if (wish=="muon_id") extra_arguments.setNamedVal(wish, value);
    else if (wish=="electron_id") extra_arguments.setNamedVal(wish, value);
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
    IVYout << "- input_files: Input files to run. Optional. Default is to run on all files\n";
    IVYout << "- run_sync: Turn on synchronization output. Optional. Default is to run without synchronization output.\n";
    IVYout
      << "- muon_id: Can be 'Cutbased_Run2', 'TopMVA_Run2', or 'TopMVAv2_Run2'.\n"
      << "  Default is whatever is in MuonSelectionHelpers (currently 'Cutbased_Run2') if no value is given.\n";
    IVYout
      << "- electron_id: Can be 'Cutbased_Run2', 'TopMVA_Run2', or 'TopMVAv2_Run2'.\n"
      << "  Default is whatever is in ElectronSelectionHelpers (currently 'Cutbased_Run2') if no value is given.\n";

    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  SampleHelpers::configure(str_period, Form("skims:%s", str_tag.data()), HostHelpers::kUCSDT2);

  return ScanChain(str_outtag, str_dset, str_proc, xsec, extra_arguments);
}
