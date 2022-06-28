#ifndef ELECTRONHANDLER_H
#define ELECTRONHANDLER_H

#include <vector>
#include "IvyBase.h"
#include "ElectronObject.h"
#include "ParticleDisambiguator.h"


class ElectronHandler : public IvyBase{
public:
  typedef ElectronObject ProductType_t;

  static const std::string colName;

protected:
  friend class ParticleDisambiguator;

  std::vector<ProductType_t*> productList;

  void clear(){ this->resetCache(); for (ProductType_t*& prod:productList) delete prod; productList.clear(); }

  bool constructElectronObjects();

public:
  // Constructors
  ElectronHandler();

  // Destructors
  ~ElectronHandler(){ clear(); }

  bool constructElectrons();

  std::vector<ProductType_t*> const& getProducts() const{ return productList; }

  void bookBranches(BaseTree* tree);

};


#endif
