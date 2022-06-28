#include <cassert>
#include "SystematicVariations.h"
#include "SamplesCore.h"


using namespace std;
using namespace IvyStreamHelpers;


std::string SystematicsHelpers::getSystCoreName(SystematicsHelpers::SystematicVariationTypes const& type){
  switch (type){
  case sNominal:
    return "Nominal";

  case tPDFScaleDn:
  case tPDFScaleUp:
    return "FacScale";
  case tQCDScaleDn:
  case tQCDScaleUp:
    return "RenScale";
  case tAsMZDn:
  case tAsMZUp:
    return "AsMZ";
  case tPDFReplicaDn:
  case tPDFReplicaUp:
    return "PDFVariation";
  case tPythiaScaleDn:
  case tPythiaScaleUp:
    return "PythiaScale";
  case tPythiaTuneDn:
  case tPythiaTuneUp:
    return "PythiaTune";

  case eEleEffDn:
  case eEleEffUp:
    return "ElectronEff";

  case eMuEffDn:
  case eMuEffUp:
    return "MuonEff";

  case ePhoEffDn:
  case ePhoEffUp:
    return "PhotonEff";

  case eJECDn:
  case eJECUp:
    return "JEC";
  case eJERDn:
  case eJERUp:
    return "JER";
  case ePUDn:
  case ePUUp:
    return "PU";
  case ePUJetIdEffDn:
  case ePUJetIdEffUp:
    return "PUJetIdEff";
  case eBTagSFDn:
  case eBTagSFUp:
    return "BTagSF";

  case eL1PrefiringDn:
  case eL1PrefiringUp:
    return "L1Prefiring";

  case eTriggerEffDn:
  case eTriggerEffUp:
    return "TriggerEff";

  case sUncorrected:
    return "Uncorrected";

  default:
    IVYerr << "SystematicsHelpers::getSystCoreName: Systematic " << type << " is not defined!" << endl;
    assert(0);
    return "";
  }
}
bool SystematicsHelpers::isDownSystematic(SystematicsHelpers::SystematicVariationTypes const& type){
  if (type<nSystematicVariations && type!=sNominal) return (((int) type)%2 == 1);
  else return false;
}
bool SystematicsHelpers::isUpSystematic(SystematicsHelpers::SystematicVariationTypes const& type){
  if (type<nSystematicVariations && type!=sNominal) return (((int) type)%2 == 0);
  else return false;
}
SystematicsHelpers::SystematicVariationTypes SystematicsHelpers::getSystComplement(SystematicsHelpers::SystematicVariationTypes const& type){
  bool isDn = SystematicsHelpers::isDownSystematic(type);
  bool isUp = SystematicsHelpers::isUpSystematic(type);
  if (isDn == isUp) return type;
  else if (isDn) return static_cast<SystematicsHelpers::SystematicVariationTypes>(static_cast<int>(type)+1);
  else /*if (isUp)*/ return static_cast<SystematicsHelpers::SystematicVariationTypes>(static_cast<int>(type)-1);
}
std::string SystematicsHelpers::getSystName(SystematicsHelpers::SystematicVariationTypes const& type){
  std::string res = getSystCoreName(type);
  if (SystematicsHelpers::isDownSystematic(type)) res = res + "Dn";
  else if (SystematicsHelpers::isUpSystematic(type)) res = res + "Up";
  return res;
}

// The following numbers are taken from https://twiki.cern.ch/twiki/bin/view/CMS/TWikiLUM#LumiComb.
// Documentation can be found one subsection above in the same twiki, https://twiki.cern.ch/twiki/bin/view/CMS/TWikiLUM#TabLum.
double SystematicsHelpers::getLumiUncertainty_Uncorrelated(){
  auto const& year = SampleHelpers::getDataYear();
  double res = 0;
  switch (year){
  case 2011:
    res = 2.2;
    break;
  case 2012:
    res = 2.6;
    break;
  case 2015:
    res = 0.9;
    break;
  case 2016:
    res = 0.6;
    break;
  case 2017:
    res = 2.0;
    break;
  case 2018:
    res = 1.5;
    break;
  default:
    IVYerr << "SystematicsHelpers::getLumiUncertainty_Uncorrelated: Year " << year << " is unspecified." << endl;
    assert(0);
    break;
  }
  res *= 0.01;
  return res;
}
double SystematicsHelpers::getLumiUncertainty_Correlated(){
  auto const& year = SampleHelpers::getDataYear();
  double res = 0;
  switch (year){
  case 2015:
    res = 0.583;
    break;
  case 2016:
    res = 0.625;
    break;
  case 2017:
    res = 0.911;
    break;
  case 2018:
    res = 2.022;
    break;
  default:
    IVYerr << "SystematicsHelpers::getLumiUncertainty_Correlated: Year " << year << " is unspecified." << endl;
    assert(0);
    break;
  }
  res *= 0.01;
  return res;
}
double SystematicsHelpers::getLumiUncertainty_Correlated_2015_2016(){
  auto const& year = SampleHelpers::getDataYear();
  double res = 0;
  switch (year){
  case 2015:
    res = 1.114;
    break;
  case 2016:
    res = 0.877;
    break;
  default:
    break;
  }
  res *= 0.01;
  return res;
}
double SystematicsHelpers::getLumiUncertainty_Correlated_2017_2018(){
  auto const& year = SampleHelpers::getDataYear();
  double res = 0;
  switch (year){
  case 2017:
    res = 0.6;
    break;
  case 2018:
    res = 0.2;
    break;
  default:
    break;
  }
  res *= 0.01;
  return res;
}
