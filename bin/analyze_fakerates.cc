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

  // This is the output directory.
  // Output should always be recorded as if you are running the job locally.
  // We will inform the Condor job later on that some files would need transfer if we are running on Condor.
  TString coutput_main = ANALYSISPKGPATH + "test/output/Analysis_FakeRates/" + strdate.data() + "/" + SampleHelpers::getDataPeriod();
  HostHelpers::ExpandEnvironmentVariables(coutput_main);
  gSystem->mkdir(coutput_main, true);
  TString stroutput = coutput_main + "/" + proc.data() + ".root"; // This is the output file.
  TString stroutput_log = coutput_main + "/log_" + proc.data() + ".out"; // This is the output file.
  IVYout.open(stroutput_log.Data());

  constexpr bool useFakeableIdForPhysicsChecks = true;
  ParticleSelectionHelpers::setUseFakeableIdForPhysicsChecks(useFakeableIdForPhysicsChecks);

  float const absEtaThr_ak4jets = (SampleHelpers::getDataYear()<=2016 ? AK4JetSelectionHelpers::etaThr_btag_Phase0Tracker : AK4JetSelectionHelpers::etaThr_btag_Phase1Tracker);

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
  std::vector<TriggerHelpers::TriggerType> requiredTriggers_SingleLepton{
    TriggerHelpers::kSingleMu_Control_Iso,
    TriggerHelpers::kSingleEle_Control_Iso
  };
  if (SampleHelpers::getDataYear()==2016){
    requiredTriggers_SingleLepton = std::vector<TriggerHelpers::TriggerType>{
      TriggerHelpers::kSingleMu_Control_NoIso,
      TriggerHelpers::kSingleEle_Control_NoIso
    };
    // Related to triggers is how we apply loose and fakeable IDs in electrons.
    // This setting should vary for 2016 when analyzing fake rates instead of the signal or SR-like control regions.
    // If trigger choices change, this setting may not be relevant either.
    if (ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kCutbased_Run2) ElectronSelectionHelpers::setApplyMVALooseFakeableNoIsoWPs(true);
  }
  std::vector<std::string> const hltnames_SingleLepton = TriggerHelpers::getHLTMenus(requiredTriggers_SingleLepton);
  auto triggerPropsCheckList_SingleLepton = TriggerHelpers::getHLTMenuProperties(requiredTriggers_SingleLepton);

  // Declare handlers
  GenInfoHandler genInfoHandler;
  SimEventHandler simEventHandler;
  EventFilterHandler eventFilter(requiredTriggers_SingleLepton);
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

  RunNumber_t* ptr_RunNumber = nullptr;
  LuminosityBlock_t* ptr_LuminosityBlock = nullptr;
  EventNumber_t* ptr_EventNumber = nullptr;
  tin->bookBranch<EventNumber_t>("event", 0);
  tin->getValRef("event", ptr_EventNumber);
  if (isData){
    tin->bookBranch<RunNumber_t>("run", 0);
    tin->getValRef("run", ptr_RunNumber);
    tin->bookBranch<LuminosityBlock_t>("luminosityBlock", 0);
    tin->getValRef("luminosityBlock", ptr_LuminosityBlock);
  }

  curdir->cd();

  SimpleEntry rcd_output;
  TFile* foutput = TFile::Open(stroutput, "recreate");
  BaseTree* tout = new BaseTree("Events");
  tout->setAutoSave(0);
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

      // Add L1 prefiring weight for 2016 and 2017
      wgt *= simEventHandler.getL1PrefiringWeight(SystematicsHelpers::sNominal);
    }
    wgt *= norm_scale;

    muonHandler.constructMuons();
    electronHandler.constructElectrons();
    jetHandler.constructJetMET(&simEventHandler);

    particleDisambiguator.disambiguateParticles(&muonHandler, &electronHandler, nullptr, &jetHandler);

    // Muon sync. write variables
#define SYNC_MUONS_BRANCH_VECTOR_COMMANDS \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, muons, is_loose) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, muons, is_fakeable) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, muons, is_tight) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, pt) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, eta) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, phi) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, mass) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, ptrel_final) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, ptratio_final) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, bscore) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, muons, extMVAscore) \
    MUON_EXTRA_VARIABLES
    // Electron sync. write variables
