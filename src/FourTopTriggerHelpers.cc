#include <cassert>
#include <regex>
#include "FourTopTriggerHelpers.h"
#include "SamplesCore.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctionsCore.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


namespace TriggerHelpers{
  std::unordered_map< TriggerHelpers::TriggerType, std::vector<std::string> > HLT_type_list_map;
  std::unordered_map< TriggerHelpers::TriggerType, std::vector<HLTTriggerPathProperties> > HLT_type_proplist_map;
  std::vector<HLTTriggerPathProperties const*> runRangeExcluded_HLTprop_list; // This list is only for ease

  std::unordered_map< HLTTriggerPathProperties const*, std::vector<std::pair<TriggerObjectType, unsigned long long int>> > HLT_prop_TOreqs_map;
  std::vector<std::pair<TriggerObjectType, unsigned long long int>> getTriggerObjectReqs(HLTTriggerPathProperties const& hltprop);

  void assignRunRangeExclusions(std::string const& name, std::vector< std::pair<int, int> > const& rangelist);
  void assignTriggerObjectCheckException(std::string const& name, HLTTriggerPathProperties::TriggerObjectExceptionType const& flag);
}


using namespace std;
using namespace IvyStreamHelpers;


std::vector<std::string> TriggerHelpers::getHLTMenus(TriggerHelpers::TriggerType type){ return getHLTMenus(std::vector<TriggerHelpers::TriggerType>{ type }); }
std::vector<std::string> TriggerHelpers::getHLTMenus(std::vector<TriggerHelpers::TriggerType> const& types){
  std::vector<std::string> res;
  for (auto const& type:types){
    auto it = HLT_type_list_map.find(type);
    if (it != HLT_type_list_map.end()) HelperFunctions::appendVector(res, it->second);
  }
  return res;
}
std::vector< std::pair<TriggerHelpers::TriggerType, HLTTriggerPathProperties const*> > TriggerHelpers::getHLTMenuProperties(TriggerHelpers::TriggerType type){ return getHLTMenuProperties(std::vector<TriggerHelpers::TriggerType>{ type }); }
std::vector< std::pair<TriggerHelpers::TriggerType, HLTTriggerPathProperties const*> > TriggerHelpers::getHLTMenuProperties(std::vector<TriggerHelpers::TriggerType> const& types){
  unsigned int isize=0;
  for (auto const& type:types){
    std::unordered_map< TriggerHelpers::TriggerType, std::vector<HLTTriggerPathProperties> >::const_iterator it = HLT_type_proplist_map.find(type);
    if (it == HLT_type_proplist_map.cend()){
      IVYerr << "TriggerHelpers::getHLTMenuProperties: Trigger type " << type << " is not defined in the HLT type-properties map." << endl;
      assert(0);
    }
    isize += it->second.size();
  }

  std::vector< std::pair<TriggerHelpers::TriggerType, HLTTriggerPathProperties const*> > res; res.reserve(isize);
  for (auto const& type:types){
    std::unordered_map< TriggerHelpers::TriggerType, std::vector<HLTTriggerPathProperties> >::const_iterator it = HLT_type_proplist_map.find(type);
    if (it == HLT_type_proplist_map.cend()){
      IVYerr << "TriggerHelpers::getHLTMenuProperties: Trigger type " << type << " is not defined in the HLT type-properties map." << endl;
      assert(0);
    }
    for (auto const& hltprop:it->second) res.emplace_back(type, &hltprop);
  }

  return res;
}
std::vector<std::pair<TriggerObjectType, unsigned long long int>> const& TriggerHelpers::getHLTMenuTriggerObjectReqs(HLTTriggerPathProperties const* hltprop){
  assert(hltprop!=nullptr);
  auto it = HLT_prop_TOreqs_map.find(hltprop);
  if (it==HLT_prop_TOreqs_map.cend()){
    IVYerr << "TriggerHelpers::getHLTMenuTriggerObjectReqs: Trigger object requiremens for " << hltprop->getName() << " is not found." << endl;
    assert(0);
  }
  return it->second;
}

void TriggerHelpers::dropSelectionCuts(TriggerHelpers::TriggerType type){
  std::unordered_map< TriggerHelpers::TriggerType, std::vector<HLTTriggerPathProperties> >::iterator it = HLT_type_proplist_map.find(type);
  if (it != HLT_type_proplist_map.end()){ for (auto& props:it->second) props.resetCuts(); }
  else{
    IVYerr << "TriggerHelpers::dropSelectionCuts: Trigger type " << type << " is not defined." << endl;
    assert(0);
  }
}

void TriggerHelpers::assignRunRangeExclusions(std::string const& name, std::vector< std::pair<int, int> > const& rangelist){
  if (rangelist.empty()) return;

  std::pair<int, int> runrange_dp(-1, -1);
  for (auto const& pp:SampleHelpers::getRunRangesFromDataPeriod(SampleHelpers::getDataPeriod())){
    if (runrange_dp.first<0 || runrange_dp.first>pp.first) runrange_dp.first = pp.first;
    if (runrange_dp.second<0 || runrange_dp.second<pp.second) runrange_dp.second = pp.second;
  }

  std::vector< std::pair<unsigned int, unsigned int> > rangelist_eff; rangelist_eff.reserve(rangelist.size());

  // Place run range exclusions only when needed!
  // Do not place them if the excluded run range falls outside the data period run range!
  // Also modify run ranges in case they contain -1.
  for (auto pp:rangelist){
    if (pp.first == -1) pp.first = runrange_dp.first;
    if (pp.second == -1) pp.second = runrange_dp.second;
    if ((unsigned int) pp.first>runrange_dp.second || (unsigned int) pp.second<runrange_dp.first) continue;
    rangelist_eff.emplace_back(pp.first, pp.second);
  }

  if (rangelist_eff.empty()) return;

  for (int itt=0; itt!=(int) nTriggerTypes; itt++){
    // No need to check the iterator since it was already checked.
    auto it = HLT_type_proplist_map.find((TriggerType) itt);
    for (auto& hltprop:it->second){
      if (!hltprop.isSameTrigger(name)) continue;

      IVYout << "TriggerHelpers::assignRunRangeExclusions: Adding run range exclusion to " << name << ". Excluded ranges = " << rangelist_eff << endl;

      hltprop.setExcludedRunRanges(rangelist_eff);
      runRangeExcluded_HLTprop_list.push_back(&hltprop);
    }
  }
}
bool TriggerHelpers::hasRunRangeExclusions(std::string const& name, HLTTriggerPathProperties const** out_hltprop){
  if (out_hltprop) *out_hltprop = nullptr;
  for (auto const& hltprop:runRangeExcluded_HLTprop_list){
    if (!hltprop->isSameTrigger(name)) continue;
    if (out_hltprop) *out_hltprop = hltprop;
    return true;
  }
  return false;
}

void TriggerHelpers::assignTriggerObjectCheckException(std::string const& name, HLTTriggerPathProperties::TriggerObjectExceptionType const& flag){
  for (int itt=0; itt!=(int) nTriggerTypes; itt++){
    // No need to check the iterator since it was already checked.
    auto it = HLT_type_proplist_map.find((TriggerType) itt);
    for (auto& hltprop:it->second){
      if (!hltprop.isSameTrigger(name)) continue;

      IVYout << "TriggerHelpers::assignTriggerObjectCheckException: Adding TO exception of type " << flag << " to " << name << "." << endl;

      hltprop.setTOException(flag);
    }
  }
}

