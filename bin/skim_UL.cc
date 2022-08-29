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
#include "SamplesCore.h"
#include "MuonSelectionHelpers.h"
#include "ElectronSelectionHelpers.h"
#include "AK4JetSelectionHelpers.h"
#include "IsotrackSelectionHelpers.h"
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


using namespace std;
using namespace HelperFunctions;
using namespace IvyStreamHelpers;


constexpr bool global_include_veto_isotracks = false;


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


int ScanChain(std::vector<TString> const& inputfnames, std::string const& output_fname, std::string const& dset, std::string const& proc, std::string const& str_period, std::unordered_map<std::string, std::string> const& extra_arguments){
  if (!SampleHelpers::checkRunOnCondor()) std::signal(SIGINT, SampleHelpers::setSignalInterrupt);

  TDirectory* curdir = gDirectory;

  {
    std::size_t tmp_pos = output_fname.find_last_of('/');
    if (tmp_pos!=std::string::npos){
      std::string output_dir(output_fname, 0, tmp_pos);
      gSystem->mkdir(output_dir.data(), true);
    }
  }
  std::string strjsonfile;
  std::vector<std::string> kfactoropts;
  for (auto const& pp:extra_arguments){
    if (pp.first.find("applyKFactor")!=std::string::npos){
      bool flag = false;
      HelperFunctions::castStringToValue(pp.second, flag);
      if (flag) kfactoropts.push_back(pp.first);
    }
    else if (pp.first == "goldenjson") strjsonfile = pp.second;
  }

  std::vector<TriggerHelpers::TriggerType> requiredTriggers;
  std::vector<TriggerHelpers::TriggerType> const requiredTriggers_Dilepton{
    TriggerHelpers::kDoubleMu,
    TriggerHelpers::kDoubleEle,
    TriggerHelpers::kMuEle,
    // These PFHT triggers were used in the 2016 analysis. We keep them for now, but we could drop them later.
    TriggerHelpers::kDoubleMu_PFHT,
    TriggerHelpers::kDoubleEle_PFHT,
    TriggerHelpers::kMuEle_PFHT
  };
  std::vector<std::string> const hltnames_Dilepton = TriggerHelpers::getHLTMenus(requiredTriggers_Dilepton);
  HelperFunctions::appendVector(requiredTriggers, requiredTriggers_Dilepton);
  std::vector<TriggerHelpers::TriggerType> const requiredTriggers_SingleLeptonControl{
    TriggerHelpers::kSingleMu_Control_Iso,
    TriggerHelpers::kSingleEle_Control_Iso,
    // These nonisolated triggers were used in the 2016 analysis. We keep them for now, but we could drop them later.
    TriggerHelpers::kSingleMu_Control_NoIso,
    TriggerHelpers::kSingleEle_Control_NoIso
  };
  std::vector<std::string> const hltnames_SingleLeptonControl = TriggerHelpers::getHLTMenus(requiredTriggers_SingleLeptonControl);
  HelperFunctions::appendVector(requiredTriggers, requiredTriggers_SingleLeptonControl);

  // Declare handlers
  GenInfoHandler genInfoHandler;
  SimEventHandler simEventHandler;
  EventFilterHandler eventFilter(requiredTriggers);
  MuonHandler muonHandler;
  ElectronHandler electronHandler;
  JetMETHandler jetHandler;
  IsotrackHandler isotrackHandler;

  // These are called handlers, but they are more like helpers.
  DileptonHandler dileptonHandler;
  //ParticleDisambiguator particleDisambiguator;

  // Setup K factors
  genInfoHandler.setupKFactorHandles(kfactoropts);

  // Disable some advanced event tracking for skims
  eventFilter.setTrackDataEvents(false);
  eventFilter.setCheckUniqueDataEvent(false);
  eventFilter.setCheckHLTPathRunRanges(false);
  if (strjsonfile!="") eventFilter.loadGoldenJSON(strjsonfile);

  curdir->cd();

  // Acquire input tree/chain
  BaseTree* tin = new BaseTree(inputfnames, std::vector<TString>{ "Events" }, "");
  tin->sampleIdentifier = SampleHelpers::getSampleIdentifier(dset);
  bool const isData = SampleHelpers::checkSampleIsData(tin->sampleIdentifier);

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

  // Create the output file and skim tree
  // It should have the same format as far as anything is concerned.
  // That is why SetBranchStatus("...", 1) is called explicitly for all relevant collections - in case we need a variable we hadn't used during skimming.
#define COLLECTIONNAME_DIRECTIVE(NAME, LABEL, MAXSIZE) \
tin->getSelectedTree()->SetBranchStatus((GlobalCollectionNames::colName_##NAME+"*").data(), 1); \
if (MAXSIZE>0) tin->getSelectedTree()->SetBranchStatus(Form("n%s", GlobalCollectionNames::colName_##NAME.data()), 1);
  COLLECTIONNAME_COMMON_DIRECTIVES;
  if (!isData){
    COLLECTIONNAME_SIM_DIRECTIVES;
  }
#undef COLLECTIONNAME_DIRECTIVE

  TFile* foutput = TFile::Open(output_fname.data(), "recreate");
  foutput->cd();

  SimpleEntry commonEntry;
  BaseTree* tout = nullptr;
  {
    TTree* tin_copy	=	tin->getSelectedTree()->CloneTree(0);
    tout = new BaseTree(nullptr, tin_copy, nullptr, nullptr, false); // Acquires the possession of tin_copy
    tout->setAutoSave(0);
  }

  curdir->cd();

  // Keep track of sums of weights
  SelectionTracker seltracker;
  unsigned int n_zero_genwgts=0;
  double frac_zero_genwgts=0;
  double sum_wgts_noPU=0;
  double sum_abs_wgts_noPU=0;
  double sum_wgts=0;
  double sum_wgts_PUDn=0;
  double sum_wgts_PUUp=0;

  bool firstOutputEvent = true;
  unsigned int n_traversed = 0;
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

    double wgt_gensim_nominal = 1;
    if (!isData){
      double genwgt = genInfo->getGenWeight(SystematicsHelpers::sNominal);
      if (genwgt==0.){
        n_zero_genwgts++;
        continue;
      }
      sum_wgts_noPU += genwgt;
      sum_abs_wgts_noPU += std::abs(genwgt);

      double puwgt;
      puwgt = simEventHandler.getPileUpWeight(SystematicsHelpers::sNominal);
      wgt_gensim_nominal = genwgt * puwgt;
      sum_wgts += wgt_gensim_nominal;
      puwgt = simEventHandler.getPileUpWeight(SystematicsHelpers::ePUDn);
      sum_wgts_PUDn += genwgt * puwgt;
      puwgt = simEventHandler.getPileUpWeight(SystematicsHelpers::ePUUp);
      sum_wgts_PUUp += genwgt * puwgt;
    }

    eventFilter.constructFilters(&simEventHandler);

    // In what follows, note that basic selection flags for leptons and jets are set,
    // but object disambiguation is not done yet.
    // Therefore, whatever you call 'tight' is not really as tight.

    muonHandler.constructMuons();
    std::vector<MuonObject*> muons_selected;
    {
      auto const& muons = muonHandler.getProducts();
      muons_selected.reserve(muons.size());
      for (auto const& muon:muons){
        if (muon->testSelectionBit(MuonSelectionHelpers::kKinOnly)) muons_selected.push_back(muon);
      }
    }
    unsigned int const n_muons = muons_selected.size();

    electronHandler.constructElectrons();
    std::vector<ElectronObject*> electrons_selected;
    {
      auto const& electrons = electronHandler.getProducts();
      electrons_selected.reserve(electrons.size());
      for (auto const& electron:electrons){ if (electron->testSelectionBit(ElectronSelectionHelpers::kKinOnly)) electrons_selected.push_back(electron); }
    }
    unsigned int const n_electrons = electrons_selected.size();

    jetHandler.constructJetMET(&simEventHandler);
    auto const& ak4jets = jetHandler.getAK4Jets();

    // Do not apply any requirements using tight leptons because at skim level, we are not supposed to know what a tight lepton is!
    // You can, however, apply requirements on jet multiplicity:
    // If you count N for the number of jets, after cleaning, it is only going to reduce.
    // Therefore, N>N_req is only going to be slightly inefficient.
    unsigned int const n_ak4jets = ak4jets.size();
    unsigned int n_ak4jets_tight = 0;
    unsigned int n_ak4jets_tight_btagged_loose = 0; // As the name suggesst, the b-tagging WP is kept as 'loose' as possible.
    for (auto const& jet:ak4jets){
      // When we count jets, only look for jets that pass jet ID.
      // This way, when we apply JES/JER variations, we can account for changes in Nj/Nb counting because of the migration of some jets close to the pT threshold.
      if (jet->testSelectionBit(AK4JetSelectionHelpers::kJetIdOnly)){
        n_ak4jets_tight++;
        if (jet->testSelectionBit(AK4JetSelectionHelpers::kBTagged_Loose)) n_ak4jets_tight_btagged_loose++;
      }
    }

    // BEGIN PRESELECTION
    seltracker.accumulate("Full sample", wgt_gensim_nominal);

    // Do not apply HEM veto since it depends on the jet pT threshold, which itself depends on JES/JER variations.
    //if (!eventFilter.test2018HEMFilter(&simEventHandler, nullptr, nullptr, &ak4jets)) continue; // Test for 2018 partial HEM failure
    //seltracker.accumulate("Pass HEM veto", wgt_gensim_nominal);

    //if (!eventFilter.passMETFilters()) continue; // Test for MET filters
    //seltracker.accumulate("Pass MET filters", wgt_gensim_nominal);

    float event_weight_triggers_dilepton = eventFilter.getTriggerWeight(hltnames_Dilepton);
    float event_weight_triggers_singleleptoncontrol = eventFilter.getTriggerWeight(hltnames_SingleLeptonControl);
    seltracker.accumulate("Pass dilepton triggers", wgt_gensim_nominal*double(event_weight_triggers_dilepton!=0.));
    seltracker.accumulate("Pass single lepton control triggers", wgt_gensim_nominal*double(event_weight_triggers_singleleptoncontrol!=0.));
    if ((event_weight_triggers_dilepton+event_weight_triggers_singleleptoncontrol)==0.) continue; // Test if any triggers passed at all
    seltracker.accumulate("Pass any trigger", wgt_gensim_nominal);

    // Construct all possible dilepton pairs
    dileptonHandler.constructDileptons(&muons_selected, &electrons_selected);
    auto const& dileptons = dileptonHandler.getProducts();
    bool found_dilepton_SS = false;
    bool found_dilepton_OS = false;
    bool found_dilepton_OSSF_Zcand = false;
    for (auto const& dilepton:dileptons){
      if (!dilepton->isOS()) found_dilepton_SS = true;
      else{
        found_dilepton_OS = true;
        if (dilepton->isSF() && std::abs(dilepton->m()-91.2)<15.) found_dilepton_OSSF_Zcand = true;
      }
    }

    bool const pass_loose_dilepton = (
      (found_dilepton_SS || found_dilepton_OS) && (n_ak4jets_tight>=2 && n_ak4jets_tight_btagged_loose>=1)
      ||
      found_dilepton_OSSF_Zcand
      ) && event_weight_triggers_dilepton!=0.;
    bool const pass_loose_singlelepton = (n_muons + n_electrons)>=1 && n_ak4jets_tight>=1 && event_weight_triggers_singleleptoncontrol!=0.;
    seltracker.accumulate("Pass loose dilepton selection", wgt_gensim_nominal*double(pass_loose_dilepton));
    seltracker.accumulate("Pass loose single lepton control selection", wgt_gensim_nominal*double(pass_loose_singlelepton));
    if (!pass_loose_dilepton && !pass_loose_singlelepton) continue;
    seltracker.accumulate("Pass loose object selection", wgt_gensim_nominal);

    if (global_include_veto_isotracks){
      isotrackHandler.constructIsotracks(nullptr, nullptr); // Do not pass electrons or muons
      auto const& isotracks = isotrackHandler.getProducts();

      bool pass_loose_isotrack_veto = true;
      for (auto const& isotrack:isotracks){
        if (!isotrack->testSelectionBit(IsotrackSelectionHelpers::kPreselectionVeto)) continue;

        double min_dR = -1;
        for (auto const& part:muons_selected){
          double tmp_dR = part->deltaR(isotrack);
          if (min_dR<0. || min_dR>tmp_dR) min_dR = tmp_dR;
        }
        for (auto const& part:electrons_selected){
          double tmp_dR = part->deltaR(isotrack);
          if (min_dR<0. || min_dR>tmp_dR) min_dR = tmp_dR;
        }
        if (min_dR>0.4){ pass_loose_isotrack_veto=false; break; }
      }
      if (!pass_loose_isotrack_veto) continue;
      seltracker.accumulate("Pass isotrack veto", wgt_gensim_nominal);
    }

    if (!eventFilter.isUniqueDataEvent() || !eventFilter.passDataCert()) continue;
    seltracker.accumulate("Pass unique event check and data certification", wgt_gensim_nominal);

    // Accumulate any ME weights and K factors that might be present
    if (!isData){
      for (auto const& pp:genInfo->extras.LHE_ME_weights) commonEntry.setNamedVal(pp.first, pp.second);
      for (auto const& pp:genInfo->extras.Kfactors) commonEntry.setNamedVal(pp.first, pp.second);
    }

    /*************************************************/
    /* NO MORE CALLS TO SELECTION BEYOND THIS POINT! */
    /*************************************************/

    // If this is the first output event, create the tree branches based on what is available in the commonEntry.
    if (firstOutputEvent){
#define SIMPLE_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=commonEntry.named##name_t##s.begin(); itb!=commonEntry.named##name_t##s.end(); itb++) tout->putBranch(itb->first, itb->second);
#define VECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=commonEntry.namedV##name_t##s.begin(); itb!=commonEntry.namedV##name_t##s.end(); itb++) tout->putBranch(itb->first, &(itb->second));
#define DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=commonEntry.namedVV##name_t##s.begin(); itb!=commonEntry.namedVV##name_t##s.end(); itb++) tout->putBranch(itb->first, &(itb->second));
      SIMPLE_DATA_OUTPUT_DIRECTIVES;
      VECTOR_DATA_OUTPUT_DIRECTIVES;
      DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVES;
#undef SIMPLE_DATA_OUTPUT_DIRECTIVE
#undef VECTOR_DATA_OUTPUT_DIRECTIVE
#undef DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE
    }

    // Record whatever is in commonEntry into the tree.
#define SIMPLE_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=commonEntry.named##name_t##s.begin(); itb!=commonEntry.named##name_t##s.end(); itb++) tout->setVal(itb->first, itb->second);
#define VECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=commonEntry.namedV##name_t##s.begin(); itb!=commonEntry.namedV##name_t##s.end(); itb++) tout->setVal(itb->first, &(itb->second));
#define DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=commonEntry.namedVV##name_t##s.begin(); itb!=commonEntry.namedVV##name_t##s.end(); itb++) tout->setVal(itb->first, &(itb->second));
    SIMPLE_DATA_OUTPUT_DIRECTIVES;
    VECTOR_DATA_OUTPUT_DIRECTIVES;
    DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVES;
#undef SIMPLE_DATA_OUTPUT_DIRECTIVE
#undef VECTOR_DATA_OUTPUT_DIRECTIVE
#undef DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE

    tout->fill();

    if (firstOutputEvent) firstOutputEvent = false;
  }

  frac_zero_genwgts = double(n_zero_genwgts)/double(nEntries);
  IVYout << "Number of events recorded: " << tout->getNEvents() << " / " << n_traversed << " / " << nEntries << endl;
  seltracker.print();
  if (!isData){
    IVYout << "Writing the sum of gen. weights:" << endl;
    IVYout << "\t- Fraction of discarded events with null weight: " << frac_zero_genwgts << endl;
    IVYout << "\t- Before PU reweighting: " << setprecision(15) << sum_wgts_noPU << endl;
    IVYout << "\t- Fraction of negative weights before PU reweighting: " << setprecision(15) << (sum_abs_wgts_noPU-sum_wgts_noPU)*0.5/sum_abs_wgts_noPU << endl;
    IVYout << "\t- PU nominal: " << setprecision(15) << sum_wgts << endl;
    IVYout << "\t- PU down: " << setprecision(15) << sum_wgts_PUDn << endl;
    IVYout << "\t- PU up: " << setprecision(15) << sum_wgts_PUUp << endl;

    TH2D hCounters("Counters", "", 3, 0, 3, 1, 0, 1);
    hCounters.SetBinContent(0, 0, sum_wgts_noPU); // Sum with no PU reweighting
    hCounters.SetBinContent(0, 1, frac_zero_genwgts); // Fraction of discarded events
    hCounters.SetBinContent(0, 2, nEntries); // Total number of events iterated
    hCounters.SetBinContent(1, 1, sum_wgts); // Sum with nominal PU reweighting
    hCounters.SetBinContent(2, 1, sum_wgts_PUDn); // Sum with PU reweighting down
    hCounters.SetBinContent(3, 1, sum_wgts_PUUp); // Sum with PU reweighting up
    foutput->WriteTObject(&hCounters);
  }

  tout->writeToFile(foutput);
  delete tout;
  foutput->Close();

  curdir->cd();
  delete tin;

  return 0;
}

int main(int argc, char** argv){
  constexpr int iarg_offset=1; // argv[0]==[Executable name]

  bool print_help=false, has_help=false;
  std::vector<std::string> inputs;
  std::string str_dset;
  std::string str_proc;
  std::string str_period;
  std::string str_output;
  std::unordered_map<std::string, std::string> extra_arguments;
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
    else if (wish=="inputs"){
      splitOptionRecursive(value, inputs, ',', true);
    }
    else if (wish=="dataset") str_dset = value;
    else if (wish=="short_name") str_proc = value;
    else if (wish=="period") str_period = value;
    else if (wish=="output") str_output = value;
    else extra_arguments[wish] = value;
  }

  if (!print_help && (inputs.empty() || str_proc=="" || str_dset=="" || str_period=="" || str_output=="")){
    IVYerr << "ERROR: Not all mandatory inputs are present." << endl;
    print_help=true;
  }

  if (print_help){
    IVYout << "skim_UL options:\n\n";
    IVYout << "- help: Prints this help message.\n";
    IVYout << "- inputs: Comma-separated list of input files. Mandatory.\n";
    IVYout << "- dataset: Data set name. Mandatory.\n";
    IVYout << "- short_name: Process short name. Mandatory.\n";
    IVYout << "- period: Data period. Mandatory.\n";
    IVYout << "- output: Name of the output file. Could be nested in a directory. Mandatory.\n";
    IVYout << "Other optional arguments could also be passed using the structure '<argument>=<value>'.\n";

    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  SampleHelpers::configure(str_period, "skims", HostHelpers::kUCSDT2);

  std::vector<TString> strinputs; strinputs.reserve(inputs.size());
  for (auto& fname:inputs){
    if (fname.find("/store")==0) fname = std::string("root://xcache-redirector.t2.ucsd.edu:2042/")+fname;
    strinputs.push_back(fname);
  }

  return ScanChain(strinputs, str_output, str_dset, str_proc, str_period, extra_arguments);
}
