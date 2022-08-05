#include <cassert>
#include <limits>
#include "IvyFramework/IvyDataTools/interface/HostHelpersCore.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "XGBoostInterface.h"


using namespace std;
using namespace IvyStreamHelpers;


#define SAFE_XGBOOST(CALL) \
{ int err_call = (CALL); if (err_call!=0){ IVYerr << "Call '" << #CALL << "' return error code " << err_call << ". XGBoost last error: " << XGBGetLastError() << endl; } }


XGBoostInterface::XGBoostInterface() :
  IvyMLWrapper(),
  booster(nullptr),
  nColumns(0),
  nRows(0),
  defval(0)
{}

XGBoostInterface::~XGBoostInterface(){
  SAFE_XGBOOST(XGBoosterFree(*booster));
  delete booster;
}

bool XGBoostInterface::build(TString fname, std::vector<TString> const& varnames, IvyMLDataType_t missing_entry_val, unsigned long long nCols){
  HostHelpers::ExpandEnvironmentVariables(fname);
  if (!HostHelpers::FileExists(fname)){
    IVYerr << "XGBoostInterface::build: File " << fname << " does not exist." << endl;
    assert(0);
  }

  if (booster){
    IVYerr << "XGBoostInterface::build: Booster is already built." << endl;
    return false;
  }
  if (nCols==0){
    IVYerr << "XGBoostInterface::build: Number of columns (" << nCols << ") is invalid." << endl;
    return false;
  }
  if (varnames.size()%nCols != 0){
    IVYerr << "XGBoostInterface::build: Number of variables (" << varnames.size() << ") is not divisible by the number of columns (" << nCols << ")." << endl;
    return false;
  }

  defval = missing_entry_val;
  nColumns = nCols;
  nRows = varnames.size()/nColumns;

  for (auto const& vv:varnames) variables[vv] = defval;
  booster = new BoosterHandle;
  SAFE_XGBOOST(XGBoosterCreate(nullptr, 0, booster));
  SAFE_XGBOOST(XGBoosterLoadModel(booster, fname.Data()));

  return true;
}

bool XGBoostInterface::eval(std::unordered_map<TString, IvyMLDataType_t> const& vars, std::vector<double>& res){
  for (auto const& vv:vars){
    auto it_var = variables.find(vv.first);
    if (it_var==variables.end()){
      IVYerr << "XGBoostInterface::eval: Variable " << vv.first << " is not in the set of input data collection." << endl;
      return false;
    }
    it_var->second = vv.second;
  }

  IvyMLDataType_t* data_arr = new IvyMLDataType_t[nColumns*nRows];
  IvyMLDataType_t* data_arr_ptr = &(data_arr[0]);
  for (auto& vv:variables){
    *data_arr_ptr = vv.second;
    data_arr_ptr++;
  };

  bst_ulong nout = 0;
  const float* score;
  DMatrixHandle dvalues;
  SAFE_XGBOOST(XGDMatrixCreateFromMat(data_arr, nColumns, nRows, defval, &dvalues));
  SAFE_XGBOOST(XGBoosterPredict(booster, dvalues, 0, 0, &nout, &score));
  SAFE_XGBOOST(XGDMatrixFree(dvalues));

  res.clear();
  res.reserve(nout);
  for (bst_ulong rr=0; rr<nout; rr++) res.push_back(score[rr]);

  delete[] data_arr;
  for (auto& vv:variables) vv.second = defval;

  return true;
}