// Here be the dragons...
// Note that object matching is done for only muons, electrons, and photons, both here and in EventFilterHandler.cc.
// If one would like to match jets (why???), one has to edit both this function
// and EventFilterHandler::getTriggerWeight (i.e., search for the call blocks to TriggerObject::getMatchedPhysicsObjects in EventFilterHandler).
std::vector<std::pair<TriggerObjectType, unsigned long long int>> TriggerHelpers::getTriggerObjectReqs(HLTTriggerPathProperties const& hltprop){
  static bool print_warnings = true;

  std::vector<std::pair<TriggerObjectType, unsigned long long int>> res;

  auto const& dy = SampleHelpers::getDataYear();
  auto const& hltname = hltprop.getName();
  auto const& hltTOtype_hltprop_map = hltprop.getObjectProperties();
  unsigned short nm_req = 0;
  unsigned short ne_req = 0;
  unsigned short npho_req = 0;
  unsigned short nj_req = 0;
  for (auto const& pp:hltTOtype_hltprop_map){
    switch (pp.first){
    case HLTObjectProperties::kMuon:
      nm_req = pp.second.size();
      break;
    case HLTObjectProperties::kElectron:
      ne_req = pp.second.size();
      break;
    case HLTObjectProperties::kPhoton:
      npho_req = pp.second.size();
      break;
    case HLTObjectProperties::kAK4Jet:
      nj_req = pp.second.size();
      break;
    default:
      break;
    }
  }

  // Mask requirements taken directly from NanoAOD code
  // because the documentation is illegible:
  // https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/NanoAOD/python/triggerObjects_cff.py

  if (print_warnings && dy>2018) IVYout << "TriggerHelpers::getTriggerObjectReqs: WARNING! Year " << dy << " uses 2018 masks, but it might need to be validated." << endl;

  unsigned short nm = 0;
  unsigned long long int mask_mu = 0;
  unsigned short ne = 0;
  unsigned long long int mask_e = 0;
  unsigned short npho = 0;
  unsigned long long int mask_pho = 0;
  unsigned short nj = 0;
  //unsigned long long int mask_j = 0;

  // Muons
  if (hltname.find("TripleMu")!=std::string::npos) nm = 3;
  else if (hltname.find("DoubleMu")!=std::string::npos || hltname.find("DiMu")!=std::string::npos) nm = 2;
  else{
    std::regex rgx("(No)?(Iso)?(Tk)?(Old)?(Mu[0-9]+)(_TrkIsoVVL)?");
    auto it_begin = std::sregex_iterator(hltname.begin(), hltname.end(), rgx);
    auto it_end = std::sregex_iterator();
    for (auto it=it_begin; it!=it_end; it++){
      std::smatch sm = *it;
      std::string strmatch = sm.str();

      if (strmatch.find("NoMu")!=std::string::npos) continue; // This is a PF*NoMu trigger, ignore it.

      std::string stest;
      nm++;
      // Tests for specific filters
      stest = "TrkIsoVVL";
      if (strmatch.find(stest)!=std::string::npos){
        switch (dy){
        case 2016:
        case 2017:
        case 2018:
        case 2022:
          HelperFunctions::set_bit(mask_mu, 0);
          break;
        default:
          IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
          assert(0);
        }
      }
      stest = "IsoMu";
      if (strmatch.find(stest)!=std::string::npos){
        switch (dy){
        case 2016:
        case 2017:
        case 2018:
        case 2022:
          HelperFunctions::set_bit(mask_mu, 1);
          break;
        default:
          IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
          assert(0);
        }
      }
      stest = "IsoTkMu";
      if (strmatch.find(stest)!=std::string::npos){
        switch (dy){
        case 2016:
          HelperFunctions::set_bit(mask_mu, 3);
          break;
        default:
          IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
          assert(0);
        }
      }
      stest = "Mu50";
      if (strmatch.find(stest)!=std::string::npos){
        switch (dy){
        case 2016:
        case 2017:
        case 2018:
        case 2022:
          HelperFunctions::set_bit(mask_mu, 10);
          break;
        default:
          IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
          assert(0);
        }
      }
      stest = "Mu100";
      if (strmatch.find(stest)!=std::string::npos){
        switch (dy){
        case 2017:
        case 2018:
        case 2022:
          HelperFunctions::set_bit(mask_mu, 11);
          break;
        default:
          IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
          assert(0);
        }
      }
    }
  }
  if (nm!=nm_req){
    IVYerr << "TriggerHelpers::getTriggerObjectReqs: The trigger " << hltname << " is specified to require " << nm_req << " muons, but only " << nm << " are found during the parsing of its name." << endl;
    assert(0);
  }

  // Electrons
  if (hltname.find("DoubleEle")!=std::string::npos || hltname.find("DiEle")!=std::string::npos) ne = 2;
  else if (ne_req==2 && hltname.find("HLT_DoublePhoton")!=std::string::npos){
    // Special case:
    // HLT_DoublePhoton*_v* can be used for high-pt electrons.
    ne = 2;
    switch (dy){
    case 2016:
    case 2017:
    case 2018:
    case 2022:
      // No corresponding bits in NanoAOD, such an oversight!
      break;
    default:
      IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for DoublePhoton*." << endl;
      assert(0);
    }
  }
  else if (ne_req==1 && (hltname=="HLT_Photon175_v" || hltname=="HLT_Photon200_v")){
    // Special case:
    // HLT_Photon{175,200}_v* can be used for high-pt electrons.
    // Note that a precise HLT path name check is done, and without '*' included.
    ne = 1;
    switch (dy){
    case 2016:
    case 2017:
    case 2018:
    case 2022:
      HelperFunctions::set_bit(mask_e, 13);
      break;
    default:
      IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for Photon{175,200}." << endl;
      assert(0);
    }
  }
  else{
    std::regex rgx("(Ele[0-9]+)");
    auto it_begin = std::sregex_iterator(hltname.begin(), hltname.end(), rgx);
    auto it_end = std::sregex_iterator();
    for (auto it=it_begin; it!=it_end; it++){
      std::smatch sm = *it;
      std::string strmatch = sm.str();
      ne++;
    }
  }
  if (ne>0){
    std::string stest;
    // Tests for specific filters
    stest = "CaloIdL_TrackIdL_IsoVL";
    if (hltname.find(stest)!=std::string::npos){
      switch (dy){
      case 2016:
      case 2017:
      case 2018:
      case 2022:
        HelperFunctions::set_bit(mask_e, 0);
        break;
      default:
        IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
        assert(0);
      }
    }
    stest = "WPTight";
    if (hltname.find(stest)!=std::string::npos){
      switch (dy){
      case 2016:
      case 2017:
      case 2018:
      case 2022:
        HelperFunctions::set_bit(mask_e, 1);
        break;
      default:
        IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
        assert(0);
      }
    }
    stest = "WPLoose";
    if (hltname.find(stest)!=std::string::npos){
      switch (dy){
      case 2016:
      case 2017:
      case 2018:
      case 2022:
        HelperFunctions::set_bit(mask_e, 2);
        break;
      default:
        IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
        assert(0);
      }
    }
    stest = "CaloIdVT_GsfTrkIdT";
    if (hltname.find(stest)!=std::string::npos){
      switch (dy){
      case 2016:
      case 2017:
      case 2018:
      case 2022:
        HelperFunctions::set_bit(mask_e, 11);
        break;
      default:
        IVYerr << "TriggerHelpers::getTriggerObjectReqs: Year " << dy << " is not implemented to set the bit for " << stest << "." << endl;
        assert(0);
      }
    }
  }
  if (ne!=ne_req){
    IVYerr << "TriggerHelpers::getTriggerObjectReqs: The trigger " << hltname << " is specified to require " << ne_req << " electrons, but only " << ne << " are found during the parsing of its name." << endl;
    assert(0);
  }

  // Photons
  if (npho_req>0){
    if (hltname.find("DoublePhoton")!=std::string::npos) npho = 2;
    else{
      std::regex rgx("(Photon[0-9]+)");
      auto it_begin = std::sregex_iterator(hltname.begin(), hltname.end(), rgx);
      auto it_end = std::sregex_iterator();
      for (auto it=it_begin; it!=it_end; it++){
        std::smatch sm = *it;
        std::string strmatch = sm.str();
        npho++;
      }
    }
  }
  // No matching requirement is sought for photons.
  // Why?
  // Because NanoAOD geniuses messed up photon trigger objects!
  // THERE ARE NONE! IT DOESN'T MATTER!
  // That is why a trigger object exception is assigned to all kSinglePho HLT path properties in configureHLTmap().
  if (npho!=npho_req){
    IVYerr << "TriggerHelpers::getTriggerObjectReqs: The trigger " << hltname << " is specified to require " << npho_req << " photons, but only " << npho << " are found during the parsing of its name." << endl;
    assert(0);
  }

  // Jets
  // While we do not match jets, we still have to count them because some masks have bits related to jets.
  // (What an idiotic arrangement of information...)
  if (hltname.find("DiPFJet")!=std::string::npos || hltname.find("DiJet")!=std::string::npos) nj = 2;
  else{
    std::regex rgx("(AK8)?(PFJet[0-9]+)");
    auto it_begin = std::sregex_iterator(hltname.begin(), hltname.end(), rgx);
    auto it_end = std::sregex_iterator();
    for (auto it=it_begin; it!=it_end; it++){
      std::smatch sm = *it;
      std::string strmatch = sm.str();
      if (strmatch.find("AK8")==std::string::npos) nj++;
    }
  }
  if (nj!=nj_req){
    IVYerr << "TriggerHelpers::getTriggerObjectReqs: The trigger " << hltname << " is specified to require " << nj_req << " PF jets, but only " << nj << " are found during the parsing of its name." << endl;
    assert(0);
  }

  // (Repeat: What an idiotic arrangement of information...)
  // Trilepton triggers
  if (nm==3){
    if (dy>2016) HelperFunctions::set_bit(mask_mu, 7);
  }
  else if (nm==2 && ne==1){
    if (dy>2016) HelperFunctions::set_bit(mask_mu, 8);
    HelperFunctions::set_bit(mask_e, 9);
  }
  else if (nm==1 && ne==2){
    if (dy>2016) HelperFunctions::set_bit(mask_mu, 9);
    HelperFunctions::set_bit(mask_e, 8);
  }
  else if (ne==3){
    HelperFunctions::set_bit(mask_e, 7);
  }
  // Dilepton triggers
  else if (nm==2){
    if (dy>2016) HelperFunctions::set_bit(mask_mu, 4);
  }
  else if (nm==1 && ne==1){
    if (dy>2016) HelperFunctions::set_bit(mask_mu, 5);
    HelperFunctions::set_bit(mask_e, 5);
  }
  else if (ne==2){
    HelperFunctions::set_bit(mask_e, 4);
  }
  // Single electron + jet triggers
  else if (ne==1 && nj==1){
    HelperFunctions::set_bit(mask_e, 12);
  }

  res.reserve((nm>0)+(ne>0)+(npho>0));
  if (nm>0) res.emplace_back(TriggerMuon, mask_mu);
  if (ne>0) res.emplace_back(TriggerElectron, mask_e);
  if (npho>0) res.emplace_back(TriggerPhoton, mask_pho);

  print_warnings = false;

  return res;
}

