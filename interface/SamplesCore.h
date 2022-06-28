#ifndef SAMPLES_CORE_H
#define SAMPLES_CORE_H

#include <string>
#include <vector>
#include <utility>
#include "IvyFramework/IvyDataTools/interface/SampleHelpersCore.h"


const TString ANALYSISPKGPATH = "${CMSSW_BASE}/src/tttt/";
const TString ANALYSISPKGDATAPATH = ANALYSISPKGPATH + "data/";

// Cross section scale for the MC
constexpr float xsecScale = 1e3;


namespace SampleHelpers{
  extern bool runConfigure;

  extern int theDataYear;
  extern TString theDataPeriod;
  extern TString theInputDirectory;

  void setDataPeriod(TString s);
  void setInputDirectory(TString s);

  void configure(TString period);

  int const& getDataYear();
  TString const& getDataPeriod();
  TString const& getInputDirectory();
  TString getSqrtsString();

  int getDataYearFromPeriod(TString const& period);

  TString getDataPeriodFromRunNumber(unsigned int run);
  std::vector< std::pair<unsigned int, unsigned int> > getRunRangesFromDataPeriod(TString const& period);
  std::vector< std::pair<unsigned int, double> > const& getRunNumberLumiPairsForDataPeriod(TString const& period);
  bool isAPV2016Affected(unsigned int run);
  bool isHEM2018Affected(unsigned int run);
  std::vector<TString> getValidDataPeriods();
  bool testDataPeriodIsLikeData(TString const& period);
  bool testDataPeriodIsLikeData();
  double getIntegratedLuminosity(TString const& period);

  std::string getDatasetCoreName(std::string sname);

  TString getSampleIdentifier(TString const& strinput);
  bool checkSampleIsData(TString const& strid, TString* theSampleDataPeriod=nullptr);
  bool checkSampleIsFastSim(TString const& strid);

  TString getRandomDataPeriod(unsigned long long const& iseed, double* rndnum_global=nullptr, double* rndnum_local=nullptr);
  int translateRandomNumberToRunNumber(TString const& period, double const& rndnum);

  bool checkRunOnCondor();
  void addToCondorTransferList(TString const& fname);
  void addToCondorCompressedTransferList(TString const& dirname);

}

#endif
