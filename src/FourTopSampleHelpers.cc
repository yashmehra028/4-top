#include <cassert>
#include "HostHelpersCore.h"
#include "HelperFunctionsCore.h"
#include "SamplesCore.h"
#include "FourTopTriggerHelpers.h"


using namespace std;
using namespace IvyStreamHelpers;


void SampleHelpers::configure(TString period, TString stag, HostHelpers::Hosts input_host){
  setDataPeriod(period);

  // FIXME: This is to be decided later, but put an ansatz for now.
  TString strInputDir = "/home/users/usarica/work/FourTop";
  if (stag.Contains(":")){
    std::vector<TString> splitstr; char delimiter=':';
    HelperFunctions::splitOptionRecursive(stag, splitstr, delimiter);
    assert(splitstr.size()<=3);
    stag = splitstr.back();
    if (splitstr.size()>=2){ // Order goes as "[store/ceph/nfs-7/home]:[user (optional)]:[tag]
      TString const& strlocation = splitstr.front();
      if (strlocation == "skims") strInputDir += "/Skims";
      else if (strlocation == "finaltrees") strInputDir += "/FinalTrees";

      if (splitstr.size()==3) HelperFunctions::replaceString<TString, const TString>(strInputDir, "usarica", splitstr.at(1));
    }
  }
  if (strInputDir.BeginsWith("/store")) strInputDir = HostHelpers::GetStandardHostPathToStore(strInputDir, input_host);

  TriggerHelpers::configureHLTmap();

  runConfigure = true;
}