#define SYNC_ELECTRONS_BRANCH_VECTOR_COMMANDS \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, electrons, is_loose) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, electrons, is_fakeable) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, electrons, is_tight) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, pt) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, eta) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, etaSC) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, phi) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, mass) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, ptrel_final) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, ptratio_final) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, mvaFall17V2noIso_raw) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, bscore) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, electrons, extMVAscore) \
    ELECTRON_EXTRA_VARIABLES
    // ak4jet sync. write variables
#define SYNC_AK4JETS_BRANCH_VECTOR_COMMANDS \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, ak4jets, is_tight) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, ak4jets, is_btagged) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(bool, ak4jets, is_clean) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, ak4jets, pt) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, ak4jets, eta) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, ak4jets, phi) \
    SYNC_OBJ_BRANCH_VECTOR_COMMAND(float, ak4jets, mass) \
    AK4JET_EXTRA_INPUT_VARIABLES
    // All sync. write objects
#define SYNC_ALLOBJS_BRANCH_VECTOR_COMMANDS \
    SYNC_MUONS_BRANCH_VECTOR_COMMANDS \
    SYNC_ELECTRONS_BRANCH_VECTOR_COMMANDS \
    SYNC_AK4JETS_BRANCH_VECTOR_COMMANDS
#define SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, COLLNAME, NAME) std::vector<TYPE> COLLNAME##_##NAME;
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, muons, NAME)
#define ELECTRON_VARIABLE(TYPE, NAME, DEFVAL) SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, electrons, NAME)
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, ak4jets, NAME)
    SYNC_ALLOBJS_BRANCH_VECTOR_COMMANDS;
#undef AK4JET_VARIABLE
#undef ELECTRON_VARIABLE
#undef MUON_VARIABLE
#undef SYNC_OBJ_BRANCH_VECTOR_COMMAND

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

#define SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, COLLNAME, NAME) COLLNAME##_##NAME.push_back(NAME);
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) muons_##NAME.push_back(part->extras.NAME);
      SYNC_MUONS_BRANCH_VECTOR_COMMANDS;
#undef MUON_VARIABLE
#undef SYNC_OBJ_BRANCH_VECTOR_COMMAND
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

#define SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, COLLNAME, NAME) COLLNAME##_##NAME.push_back(NAME);
#define ELECTRON_VARIABLE(TYPE, NAME, DEFVAL) electrons_##NAME.push_back(part->extras.NAME);
      SYNC_ELECTRONS_BRANCH_VECTOR_COMMANDS;
#undef ELECTRON_VARIABLE
#undef SYNC_OBJ_BRANCH_VECTOR_COMMAND
    }
    HelperFunctions::appendVector(electrons_selected, electrons_tight);
    HelperFunctions::appendVector(electrons_selected, electrons_fakeable);
    HelperFunctions::appendVector(electrons_selected, electrons_loose);

    unsigned int const nleptons_tight = muons_tight.size() + electrons_tight.size();
    unsigned int const nleptons_fakeable = muons_fakeable.size() + electrons_fakeable.size();
    unsigned int const nleptons_loose = muons_loose.size() + electrons_loose.size();
    unsigned int const nleptons_selected = nleptons_tight + nleptons_fakeable + nleptons_loose;

    auto const& ak4jets = jetHandler.getAK4Jets();
    unsigned int nak4jets_tight_pt25_etaCentral = 0;
    for (auto const& jet:ak4jets){
      float pt = jet->pt();
      float eta = jet->eta();
      float phi = jet->phi();
      float mass = jet->mass();

      bool is_tight = ParticleSelectionHelpers::isTightJet(jet);
      bool is_btagged = jet->testSelectionBit(bit_preselection_btag);
      constexpr bool is_clean = true;

#define SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, COLLNAME, NAME) COLLNAME##_##NAME.push_back(NAME);
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) ak4jets_##NAME.push_back(jet->extras.NAME);
      SYNC_AK4JETS_BRANCH_VECTOR_COMMANDS;