void TriggerHelpers::configureHLTmap(){
  if (!SampleHelpers::runConfigure){
    IVYerr << "TriggerHelpers::configureHLTmap: Need to call SampleHelpers::configure(period) first!" << endl;
    assert(0);
  }

  // Clear the maps
  HLT_type_list_map.clear();
  HLT_type_proplist_map.clear();

  // Clear the list of HLT properties with run range exclusions
  runRangeExcluded_HLTprop_list.clear();

  auto const& dy = SampleHelpers::getDataYear();

  // Notice that the triggers that require cuts are ORDERED!
  // Ordering within each list is important.
  // FIXME: Triggers with prescales need to include higher-pT thresholds. SinglePhoton is done, but the rest needs revision either.
  switch (dy){
  case 2016:
    HLT_type_proplist_map[kDoubleMu] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } } // Prescale=0 in 2016H and >0 in v2
    };
    HLT_type_proplist_map[kDoubleMu_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }, // HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v* is prescaled, do not use it!
      { "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }, // Prescale=0 in 2016H and >0 in v2
      { "HLT_Mu30_TkMu11_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Mu40_TkMu11_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } } // Probably no need for this
    };
    HLT_type_proplist_map[kDoubleMu_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kDoubleMu_PFHT] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoubleMu8_Mass8_PFHT300_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 300.f } } } } }
    };
    HLT_type_proplist_map[kDoubleEle] = std::vector<HLTTriggerPathProperties>{
      //{ "HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*", { { HLTObjectProperties::kElectron }, { HLTObjectProperties::kElectron } } }, // If uncommented, needs to also uncomment its run range exclusion below.
      { "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoubleEle33_CaloIdL_MW_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } },
      { "HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoublePhoton60_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_PFHT] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_PFHT300_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 300.f } } } } }
    };
    // These MuEle triggers are tricky to deal with because they are disabled for part of the run...
    HLT_type_proplist_map[kMuEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele8_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele8_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_TrkIsoVVL_Ele17_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu17_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu30_Ele30_CaloIdL_GsfTrkIdVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu33_Ele33_CaloIdL_GsfTrkIdVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_PFHT] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT300_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 300.f } } } } }
    };
    HLT_type_proplist_map[kSingleMu] = std::vector<HLTTriggerPathProperties>{
      { "HLT_IsoMu24_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_IsoTkMu24_v*",{ { HLTObjectProperties::kMuon } } }
    };
    // Prescales are 0 or 1, so ignore pT thresholds
    HLT_type_proplist_map[kSingleMu_Prescaled] = std::vector<HLTTriggerPathProperties>{
      { "HLT_IsoMu22_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_IsoTkMu22_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_IsoMu20_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_IsoTkMu20_v*",{ { HLTObjectProperties::kMuon } } }
    };
    // Prescales are 0 or 1, so ignore pT thresholds.
    HLT_type_proplist_map[kSingleMu_Eta2p1_Prescaled] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu45_eta2p1_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_IsoMu22_eta2p1_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_IsoTkMu22_eta2p1_v*",{ { HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kSingleMu_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_TkMu50_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_Mu50_v*",{ { HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kSingleMu_HighPt_Extra] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele25_eta2p1_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } },
      { "HLT_Ele27_eta2p1_WPLoose_Gsf_v*",{ { HLTObjectProperties::kElectron } } }, // Prescale>1 for 2016H, but we include here with a run range exclusion below
      { "HLT_Ele27_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } },
      { "HLT_Ele32_eta2p1_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kSingleEle_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_HighPt] = std::vector<HLTTriggerPathProperties>{ { "HLT_Photon175_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSingleEle_HighPt_Extra] = std::vector<HLTTriggerPathProperties>{ { "HLT_Ele115_CaloIdVT_GsfTrkIdT_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSingleEle_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>{ { "HLT_Ele105_CaloIdVT_GsfTrkIdT_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSinglePho] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Photon175_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 175.f+45.f } } } } }, // Unprescaled
      { "HLT_Photon165_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 165.f+35.f },{ HLTObjectProperties::kPtHigh, 175.f+45.f } } } } },
      { "HLT_Photon120_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 120.f+15.f },{ HLTObjectProperties::kPtHigh, 165.f+35.f } } } } },
      { "HLT_Photon90_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 90.f*1.1f },{ HLTObjectProperties::kPtHigh, 120.f+15.f } } } } },
      { "HLT_Photon75_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 75.f*1.1f },{ HLTObjectProperties::kPtHigh, 90.f*1.1f } } } } },
      { "HLT_Photon50_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 50.f*1.1f },{ HLTObjectProperties::kPtHigh, 75.f*1.1f } } } } }
    };
    HLT_type_proplist_map[kTripleLep] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_DiEle12_CaloIdL_TrackIdL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } },
      { "HLT_DiMu9_Ele9_CaloIdL_TrackIdL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_TripleMu_12_10_5_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kSingleMu_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu27_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 27.f } } } } }, // Has HLT prescales only
      { "HLT_Mu20_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 20.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu17_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Jet_Control_NoIso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_Jet_Control_Iso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele17_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 17.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele12_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 12.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    HLT_type_proplist_map[kSingleEle_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele17_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 17.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele12_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 12.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    HLT_type_proplist_map[kAK8PFJet_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_AK8PFJet360_TrimMass30_v*",{ { HLTObjectProperties::kAK8Jet,{ { HLTObjectProperties::kPt, 360.f },{ HLTObjectProperties::kMass, 30.f } } } } }
    };
    HLT_type_proplist_map[kVBFJets_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DiPFJet40_DEta3p5_MJJ600_PFMETNoMu140_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 40.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 40.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kEtaLow, 3.5f },{ HLTObjectProperties::kMass, 600.f } } },{ HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 140.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT900_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 1000.f } } } } }, // Prescaled
                                                                                                      //{ "HLT_PFHT800_v*", { { HLTObjectProperties::kHT, { { HLTObjectProperties::kPt, 800.f } } } } }, // Prescaled, but it doesn't exist in 2016H. Therefore, it is disabled.
      { "HLT_PFHT650_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 740.f },{ HLTObjectProperties::kPtHigh, 1000.f } } } } }, // Prescaled
      { "HLT_PFHT600_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 690.f },{ HLTObjectProperties::kPtHigh, 740.f } } } } }, // Prescaled
      { "HLT_PFHT475_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 600.f },{ HLTObjectProperties::kPtHigh, 690.f } } } } }, // Prescaled
      { "HLT_PFHT400_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 550.f },{ HLTObjectProperties::kPtHigh, 600.f } } } } }, // Prescaled
      { "HLT_PFHT350_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 520.f },{ HLTObjectProperties::kPtHigh, 550.f } } } } }, // Prescaled
      { "HLT_PFHT300_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kPtHigh, 520.f } } } } }, // Prescaled
      { "HLT_PFHT250_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 480.f },{ HLTObjectProperties::kPtHigh, 500.f } } } } }, // Prescaled
      { "HLT_PFHT200_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 440.f },{ HLTObjectProperties::kPtHigh, 480.f } } } } }, // Prescaled
      { "HLT_PFHT125_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 350.f },{ HLTObjectProperties::kPtHigh, 440.f } } } } } // Prescaled
    };
    HLT_type_proplist_map[kMET_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_MET600_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 600.f } } } } },
      { "HLT_MET300_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 300.f } } } } },
      { "HLT_MET250_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 250.f } } } } },
      { "HLT_MET200_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 200.f } } } } }
    };
    HLT_type_proplist_map[kPFMET_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMET600_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 600.f } } } } },
      { "HLT_PFMET500_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 500.f } } } } },
      { "HLT_PFMET400_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 400.f } } } } },
      { "HLT_PFMET300_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 300.f } } } } },
      { "HLT_PFMET170_HBHECleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 170.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT300_PFMET110_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 300.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 110.f } } } } }
    };
    HLT_type_proplist_map[kPFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_v*",{ { HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT_NoMu,{ { HLTObjectProperties::kMass, 120.f } } } } },
      { "HLT_PFMET120_PFMHT120_IDTight_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kMass, 120.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kAuxiliary] = std::vector<HLTTriggerPathProperties>{
      { "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }, // HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v* is prescaled, do not use it!
    };

    // Assign run range exclusions
    assignRunRangeExclusions(
      "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v*", {
        { 280919/*281613*/, -1 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v*", {
        { 280919/*281613*/, -1 }
      }
    );
    // HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v* is disabled because there is almost no gain from including it.
    //assignRunRangeExclusions(
    //  "HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*", {
    //    { 274968, -1 } // Prescales in 2016D-H are especially crazy, they interchange between 0 and 1.
    //  }
    //);
    assignRunRangeExclusions(
      "HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_v*", {
        { 278873, -1 }
      }
    );
    /*
    Note on HLT_DoubleEle33_CaloIdL_MW_v*:
    - Egamma POG recommends 276453 as the lower boundary, but the golden JSON has 276454 currently as the first run after that.
    They also mention this trigger 'has a bug in that only one of the electrons is required to pass the medium window (MW) pixel requirement before 276453', but
    since this will create a mismatch between data and MC, we exclude all runs before 276453 as well.
    - HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_v* and this trigger are able to cover all of Run 2016 for pT1,2>33 GeV since the first good run after 278822 is 278873.
    */
    assignRunRangeExclusions(
      "HLT_DoubleEle33_CaloIdL_MW_v*", {
        { -1/*276453*//*276454*/, 278822 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu8_TrkIsoVVL_Ele17_CaloIdL_TrackIdL_IsoVL_v*", {
        { 274968, -1 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v*", {
        { 280919/*281613*/, -1 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*", {
        { -1, 278240 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu17_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*", {
        { 274968, -1 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu23_TrkIsoVVL_Ele8_CaloIdL_TrackIdL_IsoVL_v*", {
        { 280919/*281613*/, -1 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu23_TrkIsoVVL_Ele8_CaloIdL_TrackIdL_IsoVL_DZ_v*", {
        { -1, 278240 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*", {
        { 280919/*281613*/, -1 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*", {
        { -1, 278240 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu30_Ele30_CaloIdL_GsfTrkIdVL_v*", {
        { 280919/*281613*/, -1 } // FIXME: Check the exact run exclusion range
      }
    );
    assignRunRangeExclusions(
      "HLT_TkMu50_v*", {
        { -1, 274442 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Ele27_eta2p1_WPLoose_Gsf_v*", {
        { 280919/*281613*/, -1 } // Prescale>1 for 2016H
      }
    );
    // HLT_PFHT800_v* seems disabled in 2016H. However, this would create a gap in the HT spectrum, so it is excluded from the set of control triggers altogether.
    assignRunRangeExclusions(
      "HLT_PFHT800_v*", {
        { 280919/*281613*/, -1 }
      }
    );
    break;
  case 2017:
    HLT_type_proplist_map[kDoubleMu] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kDoubleMu_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu37_TkMu27_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kDoubleMu_Prescaled] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f*1.1f } } },{ HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f*1.1f } } } } }
    };
    HLT_type_proplist_map[kDoubleMu_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kDoubleEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoubleEle33_CaloIdL_MW_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoublePhoton70_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kMuEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu27_Ele37_CaloIdL_MW_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu37_Ele27_CaloIdL_MW_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu] = std::vector<HLTTriggerPathProperties>{ { "HLT_IsoMu27_v*",{ { HLTObjectProperties::kMuon } } } };
    // HLT_IsoMu20_v* have L1 and HLT prescales
    // HLT_IsoMu24_v* is disabled for a large portion of 2017, so omit it.
    HLT_type_proplist_map[kSingleMu_Prescaled] = std::vector<HLTTriggerPathProperties>{
      //{ "HLT_IsoMu24_v*", { { HLTObjectProperties::kMuon, { { HLTObjectProperties::kPt, 24.f*1.1f } } } } },
      //{ "HLT_IsoMu20_v*", { { HLTObjectProperties::kMuon, { { HLTObjectProperties::kPt, 20.f*1.1f }, { HLTObjectProperties::kPtHigh, 24.f*1.1f } } } } }
      { "HLT_IsoMu20_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 20.f*1.1f } } } } }
    };
    // HLT_IsoMu24_eta2p1_v* is disabled for a large portion of 2017, so omit it.
    //HLT_type_proplist_map[kSingleMu_Eta2p1_Prescaled] = std::vector<HLTTriggerPathProperties>{ { "HLT_IsoMu24_eta2p1_v*",{ { HLTObjectProperties::kMuon } } } };
    HLT_type_proplist_map[kSingleMu_Eta2p1_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_TkMu100_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_OldMu100_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_Mu50_v*",{ { HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kSingleMu_HighPt_Extra] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele32_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } }, // Note: Some lumi sections might have prescale=0.
      { "HLT_Ele35_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } }, // Note: Some lumi sections might have prescale=0 or >1.
      { "HLT_Ele38_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } },
      { "HLT_Ele40_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kSingleEle_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_HighPt] = std::vector<HLTTriggerPathProperties>{ { "HLT_Photon200_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSingleEle_HighPt_Extra] = std::vector<HLTTriggerPathProperties>(); // HLT_Ele115_CaloIdVT_GsfTrkIdT_v* is prescaled for most of the run, so ignore.
    HLT_type_proplist_map[kSingleEle_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSinglePho] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Photon200_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 200.f*1.15f } } } } }, // Unprescaled
      { "HLT_Photon165_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 165.f*1.1f },{ HLTObjectProperties::kPtHigh, 200.f*1.15f } } } } },
      { "HLT_Photon120_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 120.f*1.1f },{ HLTObjectProperties::kPtHigh, 165.f*1.1f } } } } },
      { "HLT_Photon90_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 90.f*1.1f },{ HLTObjectProperties::kPtHigh, 120.f*1.1f } } } } },
      { "HLT_Photon75_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 75.f*1.1f },{ HLTObjectProperties::kPtHigh, 90.f*1.1f } } } } },
      { "HLT_Photon50_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 50.f*1.1f },{ HLTObjectProperties::kPtHigh, 75.f*1.1f } } } } }
    };
    HLT_type_proplist_map[kTripleLep] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } },
      { "HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_TripleMu_10_5_5_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_TripleMu_12_10_5_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kSingleMu_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu27_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 27.f } } } } }, // Has HLT prescales only
      { "HLT_Mu20_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 20.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu17_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Jet_Control_NoIso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_Jet_Control_Iso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele17_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 17.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    HLT_type_proplist_map[kSingleEle_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele12_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 12.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    // HLT_AK8PFJet360_TrimMass30_v* is disabled for most of 2017, so omit it altogether.
    HLT_type_proplist_map[kAK8PFJet_Control] = std::vector<HLTTriggerPathProperties>{
      //{ "HLT_AK8PFJet360_TrimMass30_v*", { { HLTObjectProperties::kAK8Jet, { { HLTObjectProperties::kPt, 360.f }, { HLTObjectProperties::kMass, 30.f } } } } }
    };
    HLT_type_proplist_map[kVBFJets_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DiJet110_35_Mjj650_PFMET110_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 110.f } } } } },
      { "HLT_DiJet110_35_Mjj650_PFMET120_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } } } },
      { "HLT_DiJet110_35_Mjj650_PFMET130_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 130.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT1050_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 1150.f } } } } },
      { "HLT_PFHT890_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 1000.f },{ HLTObjectProperties::kPtHigh, 1150.f } } } } },
      { "HLT_PFHT780_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 900.f },{ HLTObjectProperties::kPtHigh, 1000.f } } } } }, // Prescaled
      { "HLT_PFHT680_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kPtHigh, 900.f } } } } }, // Prescaled
      { "HLT_PFHT590_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 750.f },{ HLTObjectProperties::kPtHigh, 800.f } } } } }, // Prescaled
      { "HLT_PFHT510_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kPtHigh, 750.f } } } } }, // Prescaled
      { "HLT_PFHT430_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 680.f },{ HLTObjectProperties::kPtHigh, 700.f } } } } }, // Prescaled
      { "HLT_PFHT370_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 600.f },{ HLTObjectProperties::kPtHigh, 680.f } } } } }, // Prescaled
      { "HLT_PFHT350_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 580.f },{ HLTObjectProperties::kPtHigh, 600.f } } } } }, // Prescaled
      { "HLT_PFHT250_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 550.f },{ HLTObjectProperties::kPtHigh, 580.f } } } } }, // Prescaled
      { "HLT_PFHT180_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kPtHigh, 550.f } } } } } // Prescaled
    };
    HLT_type_proplist_map[kMET_Control] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kPFMET_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMET300_HBHECleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 300.f } } } } },
      { "HLT_PFMET250_HBHECleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 250.f } } } } },
      { "HLT_PFMET200_HBHE_BeamHaloCleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 200.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_Control] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kPFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*",{ { HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT_NoMu,{ { HLTObjectProperties::kMass, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 60.f } } } } },
      { "HLT_PFMET120_PFMHT120_IDTight_PFHT60_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kMass, 120.f },{ HLTObjectProperties::kPt, 60.f } } } } },
      { "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_v*",{ { HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT_NoMu,{ { HLTObjectProperties::kMass, 120.f } } } } },
      { "HLT_PFMET120_PFMHT120_IDTight_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kMass, 120.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT500_PFMET100_PFMHT100_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kMass, 100.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 100.f } } } } },
      { "HLT_PFHT500_PFMET110_PFMHT110_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kMass, 110.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 110.f } } } } },
      { "HLT_PFHT700_PFMET85_PFMHT85_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kMass, 85.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 85.f } } } } },
      { "HLT_PFHT700_PFMET95_PFMHT95_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kMass, 95.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 95.f } } } } },
      { "HLT_PFHT800_PFMET75_PFMHT75_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kMass, 75.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 75.f } } } } },
      { "HLT_PFHT800_PFMET85_PFMHT85_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kMass, 85.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 85.f } } } } }
    };
    HLT_type_proplist_map[kAuxiliary] = std::vector<HLTTriggerPathProperties>{
      { "HLT_IsoMu24_eta2p1_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_IsoMu24_v*", { { HLTObjectProperties::kMuon } } },
      { "HLT_Ele115_CaloIdVT_GsfTrkIdT_v*",{ { HLTObjectProperties::kElectron } } },
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_TripleMu_5_3_3_Mass3p8to60_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }
    };

    assignRunRangeExclusions(
      "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v*", {
        { 303824/*303832*/, -1/*306462*//*306456*/ } // The actual boundary read off is 303832-306456, which is a large subset of 2017E+F. We exclude entire 2017E+F.
      }
    );
    // These triggers are replaced by the HT60 versions after 2017B.
    assignRunRangeExclusions(
      "HLT_PFMET120_PFMHT120_IDTight_v*", {
        { 305586, -1 }
      }
    );
    assignRunRangeExclusions(
      "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_v*", {
        { 305586, -1 }
      }
    );
    // These triggers need to exclude 2017B because prescale=0.
    assignRunRangeExclusions(
      "HLT_PFMET250_HBHECleaned_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_PFMET300_HBHECleaned_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_PFMET120_PFMHT120_IDTight_PFHT60_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_DiJet110_35_Mjj650_PFMET110_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_DiJet110_35_Mjj650_PFMET120_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_DiJet110_35_Mjj650_PFMET130_v*", {
        { -1, 299329 }
      }
    );
    // Mu50 is unprescaled for the entire run, but TkMu100 is not.
    assignRunRangeExclusions(
      "HLT_TkMu100_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_OldMu100_v*", {
        { -1, 299329 }
      }
    );
    // HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v* is unprescaled in 2017B.
    // We don't need to use it explicitly as a replacement for HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v*
    // because we also have HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8_v*, which is fully unprescaled.
    //assignRunRangeExclusions(
    //  "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*", {
    //    { 299368, -1 }
    //  }
    //);
    assignRunRangeExclusions(
      "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*", {
        { -1, 299329 }
      }
    );
    assignRunRangeExclusions(
      "HLT_Mu37_TkMu27_v*", {
        { -1, 302029/*302019*/ } // No prescale=0 in the last few valid runs of 2017C, but we will ignore that and exclude all of 2017B+C.
      }
    );
    assignRunRangeExclusions(
      "HLT_PFMET200_HBHE_BeamHaloCleaned_v*", {
        { -1, 304797 }
      }
    );
    break;
  case 2018:
    HLT_type_proplist_map[kDoubleMu] = std::vector<HLTTriggerPathProperties>{
      // No gain from adding the Mass8 version of the one below
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kDoubleMu_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu37_TkMu27_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kDoubleMu_Prescaled] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f*1.1f } } },{ HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f*1.1f } } } } }
    };
    HLT_type_proplist_map[kDoubleMu_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kDoubleEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoubleEle25_CaloIdL_MW_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoublePhoton70_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kMuEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu27_Ele37_CaloIdL_MW_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu37_Ele27_CaloIdL_MW_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu] = std::vector<HLTTriggerPathProperties>{ { "HLT_IsoMu24_v*",{ { HLTObjectProperties::kMuon } } } };
    HLT_type_proplist_map[kSingleMu_Prescaled] = std::vector<HLTTriggerPathProperties>{ { "HLT_IsoMu20_v*",{ { HLTObjectProperties::kMuon } } } };
    HLT_type_proplist_map[kSingleMu_Eta2p1_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_TkMu100_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_OldMu100_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_Mu50_v*",{ { HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kSingleMu_HighPt_Extra] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle] = std::vector<HLTTriggerPathProperties>{ { "HLT_Ele32_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSingleEle_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_HighPt] = std::vector<HLTTriggerPathProperties>{ { "HLT_Photon200_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSingleEle_HighPt_Extra] = std::vector<HLTTriggerPathProperties>(); // HLT_Ele115_CaloIdVT_GsfTrkIdT_v* is prescaled for most of the run, so ignore.
    HLT_type_proplist_map[kSingleEle_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSinglePho] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Photon200_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 200.f*1.15f } } } } }, // Unprescaled
      { "HLT_Photon165_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 165.f*1.1f },{ HLTObjectProperties::kPtHigh, 200.f*1.15f } } } } },
      { "HLT_Photon120_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 120.f*1.1f },{ HLTObjectProperties::kPtHigh, 165.f*1.1f } } } } },
      { "HLT_Photon90_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 90.f*1.1f },{ HLTObjectProperties::kPtHigh, 120.f*1.1f } } } } },
      { "HLT_Photon75_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 90.f },{ HLTObjectProperties::kPtHigh, 90.f*1.1f } } } } },
      { "HLT_Photon50_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 50.f*1.1f },{ HLTObjectProperties::kPtHigh, 90.f } } } } }
    };
    HLT_type_proplist_map[kTripleLep] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } },
      { "HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_TripleMu_10_5_5_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_TripleMu_12_10_5_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }//,
      //{ "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v*", { { HLTObjectProperties::kElectron }, { HLTObjectProperties::kElectron }, { HLTObjectProperties::kElectron } } } // Somehow its effective lumi is not the same as active lumi, and it does not show up in the spreadsheets
    };
    HLT_type_proplist_map[kSingleMu_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu27_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 27.f } } } } }, // Has HLT prescales only
      { "HLT_Mu20_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 20.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu17_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Jet_Control_NoIso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_Jet_Control_Iso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele17_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 17.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    HLT_type_proplist_map[kSingleEle_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele12_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 12.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    HLT_type_proplist_map[kAK8PFJet_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_AK8PFJet400_TrimMass30_v*",{ { HLTObjectProperties::kAK8Jet,{ { HLTObjectProperties::kPt, 400.f },{ HLTObjectProperties::kMass, 30.f } } } } }
    };
    HLT_type_proplist_map[kVBFJets_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DiJet110_35_Mjj650_PFMET110_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 110.f } } } } },
      { "HLT_DiJet110_35_Mjj650_PFMET120_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } } } },
      { "HLT_DiJet110_35_Mjj650_PFMET130_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 130.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT1050_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 1150.f } } } } },
      { "HLT_PFHT890_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 1000.f },{ HLTObjectProperties::kPtHigh, 1150.f } } } } },
      { "HLT_PFHT780_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 900.f },{ HLTObjectProperties::kPtHigh, 1000.f } } } } }, // Prescaled
      { "HLT_PFHT680_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kPtHigh, 900.f } } } } }, // Prescaled
      { "HLT_PFHT590_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 750.f },{ HLTObjectProperties::kPtHigh, 800.f } } } } }, // Prescaled
      { "HLT_PFHT510_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kPtHigh, 750.f } } } } }, // Prescaled
      { "HLT_PFHT430_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 680.f },{ HLTObjectProperties::kPtHigh, 700.f } } } } }, // Prescaled
      { "HLT_PFHT370_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 600.f },{ HLTObjectProperties::kPtHigh, 680.f } } } } }, // Prescaled
      { "HLT_PFHT350_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 580.f },{ HLTObjectProperties::kPtHigh, 600.f } } } } }, // Prescaled
      { "HLT_PFHT250_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 550.f },{ HLTObjectProperties::kPtHigh, 580.f } } } } }, // Prescaled
      { "HLT_PFHT180_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kPtHigh, 550.f } } } } } // Prescaled
    };
    HLT_type_proplist_map[kMET_Control] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kPFMET_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMET300_HBHECleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 300.f } } } } },
      { "HLT_PFMET250_HBHECleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 250.f } } } } },
      { "HLT_PFMET200_HBHE_BeamHaloCleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 200.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_Control] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kPFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*",{ { HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT_NoMu,{ { HLTObjectProperties::kMass, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 60.f } } } } },
      { "HLT_PFMET120_PFMHT120_IDTight_PFHT60_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kMass, 120.f },{ HLTObjectProperties::kPt, 60.f } } } } },
      { "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_v*",{ { HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT_NoMu,{ { HLTObjectProperties::kMass, 120.f } } } } },
      { "HLT_PFMET120_PFMHT120_IDTight_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kMass, 120.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT500_PFMET100_PFMHT100_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kMass, 100.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 100.f } } } } },
      { "HLT_PFHT500_PFMET110_PFMHT110_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kMass, 110.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 110.f } } } } },
      { "HLT_PFHT700_PFMET85_PFMHT85_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kMass, 85.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 85.f } } } } },
      { "HLT_PFHT700_PFMET95_PFMHT95_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kMass, 95.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 95.f } } } } },
      { "HLT_PFHT800_PFMET75_PFMHT75_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kMass, 75.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 75.f } } } } },
      { "HLT_PFHT800_PFMET85_PFMHT85_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kMass, 85.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 85.f } } } } }
    };
    HLT_type_proplist_map[kAuxiliary] = std::vector<HLTTriggerPathProperties>{
      { "HLT_IsoMu27_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_Ele115_CaloIdVT_GsfTrkIdT_v*",{ { HLTObjectProperties::kElectron } } },
      { "HLT_TripleMu_5_3_3_Mass3p8_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v*", { { HLTObjectProperties::kElectron }, { HLTObjectProperties::kElectron }, { HLTObjectProperties::kElectron } } }
    };

    // Prescale=0 for a small portion of 2018A
    assignRunRangeExclusions(
      "HLT_DiJet110_35_Mjj650_PFMET110_v*", {
        { 316153, 316239 }
      }
    );
    assignRunRangeExclusions(
      "HLT_DiJet110_35_Mjj650_PFMET120_v*", {
        { 316153, 316239 }
      }
    );
    assignRunRangeExclusions(
      "HLT_DiJet110_35_Mjj650_PFMET130_v*", {
        { 316153, 316239 }
      }
    );

    break;
  case 2022:
    HLT_type_proplist_map[kDoubleMu] = std::vector<HLTTriggerPathProperties>{
      // No gain from adding the Mass8 version of the one below
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kDoubleMu_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu37_TkMu27_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kDoubleMu_Prescaled] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f*1.1f } } },{ HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f*1.1f } } } } }
    };
    HLT_type_proplist_map[kDoubleMu_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kDoubleEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoubleEle25_CaloIdL_MW_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DoublePhoton70_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kDoubleEle_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kMuEle] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_Extra] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu27_Ele37_CaloIdL_MW_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_Mu37_Ele27_CaloIdL_MW_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } }
    };
    HLT_type_proplist_map[kMuEle_PFHT] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu] = std::vector<HLTTriggerPathProperties>{ { "HLT_IsoMu24_v*",{ { HLTObjectProperties::kMuon } } } };
    HLT_type_proplist_map[kSingleMu_Prescaled] = std::vector<HLTTriggerPathProperties>{ { "HLT_IsoMu20_v*",{ { HLTObjectProperties::kMuon } } } };
    HLT_type_proplist_map[kSingleMu_Eta2p1_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_HighPt] = std::vector<HLTTriggerPathProperties>{
      { "HLT_HighPtTkMu100_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_Mu50_v*",{ { HLTObjectProperties::kMuon } } }
    };
    HLT_type_proplist_map[kSingleMu_HighPt_Extra] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle] = std::vector<HLTTriggerPathProperties>{ { "HLT_Ele32_WPTight_Gsf_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSingleEle_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_HighPt] = std::vector<HLTTriggerPathProperties>{ { "HLT_Photon200_v*",{ { HLTObjectProperties::kElectron } } } };
    HLT_type_proplist_map[kSingleEle_HighPt_Extra] = std::vector<HLTTriggerPathProperties>(); // HLT_Ele115_CaloIdVT_GsfTrkIdT_v* is prescaled for most of the run, so ignore.
    HLT_type_proplist_map[kSingleEle_HighPt_Extra_Prescaled] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSinglePho] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Photon200_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 200.f*1.15f } } } } }, // Unprescaled
      { "HLT_Photon165_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 165.f*1.1f },{ HLTObjectProperties::kPtHigh, 200.f*1.15f } } } } },
      { "HLT_Photon120_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 120.f*1.1f },{ HLTObjectProperties::kPtHigh, 165.f*1.1f } } } } },
      { "HLT_Photon90_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 90.f*1.1f },{ HLTObjectProperties::kPtHigh, 120.f*1.1f } } } } },
      { "HLT_Photon75_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 90.f },{ HLTObjectProperties::kPtHigh, 90.f*1.1f } } } } },
      { "HLT_Photon50_R9Id90_HE10_IsoM_v*",{ { HLTObjectProperties::kPhoton,{ { HLTObjectProperties::kPt, 50.f*1.1f },{ HLTObjectProperties::kPtHigh, 90.f } } } } }
    };
    HLT_type_proplist_map[kTripleLep] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } },
      { "HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kElectron } } },
      { "HLT_TripleMu_10_5_5_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_TripleMu_12_10_5_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } }//,
      //{ "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v*", { { HLTObjectProperties::kElectron }, { HLTObjectProperties::kElectron }, { HLTObjectProperties::kElectron } } } // Somehow its effective lumi is not the same as active lumi, and it does not show up in the spreadsheets
    };
    HLT_type_proplist_map[kSingleMu_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu27_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 27.f } } } } }, // Has HLT prescales only
      { "HLT_Mu20_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 20.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu17_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Mu17_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 17.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Mu8_TrkIsoVVL_v*",{ { HLTObjectProperties::kMuon,{ { HLTObjectProperties::kPt, 8.f } } } } } // Has L1 and HLT prescales
    };
    HLT_type_proplist_map[kSingleMu_Jet_Control_NoIso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleMu_Jet_Control_Iso] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kSingleEle_Control_NoIso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele17_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 17.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdM_TrackIdM_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    HLT_type_proplist_map[kSingleEle_Control_Iso] = std::vector<HLTTriggerPathProperties>{
      { "HLT_Ele23_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 23.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele12_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 12.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } }, // Has L1 and HLT prescales
      { "HLT_Ele8_CaloIdL_TrackIdL_IsoVL_PFJet30_v*",{ { HLTObjectProperties::kElectron,{ { HLTObjectProperties::kPt, 8.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 30.f } } } } } // Has L1 prescales
    };
    HLT_type_proplist_map[kAK8PFJet_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_AK8PFJet400_TrimMass30_v*",{ { HLTObjectProperties::kAK8Jet,{ { HLTObjectProperties::kPt, 400.f },{ HLTObjectProperties::kMass, 30.f } } } } }
    };
    HLT_type_proplist_map[kVBFJets_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_DiJet110_35_Mjj650_PFMET110_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 110.f } } } } },
      { "HLT_DiJet110_35_Mjj650_PFMET120_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } } } },
      { "HLT_DiJet110_35_Mjj650_PFMET130_v*",{ { HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 110.f } } },{ HLTObjectProperties::kAK4Jet,{ { HLTObjectProperties::kPt, 35.f } } },{ HLTObjectProperties::kAK4DiJetSumWithDEtaDPhi,{ { HLTObjectProperties::kMass, 650.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 130.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT1050_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 1150.f } } } } },
      { "HLT_PFHT890_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 1000.f },{ HLTObjectProperties::kPtHigh, 1150.f } } } } },
      { "HLT_PFHT780_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 900.f },{ HLTObjectProperties::kPtHigh, 1000.f } } } } }, // Prescaled
      { "HLT_PFHT680_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kPtHigh, 900.f } } } } }, // Prescaled
      { "HLT_PFHT590_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 750.f },{ HLTObjectProperties::kPtHigh, 800.f } } } } }, // Prescaled
      { "HLT_PFHT510_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kPtHigh, 750.f } } } } }, // Prescaled
      { "HLT_PFHT430_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 680.f },{ HLTObjectProperties::kPtHigh, 700.f } } } } }, // Prescaled
      { "HLT_PFHT370_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 600.f },{ HLTObjectProperties::kPtHigh, 680.f } } } } }, // Prescaled
      { "HLT_PFHT350_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 580.f },{ HLTObjectProperties::kPtHigh, 600.f } } } } }, // Prescaled
      { "HLT_PFHT250_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 550.f },{ HLTObjectProperties::kPtHigh, 580.f } } } } }, // Prescaled
      { "HLT_PFHT180_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kPtHigh, 550.f } } } } } // Prescaled
    };
    HLT_type_proplist_map[kMET_Control] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kPFMET_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMET300_HBHECleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 300.f } } } } },
      { "HLT_PFMET250_HBHECleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 250.f } } } } },
      { "HLT_PFMET200_HBHE_BeamHaloCleaned_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 200.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_Control] = std::vector<HLTTriggerPathProperties>();
    HLT_type_proplist_map[kPFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*",{ { HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT_NoMu,{ { HLTObjectProperties::kMass, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 60.f } } } } },
      { "HLT_PFMET120_PFMHT120_IDTight_PFHT60_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kMass, 120.f },{ HLTObjectProperties::kPt, 60.f } } } } },
      { "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_v*",{ { HLTObjectProperties::kMET_NoMu,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT_NoMu,{ { HLTObjectProperties::kMass, 120.f } } } } },
      { "HLT_PFMET120_PFMHT120_IDTight_v*",{ { HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 120.f } } },{ HLTObjectProperties::kHT,{ { HLTObjectProperties::kMass, 120.f } } } } }
    };
    HLT_type_proplist_map[kPFHT_PFMET_MHT_Control] = std::vector<HLTTriggerPathProperties>{
      { "HLT_PFHT500_PFMET100_PFMHT100_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kMass, 100.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 100.f } } } } },
      { "HLT_PFHT500_PFMET110_PFMHT110_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 500.f },{ HLTObjectProperties::kMass, 110.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 110.f } } } } },
      { "HLT_PFHT700_PFMET85_PFMHT85_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kMass, 85.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 85.f } } } } },
      { "HLT_PFHT700_PFMET95_PFMHT95_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 700.f },{ HLTObjectProperties::kMass, 95.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 95.f } } } } },
      { "HLT_PFHT800_PFMET75_PFMHT75_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kMass, 75.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 75.f } } } } },
      { "HLT_PFHT800_PFMET85_PFMHT85_IDTight_v*",{ { HLTObjectProperties::kHT,{ { HLTObjectProperties::kPt, 800.f },{ HLTObjectProperties::kMass, 85.f } } },{ HLTObjectProperties::kMET,{ { HLTObjectProperties::kPt, 85.f } } } } }
    };
    HLT_type_proplist_map[kAuxiliary] = std::vector<HLTTriggerPathProperties>{
      { "HLT_IsoMu27_v*",{ { HLTObjectProperties::kMuon } } },
      { "HLT_Ele115_CaloIdVT_GsfTrkIdT_v*",{ { HLTObjectProperties::kElectron } } },
      { "HLT_TripleMu_5_3_3_Mass3p8_DZ_v*",{ { HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon },{ HLTObjectProperties::kMuon } } },
      { "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v*",{ { HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron },{ HLTObjectProperties::kElectron } } }
    };

    break;
  default:
    break;
  }

  // Check that all triggers are defined
  for (int itt=0; itt!=(int) nTriggerTypes; itt++){
    if (HLT_type_proplist_map.find((TriggerType) itt)==HLT_type_proplist_map.cend()){
      IVYerr << "TriggerHelpers::configureHLTmap: Triggers for type " << itt << " are not defined for year " << SampleHelpers::getDataYear() << ". Please fix the map HLT_type_proplist_map." << endl;
      assert(0);
    }
  }

  // NANOAOD WORKAROUND:
  // Assign the no-photon trigger object exception to all single photon triggers unanimously.
  // Why?
  // BECAUSE NANOAOD EXPERTS (!) MESSED UP RECORDING PHOTON TRIGGER OBJECTS!
  // (Have I ever expressed my instense dislike toward NanoAOD yet?)
  for (auto const& hltprop:HLT_type_proplist_map.find(kSinglePho)->second) assignTriggerObjectCheckException((hltprop.getName()+"*"), HLTTriggerPathProperties::toNoPhotonTriggerObjects);
  // For the DoublePhoton trigger (for high-pT dielectrons), there is no corresponding trigger bit for electrons, so ignore matching for this trigger.
  for (auto const& hltprop:HLT_type_proplist_map.find(kDoubleEle_HighPt)->second) assignTriggerObjectCheckException((hltprop.getName()+"*"), HLTTriggerPathProperties::toNoElectronTriggerObjects);

  // Fill the trigger object matching requirements.
  // Fill the name map as well for simpler checking functionality.
  for (auto const& it:HLT_type_proplist_map){
    auto const& props = it.second;
    std::vector<std::string> tmplist; tmplist.reserve(props.size());
    for (auto const& prop:props){
      auto const* prop_ptr = &prop;
      HLT_prop_TOreqs_map[prop_ptr] = getTriggerObjectReqs(prop);

      tmplist.push_back(prop.getName());
    }
    HLT_type_list_map[it.first] = tmplist;
  }
}
