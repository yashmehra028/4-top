#ifndef IVYMLWRAPPER_H
#define IVYMLWRAPPER_H

#include <unordered_map>
#include <map>
#include <vector>
#include "IvyFramework/IvyDataTools/interface/StdExtensions.h"


class IvyMLWrapper{
public:
  typedef float IvyMLDataType_t;

  IvyMLWrapper(){};
  virtual ~IvyMLWrapper(){};

  virtual bool build(TString fname, std::vector<TString> const& varnames, unsigned long long nCols) = 0;

  virtual bool eval(std::unordered_map<TString, IvyMLDataType_t> const& vals, std::vector<double>& res) = 0;

};


#endif