#undef AK4JET_VARIABLE
#undef SYNC_OBJ_BRANCH_VECTOR_COMMAND

      if (is_tight && pt>=25. && std::abs(eta)<absEtaThr_ak4jets) nak4jets_tight_pt25_etaCentral++;
    }

    // MET info
    auto const& eventmet = jetHandler.getPFMET();


    // BEGIN PRESELECTION
    seltracker.accumulate("Full sample", wgt);

    if ((nleptons_tight + nleptons_fakeable)!=1) continue;
    seltracker.accumulate("Has exactly one fakeable lepton", wgt);

    if (nak4jets_tight_pt25_etaCentral==0) continue;
    seltracker.accumulate("Has at least one tight jet with pT>25 GeV and central eta", wgt);

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
    bool pass_any_trigger = false;
    bool pass_any_trigger_TOmatched = false;
    for (auto const& hlt_type_prop_pair:triggerPropsCheckList_SingleLepton){
      std::string const& hltname = hlt_type_prop_pair.second->getName();
      std::vector< std::pair<TriggerHelpers::TriggerType, HLTTriggerPathProperties const*> > dummy_type_prop_vector{ hlt_type_prop_pair };
      float event_weight_trigger = eventFilter.getTriggerWeight(std::vector<std::string>{hltname});
      float event_weight_trigger_TOmatched = eventFilter.getTriggerWeight(
        dummy_type_prop_vector,
        &muons, &electrons, nullptr, &ak4jets, nullptr, nullptr
      );
      pass_any_trigger |= (event_weight_trigger>0.);
      pass_any_trigger_TOmatched |= (event_weight_trigger_TOmatched>0.);

      std::string hltname_pruned = hltname;
      {
        auto ipos = hltname_pruned.find_last_of("_v");
        if (ipos!=std::string::npos) hltname_pruned = hltname_pruned.substr(0, ipos-1);
      }
      rcd_output.setNamedVal(Form("event_wgt_trigger_%s", hltname_pruned.data()), pass_any_trigger);
      rcd_output.setNamedVal(Form("event_wgt_trigger_TOmatched_%s", hltname_pruned.data()), pass_any_trigger_TOmatched);
    }
    if (!pass_any_trigger) continue;
    seltracker.accumulate("Pass any trigger", wgt);
    seltracker.accumulate("Pass triggers after TO matching", static_cast<float>(pass_any_trigger_TOmatched)*wgt);

    /*************************************************/
    /* NO MORE CALLS TO SELECTION BEYOND THIS POINT! */
    /*************************************************/
    // Write output
    rcd_output.setNamedVal("event_wgt", wgt);

    rcd_output.setNamedVal("EventNumber", *ptr_EventNumber);
    if (isData){
      rcd_output.setNamedVal("RunNumber", *ptr_RunNumber);
      rcd_output.setNamedVal("LuminosityBlock", *ptr_LuminosityBlock);
    }
    rcd_output.setNamedVal("PFMET_pt_final", eventmet->pt());
    rcd_output.setNamedVal("PFMET_phi_final", eventmet->phi());

#define SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, COLLNAME, NAME) rcd_output.setNamedVal(Form("%s_%s", #COLLNAME, #NAME), COLLNAME##_##NAME);
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, muons, NAME)
#define ELECTRON_VARIABLE(TYPE, NAME, DEFVAL) SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, electrons, NAME)
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) SYNC_OBJ_BRANCH_VECTOR_COMMAND(TYPE, ak4jets, NAME)
    SYNC_ALLOBJS_BRANCH_VECTOR_COMMANDS;
#undef AK4JET_VARIABLE
#undef ELECTRON_VARIABLE
#undef MUON_VARIABLE
#undef SYNC_OBJ_BRANCH_VECTOR_COMMAND

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
  seltracker.print();

  tout->writeToFile(foutput);
  delete tout;
  foutput->Close();

  curdir->cd();

  delete tin;

  IVYout.close();

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
    else if (wish=="muon_id" || wish=="electron_id" || wish=="btag") extra_arguments.setNamedVal(wish, value);
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
    IVYout << argv[0] << " options:\n\n";
    IVYout << "- help: Prints this help message.\n";
    IVYout << "- dataset: Data set name. Mandatory.\n";
    IVYout << "- short_name: Process short name. Mandatory.\n";
    IVYout << "- period: Data period. Mandatory.\n";
    IVYout << "- input_tag: Version of the input skims. Mandatory.\n";
    IVYout << "- output_tag: Version of the output. Mandatory.\n";
    IVYout << "- xsec: Cross section value. Mandatory in the MC.\n";
    IVYout << "- BR: BR value. Mandatory in the MC.\n";
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

  return ScanChain(str_outtag, str_dset, str_proc, xsec, extra_arguments);
}
