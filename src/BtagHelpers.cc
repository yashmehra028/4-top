#include <cassert>
#include "BtagHelpers.h"
#include "SamplesCore.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


namespace BtagHelpers{
  std::unordered_map<BtagWPType, float> btagwptype_btagwp_map;
}

// All of the WPs comes from https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation.
void BtagHelpers::configureBtagWPs(){
  if (!SampleHelpers::runConfigure){
    IVYerr << "BtagHelpers::configureBtagWPs: SampleHelpers::configure needs to be called first." << endl;
    assert(0);
  }

  auto const& dp = SampleHelpers::getDataPeriod();
  auto const& dy = SampleHelpers::getDataYear();
  bool const isAPV2016Affected = SampleHelpers::isAPV2016Affected(dp);

  if (dy==2016){
    if (isAPV2016Affected){
      btagwptype_btagwp_map = std::unordered_map<BtagWPType, float>{
        { kDeepFlav_Loose, 0.0508 },
        { kDeepFlav_Medium, 0.2598 },
        { kDeepFlav_Tight, 0.6502 }
      };
    }
    else{
      btagwptype_btagwp_map = std::unordered_map<BtagWPType, float>{
        { kDeepFlav_Loose, 0.0480 },
        { kDeepFlav_Medium, 0.2489 },
        { kDeepFlav_Tight, 0.6377 }
      };
    }
  }
  else if (dy==2017){
    btagwptype_btagwp_map = std::unordered_map<BtagWPType, float>{
      { kDeepFlav_Loose, 0.0532 },
      { kDeepFlav_Medium, 0.3040 },
      { kDeepFlav_Tight, 0.7476 }
    };
  }
  else if (dy==2018){
    btagwptype_btagwp_map = std::unordered_map<BtagWPType, float>{
      { kDeepFlav_Loose, 0.0490 },
      { kDeepFlav_Medium, 0.2783 },
      { kDeepFlav_Tight, 0.7100 }
    };
  }
  else{
    IVYerr << "BtagHelpers::configureBtagWPs: Data period " << dp << " is not defined." << endl;
    assert(0);
  }
}
float const& BtagHelpers::getBtagWP(BtagHelpers::BtagWPType type){
  auto it = btagwptype_btagwp_map.find(type);
  if (it == btagwptype_btagwp_map.cend()){
    IVYerr << "BtagHelpers::getBtagWP: WP for WP type  " << type << " is not found." << endl;
    assert(0);
  }
  return it->second;
}
std::vector<float> BtagHelpers::getBtagWPs(BtagHelpers::BtagWPType type){
  std::vector<float> res; res.reserve(3);
  switch (type){
    /*
  case kDeepCSV_Loose:
  case kDeepCSV_Medium:
  case kDeepCSV_Tight:
    res.push_back(getBtagWP(kDeepCSV_Loose));
    res.push_back(getBtagWP(kDeepCSV_Medium));
    res.push_back(getBtagWP(kDeepCSV_Tight));
    break;
    */
  case kDeepFlav_Loose:
  case kDeepFlav_Medium:
  case kDeepFlav_Tight:
    res.push_back(getBtagWP(kDeepFlav_Loose));
    res.push_back(getBtagWP(kDeepFlav_Medium));
    res.push_back(getBtagWP(kDeepFlav_Tight));
    break;
  default:
    IVYerr << "BtagHelpers::getBtagWPs: No implementation for the b-tagging WP type " << type << ". Aborting..." << endl;
    assert(0);
  }
  return res;
}