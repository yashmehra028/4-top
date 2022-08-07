#ifndef IVYXGBOOSTINTERFACE_H
#define IVYXGBOOSTINTERFACE_H

#include <xgboost/c_api.h>
#include "IvyMLWrapper.h"


class IvyXGBoostInterface : public IvyMLWrapper{
protected:
  BoosterHandle* booster;
  unsigned long long nColumns;
  unsigned long long nRows;
  IvyMLDataType_t defval;
  std::map<TString, float> variables;

public:
  IvyXGBoostInterface();
  virtual ~IvyXGBoostInterface();

  bool build(TString fname, std::vector<TString> const& varnames, IvyMLDataType_t missing_entry_val, unsigned long long nCols=1);

  bool eval(std::unordered_map<TString, IvyMLDataType_t> const& vals, std::vector<double>& res);

  BoosterHandle* const& getBooster() const{ return booster; }

};


#endif
