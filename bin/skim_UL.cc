#include "TSystem.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TTree.h"
#include "TBranch.h"
#include "TChain.h"
#include "THStack.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TObjString.h"
#include "TTreeCache.h"

#include <NanoTools/NanoCORE/Nano.h>
#include <NanoTools/NanoCORE/tqdm.h>
#include <IvyFramework/IvyDataTools/interface/HelperFunctions.h>
#include <IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh>

#include "analysis_types.h"
#include "tree_tools.h"

#include <string>
#include <iostream>
#include <iomanip>

#define SUM(vec) std::accumulate((vec).begin(), (vec).end(), 0);
#define SUM_GT(vec,num) std::accumulate((vec).begin(), (vec).end(), 0, [](float x,float y){return ((y > (num)) ? x+y : x); });
#define COUNT_GT(vec,num) std::count_if((vec).begin(), (vec).end(), [](float x) { return x > (num); });
#define COUNT_LT(vec,num) std::count_if((vec).begin(), (vec).end(), [](float x) { return x < (num); });

// TODO: (low priority) create namespace and replace this directive with functions
#define H1(name,nbins,low,high) TH1F* h_##name = new TH1F(#name,#name,nbins,low,high);

struct debugger { template<typename T> debugger& operator , (const T& v) { cerr<<v<<" "; return* this; } } dbg;
#ifdef DEBUG
    #define debug(args...) do {cerr << #args << ": "; dbg,args; cerr << endl;} while(0)
#else
    #define debug(args...)
#endif


using namespace std;
using namespace HelperFunctions;
using namespace IvyStreamHelpers;


int ScanChain(std::vector<std::string> const& inputfnames, string output_fname, string proc, string str_year, float scale_factor = 1){
  TDirectory* curdir = gDirectory;

  {
    std::size_t tmp_pos = output_fname.find_last_of('/');
    if (tmp_pos!=std::string::npos){
      std::string output_dir(output_fname, 0, tmp_pos);
      gSystem->mkdir(output_dir.data(), true);
    }
  }

  TChain* ch = new TChain("Events");
  for (auto const& fname:inputfnames) ch->Add(fname.data());

  if (str_year == "2016_APV") year = 2016;
  else year = stoi(str_year);

  TFile* foutput = TFile::Open(output_fname.data(), "recreate");
  TTree* tout	=	new TTree("Events", "");

#define SIMPLE_DATA_DIRECTIVE(type, name, default_value) \
	tout->Branch(#name, &name);
  SIMPLE_DATA_DIRECTIVES
#undef SIMPLE_DATA_DIRECTIVE

  int nEventsTotal = 0;
  int nEventsChain = ch->GetEntries();
  TFile* currentFile = nullptr;
  TObjArray* listOfFiles = ch->GetListOfFiles();
  TIter fileIter(listOfFiles);
  tqdm bar;

  while ((currentFile = (TFile*) fileIter.Next())) {
    TFile* file = TFile::Open(currentFile->GetTitle());
    TTree* tin = (TTree*) file->Get("Events");

    tin->SetCacheSize(128*1024*1024);
    tin->SetCacheLearnEntries(100);

    nt.SetYear(year);
    nt.Init(tin);

    for (int ev=0; ev<tin->GetEntries(); ev++){
      clear_branches();
      nt.GetEntry(ev);
      nEventsTotal++;
      bar.progress(nEventsTotal, nEventsChain);

      // baseline AN selection goes here

      tout->Fill();
    }
    file->Close();
  }
  bar.finish();

  foutput->WriteTObject(tout);
  delete tout;
  foutput->Close();

  curdir->cd();
  delete ch;

  return 0;
}

int main(int argc, char** argv){
  constexpr int iarg_offset=1; // argv[0]==[Executable name]

  bool print_help=false, has_help=false;
  std::vector<std::string> inputs;
  std::string str_proc;
  std::string str_year;
  std::string str_output;
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
    else if (wish=="short_name") str_proc = value;
    else if (wish=="period") str_year = value;
    else if (wish=="output") str_output = value;
    else{
      IVYerr << "ERROR: Unknown argument " << wish << " = " << value << endl;
      print_help=true;
    }
  }

  if (!print_help && (inputs.empty() || str_proc=="" || str_year=="" || str_output=="")){
    IVYerr << "ERROR: Not all mandatory inputs are present." << endl;
    print_help=true;
  }

  if (print_help){
    IVYout << "skim_UL options:\n\n";
    IVYout << "- help: Prints this help message.\n";
    IVYout << "- inputs: Comma-separated list of input files. Mandatory.\n";
    IVYout << "- short_name: Process. Mandatory.\n";
    IVYout << "- period: Data period. Mandatory.\n";
    IVYout << "- output: Name of the output file. Could be nested in a directory. Mandatory.\n";

    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  // Here begins the execution.
  for (auto& fname:inputs){
    if (fname.find("/store")==0) fname = std::string("root://xcache-redirector.t2.ucsd.edu:2042/")+fname;
  }

  return ScanChain(inputs, str_output, str_proc, str_year, 1);
}