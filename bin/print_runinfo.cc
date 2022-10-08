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
#include "IvyFramework/IvyDataTools/interface/HelperFunctionsCore.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "SamplesCore.h"


using namespace std;
using namespace HelperFunctions;
using namespace IvyStreamHelpers;


int main(int argc, char** argv){
  constexpr int iarg_offset=1; // argv[0]==[Executable name]

  bool print_help=false, has_help=false;
  std::string str_period;
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
    else if (wish=="period") str_period = value;
    else{
      IVYerr << "ERROR: Unknown argument " << wish << "=" << value << endl;
      print_help=true;
    }
  }

  if (!print_help && str_period==""){
    IVYerr << "ERROR: Not all mandatory inputs are present." << endl;
    print_help=true;
  }

  if (print_help){
    IVYout << argv[0] << " options:\n\n";
    IVYout << "- help: Prints this help message.\n";
    IVYout << "- period: Data period. Mandatory.\n";
    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  SampleHelpers::setDataPeriod(str_period);
  auto const& runnumber_lumi_pairs = SampleHelpers::getRunNumberLumiPairsForDataPeriod(SampleHelpers::getDataPeriod());
  for (auto const& pp:runnumber_lumi_pairs) IVYout << pp.first << ":" << pp.second << endl;

  return 0;
}
