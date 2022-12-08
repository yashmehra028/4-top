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
#include "InputChunkSplitter.h"
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

int ScanChain(std::string const& strdate, std::string const& dset, std::string const& proc, double const& xsec, int const& ichunk, int const& nchunks, SimpleEntry const& extra_arguments){
  bool const isCondorRun = SampleHelpers::checkRunOnCondor();
  if (!isCondorRun) std::signal(SIGINT, SampleHelpers::setSignalInterrupt);

  TDirectory* curdir = gDirectory;

  // Configure analysis-specific stuff
  constexpr bool useFakeableIdForPhysicsChecks = true;
  ParticleSelectionHelpers::setUseFakeableIdForPhysicsChecks(useFakeableIdForPhysicsChecks);

  float const absEtaThr_ak4jets = (SampleHelpers::getDataYear()<=2016 ? AK4JetSelectionHelpers::etaThr_btag_Phase0Tracker : AK4JetSelectionHelpers::etaThr_btag_Phase1Tracker);

  double const lumi = SampleHelpers::getIntegratedLuminosity(SampleHelpers::getDataPeriod());
  IVYout << "Valid data periods for " << SampleHelpers::getDataPeriod() << ": " << SampleHelpers::getValidDataPeriods() << endl;
  IVYout << "Integrated luminosity: " << lumi << endl;

  // This is the output directory.
  // Output should always be recorded as if you are running the job locally.
  // We will inform the Condor job later on that some files would need transfer if we are running on Condor.
  TString coutput_main = TString("output/Analysis_FakeRates/") + strdate.data() + "/" + SampleHelpers::getDataPeriod();
  if (!isCondorRun) coutput_main = ANALYSISPKGPATH + "/test/" + coutput_main;
  HostHelpers::ExpandEnvironmentVariables(coutput_main);
  gSystem->mkdir(coutput_main, true);

  std::vector<std::pair<std::string, std::string>> dset_proc_pairs;
  {
    std::vector<std::string> dsets, procs;
    HelperFunctions::splitOptionRecursive(dset, dsets, ',', false);
    HelperFunctions::splitOptionRecursive(proc, procs, ',', false);
    HelperFunctions::zipVectors(dsets, procs, dset_proc_pairs);
  }
  bool const has_multiple_dsets = dset_proc_pairs.size()>1;

  // Configure output file name
  std::string output_file;
  extra_arguments.getNamedVal("output_file", output_file);
  if (output_file==""){
    if (has_multiple_dsets){
      IVYerr << "Multiple data sets are passed. An output file identifier must be specified through the option 'output_file'." << endl;
      assert(0);
    }
    output_file = proc;
  }
  if (nchunks>0) output_file = Form("%s_%i_of_%i", output_file.data(), ichunk, nchunks);

  // Configure full output file names with path
  TString stroutput = coutput_main + "/" + output_file.data() + ".root"; // This is the output ROOT file.
  TString stroutput_log = coutput_main + "/log_" + output_file.data() + ".out"; // This is the output log file.
  TString stroutput_err = coutput_main + "/log_" + output_file.data() + ".err"; // This is the error log file.
  IVYout.open(stroutput_log.Data());
  IVYerr.open(stroutput_err.Data());

  // Turn on individual files
  std::string input_files;
  extra_arguments.getNamedVal("input_files", input_files);

  // Shorthand option for the Run 2 UL analysis proposal
  bool use_shorthand_Run2_UL_proposal_config;
  extra_arguments.getNamedVal("shorthand_Run2_UL_proposal_config", use_shorthand_Run2_UL_proposal_config);

  // Options to set alternative muon and electron IDs, or b-tagging WP
  std::string muon_id_name;
  std::string electron_id_name;
  std::string btag_WPname;
  if (use_shorthand_Run2_UL_proposal_config){
    muon_id_name = electron_id_name = "TopMVA_Run2";
    btag_WPname = "loose";
  }
  else{
    extra_arguments.getNamedVal("muon_id", muon_id_name);
    extra_arguments.getNamedVal("electron_id", electron_id_name);
    extra_arguments.getNamedVal("btag", btag_WPname);
  }

  if (muon_id_name!=""){
    IVYout << "Switching to muon id " << muon_id_name << "..." << endl;
    MuonSelectionHelpers::setSelectionTypeByName(muon_id_name);
  }
  else IVYout << "Using default muon id = " << MuonSelectionHelpers::selection_type << "..." << endl;

  if (electron_id_name!=""){
    IVYout << "Switching to electron id " << electron_id_name << "..." << endl;
    ElectronSelectionHelpers::setSelectionTypeByName(electron_id_name);
  }
  else IVYout << "Using default electron id = " << ElectronSelectionHelpers::selection_type << "..." << endl;

  AK4JetSelectionHelpers::SelectionBits bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Medium;
  if (btag_WPname!=""){
    std::string btag_WPname_lower;
    HelperFunctions::lowercase(btag_WPname, btag_WPname_lower);
    IVYout << "Switching to b-tagging WP " << btag_WPname_lower << "..." << endl;
    if (btag_WPname_lower=="loose") bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Loose;
    else if (btag_WPname_lower=="medium") bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Medium;
    else if (btag_WPname_lower=="tight") bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Tight;
    else{
      IVYerr << "btag=" << btag_WPname << " is not implemented." << endl;
      assert(0);
    }
  }
  else IVYout << "Using default b-tagging WP = " << static_cast<int>(bit_preselection_btag)-static_cast<int>(AK4JetSelectionHelpers::kPreselectionTight_BTagged_Loose) << "..." << endl;

  // Trigger configuration
  std::vector<TriggerHelpers::TriggerType> requiredTriggers_Dilepton{
    TriggerHelpers::kMuEle
  };
  // These PFHT triggers were used in the 2016 analysis. They are a bit more efficient.
  if (SampleHelpers::getDataYear()==2016){
    requiredTriggers_Dilepton = std::vector<TriggerHelpers::TriggerType>{
      TriggerHelpers::kMuEle_PFHT
    };
    // Related to triggers is how we apply loose and fakeable IDs in electrons.
    // This setting should vary for 2016 when analyzing fake rates instead of the signal or SR-like control regions.
    // If trigger choices change, this setting may not be relevant either.
    if (ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kCutbased_Run2) ElectronSelectionHelpers::setApplyMVALooseFakeableNoIsoWPs(true);
  }
  std::vector<std::string> const hltnames_Dilepton = TriggerHelpers::getHLTMenus(requiredTriggers_Dilepton);
  auto triggerPropsCheckList_Dilepton = TriggerHelpers::getHLTMenuProperties(requiredTriggers_Dilepton);

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
  eventFilter.setCheckUniqueDataEvent(has_multiple_dsets);
  eventFilter.setCheckHLTPathRunRanges(true);

  curdir->cd();

  // We conclude the setup of event processing specifications and move on to I/O configuration next.

  // Open the output ROOT file
  SimpleEntry rcd_output;
  TFile* foutput = TFile::Open(stroutput, "recreate");
  foutput->cd();
  BaseTree* tout = new BaseTree("Events");
  tout->setAutoSave(0);
  curdir->cd();

  // Acquire input tree/chains
  TString strinputdpdir = SampleHelpers::getDataPeriod();
  if (SampleHelpers::testDataPeriodIsLikeData(SampleHelpers::getDataPeriod())){
    auto const& dy = SampleHelpers::getDataYear();
    if (dy==2016){
      if (SampleHelpers::isAPV2016Affected(SampleHelpers::getDataPeriod())) strinputdpdir = Form("%i_APV", dy);
      else strinputdpdir = Form("%i_NonAPV", dy);
    }
    else strinputdpdir = Form("%i", dy);
  }

  signed char is_sim_data_flag = -1; // =0 for sim, =1 for data
  int nevents_total = 0;
  std::vector<BaseTree*> tinlist; tinlist.reserve(dset_proc_pairs.size());
  std::unordered_map<BaseTree*, double> tin_normScale_map;
  for (auto const& dset_proc_pair:dset_proc_pairs){
    TString strinput = SampleHelpers::getInputDirectory() + "/" + strinputdpdir + "/" + dset_proc_pair.second.data();
    TString cinput = (input_files=="" ? strinput + "/*.root" : strinput + "/" + input_files.data());
    IVYout << "Accessing input files " << cinput << "..." << endl;
    BaseTree* tin = new BaseTree(cinput, "Events", "", "");
    tin->sampleIdentifier = SampleHelpers::getSampleIdentifier(dset_proc_pair.first);
    bool const isData = SampleHelpers::checkSampleIsData(tin->sampleIdentifier);
    if (!isData){
      if (xsec<0.){
        IVYerr << "xsec = " << xsec << " is not valid." << endl;
        assert(0);
      }
      if (has_multiple_dsets){
        IVYerr << "Should not process multiple data sets in sim. mode. Aborting..." << endl;
        assert(0);
      }
    }
    if (is_sim_data_flag==-1) is_sim_data_flag = (isData ? 1 : 0);
    else if (is_sim_data_flag != (isData ? 1 : 0)){
      IVYerr << "Should not process data and simulation at the same time." << endl;
      assert(0);
    }

    double sum_wgts = (isData ? 1 : 0);
    if (!isData){
      for (auto const& fname:SampleHelpers::lsdir(strinput.Data())){
        if (input_files!="" && fname!=input_files.data()) continue;
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

    // Add the tree to the list of trees to process
    tinlist.push_back(tin);

    // Calculate the overall normalization scale on the events.
    // Includes xsec (in fb), lumi (in fb-1), and 1/sum of weights in all of the MC.
    // Data normalizaion factor is always 1.
    double norm_scale = (isData ? 1. : xsec * xsecScale * lumi)/sum_wgts;
    tin_normScale_map[tin] = norm_scale;
    IVYout << "Acquired a sum of weights of " << sum_wgts << ". Overall normalization will be " << norm_scale << "." << endl;

    nevents_total += tin->getNEvents();

    curdir->cd();

    // Book the necessary branches
    genInfoHandler.bookBranches(tin);
    simEventHandler.bookBranches(tin);
    eventFilter.bookBranches(tin);
    muonHandler.bookBranches(tin);
    electronHandler.bookBranches(tin);
    jetHandler.bookBranches(tin);
    isotrackHandler.bookBranches(tin);

    // Book a few additional branches
    tin->bookBranch<EventNumber_t>("event", 0);
    if (isData){
      tin->bookBranch<RunNumber_t>("run", 0);
      tin->bookBranch<LuminosityBlock_t>("luminosityBlock", 0);
    }
  }

  curdir->cd();

  // Prepare to loop!
  // Keep track of sums of weights
  SelectionTracker seltracker;

  // Keep track of the traversed events
  bool firstOutputEvent = true;
  int eventIndex_begin = -1;
  int eventIndex_end = -1;
  int eventIndex_tracker = 0;
  splitInputEventsIntoChunks((is_sim_data_flag==1), nevents_total, ichunk, nchunks, eventIndex_begin, eventIndex_end);

  for (auto const& tin:tinlist){
    if (SampleHelpers::doSignalInterrupt==1) break;

    auto const& norm_scale = tin_normScale_map.find(tin)->second;
    bool const isData = (is_sim_data_flag==1);

    // Wrap the ivies around the input tree:
    // Booking is basically SetBranchStatus+SetBranchAddress. You can book for as many trees as you would like.
    // In some cases, bookBranches also informs the ivy dynamically that it is supposed to consume certain entries.
    // For entries common to all years or any data or MC, the consumption information is handled in the ivy constructor already.
    // None of these mean the ivy establishes its access to the input tree yet.
    // Wrapping a tree informs the ivy that it is supposed to consume the booked entries from that particular tree.
    // Without wrapping, you are not really accessing the entries from the input tree to construct the physics objects;
    // all you would get are 0 electrons, 0 jets, everything failing event filters etc.

    genInfoHandler.wrapTree(tin);
    simEventHandler.wrapTree(tin);
    eventFilter.wrapTree(tin);
    muonHandler.wrapTree(tin);
    electronHandler.wrapTree(tin);
    jetHandler.wrapTree(tin);
    isotrackHandler.wrapTree(tin);

    RunNumber_t* ptr_RunNumber = nullptr;
    LuminosityBlock_t* ptr_LuminosityBlock = nullptr;
    EventNumber_t* ptr_EventNumber = nullptr;
    tin->getValRef("event", ptr_EventNumber);
    if (isData){
      tin->getValRef("run", ptr_RunNumber);
      tin->getValRef("luminosityBlock", ptr_LuminosityBlock);
    }

    unsigned int n_traversed = 0;
    unsigned int n_recorded = 0;
    int nEntries = tin->getNEvents();
    IVYout << "Looping over " << nEntries << " events from " << tin->sampleIdentifier << "..." << endl;
    for (int ev=0; ev<nEntries; ev++){
      if (SampleHelpers::doSignalInterrupt==1) break;

      bool doAccumulate = true;
      if (isData){
        if (eventIndex_begin>0 || eventIndex_end>0) doAccumulate = (
          tin->updateBranch(ev, "run", false)
          &&
          (eventIndex_begin<0 || (*ptr_RunNumber)>=static_cast<RunNumber_t>(eventIndex_begin))
          &&
          (eventIndex_end<0 || (*ptr_RunNumber)<=static_cast<RunNumber_t>(eventIndex_end))
          );
      }
      else doAccumulate = (
        (eventIndex_begin<0 || eventIndex_tracker>=static_cast<int>(eventIndex_begin))
        &&
        (eventIndex_end<0 || eventIndex_tracker<static_cast<int>(eventIndex_end))
        );

      eventIndex_tracker++;
      if (!doAccumulate) continue;

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
        wgt *= simEventHandler.getL1PrefiringWeight(SystematicsHelpers::sNominal);
      }
      wgt *= norm_scale;

      muonHandler.constructMuons();
      electronHandler.constructElectrons();
      jetHandler.constructJetMET(&simEventHandler);

      particleDisambiguator.disambiguateParticles(&muonHandler, &electronHandler, nullptr, &jetHandler);

      std::vector<ParticleObject*> leptons_tight;

      auto const& muons = muonHandler.getProducts();
      std::vector<MuonObject*> muons_selected;
      std::vector<MuonObject*> muons_tight;
      std::vector<MuonObject*> muons_fakeable;
      std::vector<MuonObject*> muons_loose;
      for (auto const& part:muons){
        float pt = part->pt();
        float eta = part->eta();
        float phi = part->phi();
        float mass = part->mass();

        bool is_tight = false;
        bool is_fakeable = false;
        bool is_loose = false;

        if (ParticleSelectionHelpers::isTightParticle(part)){
          muons_tight.push_back(part);
          leptons_tight.push_back(dynamic_cast<ParticleObject*>(part));
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

        if (!is_loose) continue;

        float ptrel_final = part->ptrel();
        float ptratio_final = part->ptratio();

        float extMVAscore=-99;
        bool has_extMVAscore = part->getExternalMVAScore(MuonSelectionHelpers::selection_type, extMVAscore);

        float bscore = 0;
        AK4JetObject* mother = nullptr;
        for (auto const& mom:part->getMothers()){
          mother = dynamic_cast<AK4JetObject*>(mom);
          if (mother) break;
        }
        if (mother) bscore = mother->extras.btagDeepFlavB;

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
        float pt = part->pt();
        float eta = part->eta();
        float etaSC = part->etaSC();
        float phi = part->phi();
        float mass = part->mass();

        bool is_tight = false;
        bool is_fakeable = false;
        bool is_loose = false;

        if (ParticleSelectionHelpers::isTightParticle(part)){
          electrons_tight.push_back(part);
          leptons_tight.push_back(dynamic_cast<ParticleObject*>(part));
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

        if (!is_loose) continue;

        float mvaFall17V2noIso_raw = 0.5 * std::log((1. + part->extras.mvaFall17V2noIso)/(1. - part->extras.mvaFall17V2noIso));
        float ptrel_final = part->ptrel();
        float ptratio_final = part->ptratio();

        float extMVAscore=-99;
        bool has_extMVAscore = part->getExternalMVAScore(ElectronSelectionHelpers::selection_type, extMVAscore);

        float bscore = 0;
        AK4JetObject* mother = nullptr;
        for (auto const& mom:part->getMothers()){
          mother = dynamic_cast<AK4JetObject*>(mom);
          if (mother) break;
        }
        if (mother) bscore = mother->extras.btagDeepFlavB;

      }
      HelperFunctions::appendVector(electrons_selected, electrons_tight);
      HelperFunctions::appendVector(electrons_selected, electrons_fakeable);
      HelperFunctions::appendVector(electrons_selected, electrons_loose);

      unsigned int const nleptons_tight = muons_tight.size() + electrons_tight.size();
      unsigned int const nleptons_fakeable = muons_fakeable.size() + electrons_fakeable.size();
      unsigned int const nleptons_loose = muons_loose.size() + electrons_loose.size();
      unsigned int const nleptons_selected = nleptons_tight + nleptons_fakeable + nleptons_loose;

      ParticleObjectHelpers::sortByGreaterPt(leptons_tight);

      auto const& ak4jets = jetHandler.getAK4Jets();
      std::vector<AK4JetObject*> ak4jets_tight;
      unsigned int nbjets_tight = 0;
      for (auto const& jet:ak4jets){
        float pt = jet->pt();
        float eta = jet->eta();
        float phi = jet->phi();
        float mass = jet->mass();

        bool is_tight = ParticleSelectionHelpers::isTightJet(jet);
        bool is_btagged = jet->testSelectionBit(bit_preselection_btag);
        constexpr bool is_clean = true;
        if (is_tight && pt>=25. && std::abs(eta)<absEtaThr_ak4jets){
          ak4jets_tight.push_back(jet);
          if (is_btagged) nbjets_tight++;
        }
      }

      // MET info
      auto const& eventmet = jetHandler.getPFMET();

      // BEGIN PRESELECTION
      seltracker.accumulate("Full sample", wgt);

      if (nleptons_fakeable>0 || nleptons_loose>0) continue; 
      seltracker.accumulate("Has exactly 0 loose and 0 fakeable leptons", wgt);

      if (electrons_tight.size()!=1) continue;
      seltracker.accumulate("Has exactly one tight electron", wgt);

      if (muons_tight.size()!=1) continue;
      seltracker.accumulate("Has exactly one tight muon", wgt);

      // check oppososite charge
      if (electrons_tight.front()->pdgId() * muons_tight.front()->pdgId() > 0 ) continue;
      seltracker.accumulate("Has opposite charge", wgt);

      // check lep1 pt>25 lep2 pt>20
      if (leptons_tight.front()->pt() < 25. || leptons_tight.back()->pt() < 20.) continue;
      seltracker.accumulate("Has lep1 pt>25 lep2 pt>20", wgt);

      // check MET (jet1 25, jet2 20)
      if (eventmet->pt() < 25.f) continue;
      seltracker.accumulate("MET > 25 GeV", wgt);

      // check njet>2 and nbjets_tight>2
      if (ak4jets_tight.size() < 2) continue;
      seltracker.accumulate("njet > 2", wgt);

      if (nbjets_tight < 2) continue;
      seltracker.accumulate("nbjet > 2", wgt);

      // Put event filters to the last because data has unique event tracking enabled.
      eventFilter.constructFilters(&simEventHandler);
      //if (!eventFilter.test2018HEMFilter(&simEventHandler, nullptr, nullptr, &ak4jets)) continue; // Test for 2018 partial HEM failure
      //if (!eventFilter.test2018HEMFilter(&simEventHandler, &electrons, nullptr, nullptr)) continue; // Test for 2018 partial HEM failure
      //seltracker.accumulate("Pass HEM veto", wgt);
      if (!eventFilter.passMETFilters()) continue; // Test for MET filters
      seltracker.accumulate("Pass MET filters", wgt);
      if (!eventFilter.isUniqueDataEvent()) continue; // Test if the data event is unique (i.e., dorky). Does not do anything in the MC.
      seltracker.accumulate("Pass unique event check", wgt);

      // Triggers
      double event_wgt_triggers_dilepton = eventFilter.getTriggerWeight(hltnames_Dilepton);
      if (event_wgt_triggers_dilepton==0.) continue; // Test if any triggers passed at all
      seltracker.accumulate("Pass any trigger", wgt);
      rcd_output.setNamedVal("event_wgt_triggers_dilepton", static_cast<float>(event_wgt_triggers_dilepton));

      double event_wgt_triggers_dilepton_matched = eventFilter.getTriggerWeight(
        triggerPropsCheckList_Dilepton,
        &muons, &electrons, nullptr, &ak4jets, nullptr, nullptr
      );
      seltracker.accumulate("Pass triggers after matching", (event_wgt_triggers_dilepton_matched>0.)*wgt);
      rcd_output.setNamedVal("event_wgt_triggers_dilepton_matched", static_cast<float>(event_wgt_triggers_dilepton_matched));
      
      /*************************************************/
      /* NO MORE CALLS TO SELECTION BEYOND THIS POINT! */
      /*************************************************/
      // calculate min_mlb, Ht, min_mbb, max_mbb

      // min_mlb
      // loop over tight leptons:
      float min_mlb = -1;
      for (auto const& lep:leptons_tight){
        // loop over jets:
        for (auto const& jet:ak4jets_tight){
          if (!jet->testSelectionBit(bit_preselection_btag)) continue;
          float mlb = (lep->p4() + jet->p4()).M();
          if (mlb<min_mlb || min_mlb<0.f) min_mlb = mlb;
        }
      }

      // Ht
      float Ht = 0;
      for (auto const& jet:ak4jets_tight){
        Ht += jet->pt();
      }

      // min_mbb and max_mbb
      float min_mbb = -1;
      float max_mbb = -1;
      for (auto jet1 = ak4jets_tight.begin(); jet1 != ak4jets_tight.end(); ++jet1){
        for (auto jet2 = jet1+1; jet2 != ak4jets_tight.end(); ++jet2){
          float mbb = ((*jet1)->p4() + (*jet2)->p4()).M();
          if (mbb<min_mbb || min_mbb<0.f) min_mbb = mbb;
          if (mbb>max_mbb || max_mbb<0.f) max_mbb = mbb;
        }
      }

      // Write output
      rcd_output.setNamedVal("njet", static_cast<unsigned int>(ak4jets_tight.size()));
      rcd_output.setNamedVal("nbjet", nbjets_tight);
      rcd_output.setNamedVal("event_wgt", static_cast<float>(wgt));
      rcd_output.setNamedVal("min_mlb", min_mlb);
      rcd_output.setNamedVal("min_mbb", min_mbb);
      rcd_output.setNamedVal("max_mbb", max_mbb);
      rcd_output.setNamedVal("Ht", Ht); 

      // make vectors of pt, eta, phi, mass, pdgId for leptons
      std::vector<float> lep_pt;
      std::vector<float> lep_eta;
      std::vector<float> lep_phi;
      std::vector<float> lep_mass;
      std::vector<int> lep_pdgId;
      for (auto const& lep:leptons_tight){
        lep_pt.push_back(lep->pt());
        lep_eta.push_back(lep->eta());
        lep_phi.push_back(lep->phi());
        lep_mass.push_back(lep->mass());
        lep_pdgId.push_back(lep->pdgId());
      }

      // setNamedVal for pt, eta, phi, mass, pdgId for leptons
      rcd_output.setNamedVal("lep_pt", lep_pt);
      rcd_output.setNamedVal("lep_eta", lep_eta);
      rcd_output.setNamedVal("lep_phi", lep_phi);
      rcd_output.setNamedVal("lep_mass", lep_mass);
      rcd_output.setNamedVal("lep_pdgId", lep_pdgId);

      // make vectors of pt eta phi, mass, is_btagged, partonFlavour, and hadronFlavour for jets
      std::vector<float> jet_pt;
      std::vector<float> jet_eta;
      std::vector<float> jet_phi;
      std::vector<float> jet_mass;
      std::vector<bool> jet_is_btagged;
      std::vector<int> jet_partonFlavour;
      std::vector<int> jet_hadronFlavour;
      for (auto const& jet:ak4jets_tight){
        jet_pt.push_back(jet->pt());
        jet_eta.push_back(jet->eta());
        jet_phi.push_back(jet->phi());
        jet_mass.push_back(jet->mass());
        jet_is_btagged.push_back(jet->testSelectionBit(bit_preselection_btag));
        jet_partonFlavour.push_back(jet->extras.partonFlavour);
        jet_hadronFlavour.push_back(jet->extras.hadronFlavour);
      }

      // setNamedVal for pt, eta, phi, mass, is_btagged, genLevelInfo for jets
      rcd_output.setNamedVal("jet_pt", jet_pt);
      rcd_output.setNamedVal("jet_eta", jet_eta);
      rcd_output.setNamedVal("jet_phi", jet_phi);
      rcd_output.setNamedVal("jet_mass", jet_mass);
      rcd_output.setNamedVal("jet_is_btagged", jet_is_btagged);
      rcd_output.setNamedVal("jet_partonFlavour", jet_partonFlavour);
      rcd_output.setNamedVal("jet_hadronFlavour", jet_hadronFlavour);

      rcd_output.setNamedVal("EventNumber", *ptr_EventNumber);
      if (isData){
        rcd_output.setNamedVal("RunNumber", *ptr_RunNumber);
        rcd_output.setNamedVal("LuminosityBlock", *ptr_LuminosityBlock);
      }
      rcd_output.setNamedVal("PFMET_pt_final", eventmet->pt());
      rcd_output.setNamedVal("PFMET_phi_final", eventmet->phi());

      if (firstOutputEvent){
#define SIMPLE_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.named##name_t##s.begin(); itb!=rcd_output.named##name_t##s.end(); itb++) tout->putBranch(itb->first, itb->second);
#define VECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedV##name_t##s.begin(); itb!=rcd_output.namedV##name_t##s.end(); itb++) tout->putBranch(itb->first, &(itb->second));
#define DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedVV##name_t##s.begin(); itb!=rcd_output.namedVV##name_t##s.end(); itb++) tout->putBranch(itb->first, &(itb->second));
        SIMPLE_DATA_OUTPUT_DIRECTIVES;
        VECTOR_DATA_OUTPUT_DIRECTIVES;
        DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVES;
#undef SIMPLE_DATA_OUTPUT_DIRECTIVE
#undef VECTOR_DATA_OUTPUT_DIRECTIVE
#undef DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE
      }

      // Record whatever is in rcd_output into the tree.
#define SIMPLE_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.named##name_t##s.begin(); itb!=rcd_output.named##name_t##s.end(); itb++) tout->setVal(itb->first, itb->second);
#define VECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedV##name_t##s.begin(); itb!=rcd_output.namedV##name_t##s.end(); itb++) tout->setVal(itb->first, &(itb->second));
#define DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedVV##name_t##s.begin(); itb!=rcd_output.namedVV##name_t##s.end(); itb++) tout->setVal(itb->first, &(itb->second));
      SIMPLE_DATA_OUTPUT_DIRECTIVES;
      VECTOR_DATA_OUTPUT_DIRECTIVES;
      DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVES;
#undef SIMPLE_DATA_OUTPUT_DIRECTIVE
#undef VECTOR_DATA_OUTPUT_DIRECTIVE
#undef DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE

      tout->fill();
      n_recorded++;

      if (firstOutputEvent) firstOutputEvent = false;
    }

    IVYout << "Number of events recorded: " << n_recorded << " / " << n_traversed << " / " << nEntries << endl;
  }
  seltracker.print();

  tout->writeToFile(foutput);
  delete tout;
  foutput->Close();

  curdir->cd();
  for (auto& tin:tinlist) delete tin;;

  // Split large files, and add them to the transfer queue from Condor to the target site
  // Does nothing if you are running the program locally because your output is already in the desired location.
  SampleHelpers::splitFileAndAddForTransfer(stroutput);

  // Close the output and error log files
  IVYout.close();
  IVYerr.close();

  return 0;
}

int main(int argc, char** argv){
  constexpr int iarg_offset=1; // argv[0]==[Executable name]

  bool print_help=false, has_help=false;
  int ichunk=0, nchunks=0;
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
      else if (value=="shorthand_Run2_UL_proposal_config") extra_arguments.setNamedVal<bool>(value, true);
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
    else if (wish=="muon_id" || wish=="electron_id" || wish=="btag" || wish=="output_file") extra_arguments.setNamedVal(wish, value);
    else if (wish=="xsec"){
      if (xsec<0.) xsec = 1;
      xsec *= std::stod(value);
    }
    else if (wish=="BR"){
      if (xsec<0.) xsec = 1;
      xsec *= std::stod(value);
    }
    else if (wish=="nchunks") nchunks = std::stoi(value);
    else if (wish=="ichunk") ichunk = std::stoi(value);
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
    IVYout << argv[0] << " options:\n\n";
    IVYout << "- help: Prints this help message.\n";
    IVYout << "- dataset: Data set name. Mandatory.\n";
    IVYout << "- short_name: Process short name. Mandatory.\n";
    IVYout << "- period: Data period. Mandatory.\n";
    IVYout << "- input_tag: Version of the input skims. Mandatory.\n";
    IVYout << "- output_tag: Version of the output. Mandatory.\n";
    IVYout << "- xsec: Cross section value. Mandatory in the MC.\n";
    IVYout << "- BR: BR value. Mandatory in the MC.\n";
    IVYout << "- nchunks: Number of splits of the input. Optional. Input splitting is activated only if nchunks>0.\n";
    IVYout << "- ichunk: Index of split input split to run. Optional. The index should be in the range [0, nchunks-1].\n";
    IVYout
      << "- output_file: Identifier of the output file if different from 'short_name'. Optional.\n"
      << "  The full output file name is '[identifier](_[ichunk]_[nchunks]).root.'\n";
    IVYout << "- input_files: Input files to run. Optional. Default is to run on all files.\n";
    IVYout
      << "- shorthand_Run2_UL_proposal_config: Shorthand flag for the switches for the Run 2 UL analysis proposal:\n"
      << "  * muon_id='TopMVA_Run2'\n"
      << "  * electron_id='TopMVA_Run2'\n"
      << "  * btag='loose'\n"
      << "  The use of this shorthand will ignore the user-defined setting of these options above.\n";
    IVYout
      << "- muon_id: Can be 'Cutbased_Run2', 'TopMVA_Run2', or 'TopMVAv2_Run2'.\n"
      << "  Default is whatever is in MuonSelectionHelpers (currently 'Cutbased_Run2') if no value is given.\n";
    IVYout
      << "- electron_id: Can be 'Cutbased_Run2', 'TopMVA_Run2', or 'TopMVAv2_Run2'.\n"
      << "  Default is whatever is in ElectronSelectionHelpers (currently 'Cutbased_Run2') if no value is given.\n";
    IVYout << "- btag: Name of the b-tagging WP. Can be 'medium' or 'loose' (case-insensitive). Default='medium'.\n";

    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  SampleHelpers::configure(str_period, Form("skims:%s", str_tag.data()), HostHelpers::kUCSDT2);

  return ScanChain(str_outtag, str_dset, str_proc, xsec, ichunk, nchunks, extra_arguments);
}
