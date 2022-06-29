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
#include "MuonHandler.h"
#include "ElectronHandler.h"
#include "JetMETHandler.h"
#include "EventFilterHandler.h"
#include "SimEventHandler.h"
#include "GenInfoHandler.h"
#include "SamplesCore.h"
#include "FourTopTriggerHelpers.h"


using namespace std;
using namespace HelperFunctions;
using namespace IvyStreamHelpers;


int ScanChain(std::vector<TString> const& inputfnames, std::string output_fname, std::string dset, std::string proc, std::string str_period, double scale_factor){
  if (!SampleHelpers::checkRunOnCondor()) std::signal(SIGINT, SampleHelpers::setSignalInterrupt);

  TDirectory* curdir = gDirectory;

  {
    std::size_t tmp_pos = output_fname.find_last_of('/');
    if (tmp_pos!=std::string::npos){
      std::string output_dir(output_fname, 0, tmp_pos);
      gSystem->mkdir(output_dir.data(), true);
    }
  }

  std::vector<TriggerHelpers::TriggerType> requiredTriggers;
  std::vector<TriggerHelpers::TriggerType> const requiredTriggers_Dilepton{
    TriggerHelpers::kDoubleMu,
    TriggerHelpers::kDoubleEle,
    TriggerHelpers::kMuEle
  };
  HelperFunctions::appendVector(requiredTriggers, requiredTriggers_Dilepton);
  std::vector<TriggerHelpers::TriggerType> const requiredTriggers_SingleLeptonControl{
    TriggerHelpers::kSingleMu_Control_Iso,
    TriggerHelpers::kSingleEle_Control_Iso
  };
  HelperFunctions::appendVector(requiredTriggers, requiredTriggers_SingleLeptonControl);

  // Declare handlers
  GenInfoHandler genInfoHandler;
  SimEventHandler simEventHandler;
  EventFilterHandler eventHandler(requiredTriggers);
  MuonHandler muonHandler;
  ElectronHandler electronHandler;
  JetMETHandler jetHandler;
  //ParticleDisambiguator particleDisambiguator;

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

  eventHandler.bookBranches(tin);
  eventHandler.wrapTree(tin);

  muonHandler.bookBranches(tin);
  muonHandler.wrapTree(tin);

  electronHandler.bookBranches(tin);
  electronHandler.wrapTree(tin);

  jetHandler.bookBranches(tin);
  jetHandler.wrapTree(tin);

  // Create the output file and skim tree
  // It should have the same format as far as anything is concerned.
  // That is why SetBranchStatus("...", 1) is called explicitly for all relevant collections - in case we need a variable we hadn't used during skimming.
#define COLLECTIONNAME_DIRECTIVE(NAME, LABEL, MAXSIZE) tin->getSelectedTree()->SetBranchStatus((GlobalCollectionNames::colName_##NAME+"*").data(), 1);
  COLLECTIONNAME_DIRECTIVES;
#undef COLLECTIONNAME_DIRECTIVE

  TFile* foutput = TFile::Open(output_fname.data(), "recreate");
  TTree* tout	=	tin->getSelectedTree()->CloneTree(0);
  tout->SetAutoSave(0);

  // Keep track of sums of weights
  unsigned int n_zero_genwgts=0;
  double frac_zero_genwgts=0;
  double sum_wgts_noPU=0;
  double sum_abs_wgts_noPU=0;
  double sum_wgts=0;
  double sum_wgts_PUDn=0;
  double sum_wgts_PUUp=0;

  int nEvents = tin->getNEvents();
  for (int ev=0; ev<nEvents; ev++){
    if (SampleHelpers::doSignalInterrupt==1) break;

    tin->getEvent(ev);
    HelperFunctions::progressbar(ev, nEvents);

    genInfoHandler.constructGenInfo();
    auto const& genInfo = genInfoHandler.getGenInfo();
    double genwgt = genInfo->getGenWeight(SystematicsHelpers::sNominal);
    if (genwgt==0.){
      n_zero_genwgts++;
      continue;
    }
    sum_wgts_noPU += genwgt;
    sum_abs_wgts_noPU += std::abs(genwgt);

    double puwgt;
    simEventHandler.constructSimEvent();
    puwgt = simEventHandler.getPileUpWeight(SystematicsHelpers::sNominal);
    sum_wgts += genwgt * puwgt;
    puwgt = simEventHandler.getPileUpWeight(SystematicsHelpers::ePUDn);
    sum_wgts_PUDn += genwgt * puwgt;
    puwgt = simEventHandler.getPileUpWeight(SystematicsHelpers::ePUUp);
    sum_wgts_PUUp += genwgt * puwgt;

    eventHandler.constructFilters(&simEventHandler);

    muonHandler.constructMuons();
    auto const& muons = muonHandler.getProducts();

    electronHandler.constructElectrons();
    auto const& electrons = electronHandler.getProducts();

    jetHandler.constructJetMET(&simEventHandler);
    auto const& ak4jets = jetHandler.getAK4Jets();
    auto const& eventmet = jetHandler.getPFMET();

    std::vector<MuonObject*> muons_tight;
    muons_tight.resize(muons.size());
    for (auto const& part:muons){
      if (part->testSelectionBit(MuonSelectionHelpers::kPreselectionTight)){
        muons_tight.push_back(part);
      }
    }

    std::vector<ElectronObject*> electrons_tight;
    electrons_tight.resize(electrons.size());
    for (auto const& part:electrons){
      if (part->testSelectionBit(ElectronSelectionHelpers::kPreselectionTight)){
        electrons_tight.push_back(part);
      }
    }

    std::vector<ParticleObject*> leptons_tight;
    leptons_tight.resize(electrons_tight.size() + muons_tight.size());
    for (auto const& part:muons) leptons_tight.push_back(part);
    for (auto const& part:electrons) leptons_tight.push_back(part);



    // BEGIN PRESELECTION



    tout->Fill();
  }

  frac_zero_genwgts = double(n_zero_genwgts)/double(nEvents);
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
    hCounters.SetBinContent(0, 2, nEvents); // Total number of events iterated
    hCounters.SetBinContent(1, 1, sum_wgts); // Sum with nominal PU reweighting
    hCounters.SetBinContent(2, 1, sum_wgts_PUDn); // Sum with PU reweighting down
    hCounters.SetBinContent(3, 1, sum_wgts_PUUp); // Sum with PU reweighting up
    foutput->WriteTObject(&hCounters);
  }

  foutput->WriteTObject(tout);
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
  double xsec_br = 1;
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
    else if (wish=="xsec") xsec_br *= std::stod(value);
    else if (wish=="BR") xsec_br *= std::stod(value);
    else{
      IVYerr << "ERROR: Unknown argument " << wish << " = " << value << endl;
      print_help=true;
    }
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
    IVYout << "- xsec: xsec. Defaults to 1.\n";
    IVYout << "- BR: Branching fraction. Defaults to 1.\n";

    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  SampleHelpers::configure(str_period, "skims", HostHelpers::kUCSDT2);

  std::vector<TString> strinputs; strinputs.reserve(inputs.size());
  for (auto& fname:inputs){
    if (fname.find("/store")==0) fname = std::string("root://xcache-redirector.t2.ucsd.edu:2042/")+fname;
    strinputs.push_back(fname);
  }

  return ScanChain(strinputs, str_output, str_dset, str_proc, str_period, xsec_br);
}
