#pragma GCC diagnostic ignored "-Wsign-compare"
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

#include "../NanoTools/NanoCORE/Nano.cc"
#include "../NanoTools/NanoCORE/tqdm.h"

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
#define H1(name,nbins,low,high) TH1F *h_##name = new TH1F(#name,#name,nbins,low,high);

struct debugger { template<typename T> debugger& operator , (const T& v) { cerr<<v<<" "; return *this; } } dbg;
#ifdef DEBUG
    #define debug(args...) do {cerr << #args << ": "; dbg,args; cerr << endl;} while(0)
#else
    #define debug(args...)
#endif

using namespace std;

int ScanChain( TChain *ch, string proc, string str_year, string tag, float scale_factor = 1 ) {
	int year;
	if ( str_year == "2016_APV") year = 2016;
	else { year = stoi(str_year); }
	
	TString file_name = proc + "_" + tag + "_" + str_year;
	
	TFile* f1 = new TFile("outputs_UL/" + file_name + ".root", "RECREATE");
	
	TTree *out_tree	=	new TTree("Events","output tree");
	
	#define SIMPLE_DATA_DIRECTIVE(type, name, default_value) \
	out_tree->Branch(#name, &name);
	SIMPLE_DATA_DIRECTIVES
	#undef SIMPLE_DATA_DIRECTIVE

	int nEventsTotal = 0;
	int nEventsChain = ch->GetEntries();
	TFile *currentFile = 0;
	TObjArray *listOfFiles = ch->GetListOfFiles();
	TIter fileIter(listOfFiles);
	tqdm bar;
	
	while ( (currentFile = (TFile*)fileIter.Next()) ) {
		TFile *file = TFile::Open( currentFile->GetTitle() );
		TTree *tree = (TTree*)file->Get("Events");
		
		tree->SetCacheSize(128*1024*1024);
		tree->SetCacheLearnEntries(100);
		
		nt.SetYear(year);
		nt.Init(tree);
		
		for( unsigned int loop_event = 0; loop_event < tree->GetEntriesFast(); ++loop_event) {
			nt.GetEntry(loop_event);
			nEventsTotal++;
			bar.progress(nEventsTotal, nEventsChain);
			// baseline AN selection goes here
			out_tree->Fill();
			clear_branches();
		
		} // Event loop
	    delete file;
	} // File loop
	bar.finish();
	f1->Write();
	f1->Close();
	return 0;
}
