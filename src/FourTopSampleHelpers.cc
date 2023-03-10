#include <cassert>
#include "IvyFramework/IvyDataTools/interface/HostHelpersCore.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctionsCore.h"
#include "SamplesCore.h"
#include "FourTopTriggerHelpers.h"
#include "BtagHelpers.h"


using namespace std;
using namespace IvyStreamHelpers;


// 'period' should be 2016_APV, 2016_NonAPV, 2017, 2018 etc.
// 'stag' should have the format [skims/finaltrees/home]:[tag].
// - 'skims' -> 'Skims' and 'finaltrees' -> 'FinalTrees' in standard /store/group/tttt CMS area. The argument 'input_host' (see description below) determines the absolute path.
// - 'home' is <tttt package directory>/output.
// - 'tag' is whatever name you choose/chose to distinguish ntuple versions.
// 'input_host' is one of the host sites defined in the HostHelpers namespace Hosts enums. The default is HostHelpers::kUCSDT2.
void SampleHelpers::configure(TString period, TString stag, HostHelpers::Hosts input_host){
  setDataPeriod(period);

  TString strInputDir = DEFAULTSTOREINPUTDIR;
  theInputTag = "";
  if (stag.Contains(":")){
    std::vector<TString> splitstr; char delimiter=':';
    HelperFunctions::splitOptionRecursive(stag, splitstr, delimiter);
    assert(splitstr.size()<=3);
    stag = splitstr.back();

    TString const& strlocation = splitstr.front();
    if (strlocation == "skims") strInputDir += "/Skims";
    else if (strlocation == "finaltrees") strInputDir += "/FinalTrees";
    else if (strlocation == "home") strInputDir = ANALYSISPKGPATH + "/output";
    if (splitstr.size()>=2){
      theInputTag = stag;
      strInputDir = strInputDir + "/" + theInputTag;
    }
  }
  if (strInputDir.BeginsWith("/store")) strInputDir = HostHelpers::GetStandardHostPathToStore(strInputDir, input_host);

  SampleHelpers::setInputDirectory(strInputDir);
  SampleHelpers::setInputTag(theInputTag);

  runConfigure = true;

  TriggerHelpers::configureHLTmap();
  BtagHelpers::configureBtagWPs();
}
