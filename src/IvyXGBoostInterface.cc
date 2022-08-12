#include <limits>
#include "IvyFramework/IvyDataTools/interface/HostHelpersCore.h"
#include "IvyXGBoostInterface.hpp"


IvyXGBoostInterface::IvyXGBoostInterface() :
  IvyMLWrapper(),
  booster(nullptr),
  nColumns(0),
  nRows(0),
  defval(0)
{}

IvyXGBoostInterface::~IvyXGBoostInterface(){
  SAFE_XGBOOST(XGBoosterFree(*booster));
  delete booster;
}

bool IvyXGBoostInterface::build(TString fname, std::vector<TString> const& varnames, IvyMLDataType_t missing_entry_val, unsigned long long nCols){
  HostHelpers::ExpandEnvironmentVariables(fname);
  if (!HostHelpers::FileExists(fname)){
    IVYerr << "IvyXGBoostInterface::build: File " << fname << " does not exist." << endl;
    assert(0);
  }

  if (booster){
    IVYerr << "IvyXGBoostInterface::build: Booster is already built." << endl;
    return false;
  }
  if (nCols==0){
    IVYerr << "IvyXGBoostInterface::build: Number of columns (" << nCols << ") is invalid." << endl;
    return false;
  }
  if (varnames.size()%nCols != 0){
    IVYerr << "IvyXGBoostInterface::build: Number of variables (" << varnames.size() << ") is not divisible by the number of columns (" << nCols << ")." << endl;
    return false;
  }

  defval = missing_entry_val;
  nColumns = nCols;
  nRows = varnames.size()/nColumns;

  variable_names = varnames;
  for (auto const& vv:variable_names) variables[vv] = defval;

  booster = new BoosterHandle;
  SAFE_XGBOOST(XGBoosterCreate(nullptr, 0, booster));

  IVYout << "IvyXGBoostInterface::build: A new xgboost booster is created. Loading the model in " << fname << "..." << endl;

  SAFE_XGBOOST(XGBoosterLoadModel(*booster, fname.Data()));

  return true;
}
