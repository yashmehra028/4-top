//#pragma GCC diagnostic ignored "-Wsign-compare"

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
#include "TTreeCacheUnzip.h"
#include "TTreePerfStats.h"

#include <NanoTools/NanoCORE/Nano.h>
#include <NanoTools/NanoCORE/tqdm.h>

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

int ScanChain(TChain* ch, string proc, string str_year, string tag, float scale_factor = 1) {
  if (str_year == "2016_APV") year = 2016;
  else year = stoi(str_year);

  TString file_name = proc + "_" + tag + "_" + str_year;

  TFile* foutput = TFile::Open("outputs_UL/" + file_name + ".root", "RECREATE");
  TTree* tout	=	new TTree("Events", "output tree");

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

  return 0;
}

int main(int argc, char** argv){

}