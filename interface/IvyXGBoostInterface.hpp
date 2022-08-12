#ifndef IVYXGBOOSTINTERFACE_HPP
#define IVYXGBOOSTINTERFACE_HPP

#include <cassert>
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "IvyXGBoostInterface.h"


using namespace std;
using namespace IvyStreamHelpers;


#define SAFE_XGBOOST(CALL) \
{ int err_call = (CALL); if (err_call!=0){ IVYerr << "Call '" << #CALL << "' returned error code " << err_call << ". XGBoost last error: " << XGBGetLastError() << endl; } }


template<typename T> bool IvyXGBoostInterface::eval(std::unordered_map<TString, IvyMLDataType_t> const& vars, std::vector<T>& res){
  res.clear();

  for (auto const& vv:vars){
    auto it_var = variables.find(vv.first);
    if (it_var==variables.end()){
      IVYerr << "IvyXGBoostInterface::eval: Variable " << vv.first << " is not in the set of input data collection." << endl;
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
  SAFE_XGBOOST(XGBoosterPredict(*booster, dvalues, 0, 0, &nout, &score));
  SAFE_XGBOOST(XGDMatrixFree(dvalues));

  res.clear();
  res.reserve(nout);
  for (bst_ulong rr=0; rr<nout; rr++) res.push_back(static_cast<T>(score[rr]));

  delete[] data_arr;
  for (auto& vv:variables) vv.second = defval;

  return true;
}

template<typename T> bool IvyXGBoostInterface::eval(std::unordered_map<TString, IvyMLDataType_t> const& vars, T& res){
  std::vector<T> vres;
  this->eval(vars, vres);
  if (vres.empty() || vres.size()!=1){
    IVYerr << "IvyXGBoostInterface::eval: The vector of results has size = " << vres.size() << " != 1." << endl;
    assert(0);
  }
  return vres.front();
}
template bool IvyXGBoostInterface::eval<float>(std::unordered_map<TString, IvyMLDataType_t> const& vars, float& res);
template bool IvyXGBoostInterface::eval<double>(std::unordered_map<TString, IvyMLDataType_t> const& vars, double& res);

#endif
