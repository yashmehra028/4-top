#ifndef HADRONICTAUHANDLER_H
#define HADRONICTAUHANDLER_H

#include <vector>
#include "IvyBase.h"
#include "HadronicTauObject.h"
#include "ParticleDisambiguator.h"


class HadronicTauHandler : public IvyBase{
public:
  typedef HadronicTauObject ProductType_t;

  static const std::string colName;

protected:
  friend class ParticleDisambiguator;

  std::vector<ProductType_t*> productList;

  void clear(){ this->resetCache(); for (ProductType_t*& prod:productList) delete prod; productList.clear(); }

  bool constructHadronicTauObjects();

public:
  // Constructors
  HadronicTauHandler();

  // Destructors
  ~HadronicTauHandler(){ clear(); }

  bool constructHadronicTaus();

  std::vector<ProductType_t*> const& getProducts() const{ return productList; }

  void bookBranches(BaseTree* tree);

};


#endif
