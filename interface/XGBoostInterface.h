#ifndef XGBOOSTINTERFACE_H
#define XGBOOSTINTERFACE_H

#include <unordered_map>
#include <map>
#include <xgboost/c_api.h>
#include "IvyFramework/IvyDataTools/interface/StdExtensions.h"


class IvyMLWrapper{
public:
  typedef float IvyMLDataType_t;

  IvyMLWrapper(){};
  virtual ~IvyMLWrapper(){};

  virtual bool build(TString fname, std::vector<TString> const& varnames, unsigned long long nCols) = 0;

  virtual bool eval(std::unordered_map<TString, IvyMLDataType_t> const& vals, std::vector<double>& res) = 0;

};

class XGBoostInterface : public IvyMLWrapper{
protected:
  BoosterHandle* booster;
  unsigned long long nColumns;
  unsigned long long nRows;
  IvyMLDataType_t defval;
  std::map<TString, float> variables;

public:
  XGBoostInterface();
  virtual ~XGBoostInterface();

  bool build(TString fname, std::vector<TString> const& varnames, IvyMLDataType_t missing_entry_val, unsigned long long nCols=1);

  bool eval(std::unordered_map<TString, IvyMLDataType_t> const& vals, std::vector<double>& res);

  BoosterHandle* const& getBooster() const{ return booster; }

};


#endif
