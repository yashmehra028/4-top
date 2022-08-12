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

  virtual bool build(TString fname, std::vector<TString> const& varnames, IvyMLDataType_t missing_entry_val, unsigned long long nCols) = 0;

};


#endif
