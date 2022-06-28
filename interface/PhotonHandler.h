#ifndef PHOTONHANDLER_H
#define PHOTONHANDLER_H

#include <vector>
#include "IvyBase.h"
#include "PhotonObject.h"
#include "ParticleDisambiguator.h"


class PhotonHandler : public IvyBase{
public:
  typedef PhotonObject ProductType_t;

  static const std::string colName;

protected:
  friend class ParticleDisambiguator;

  std::vector<ProductType_t*> productList;

  void clear(){ this->resetCache(); for (ProductType_t*& prod:productList) delete prod; productList.clear(); }

  bool constructPhotonObjects();

public:
  // Constructors
  PhotonHandler();

  // Destructors
  ~PhotonHandler(){ clear(); }

  bool constructPhotons();

  std::vector<ProductType_t*> const& getProducts() const{ return productList; }

  void bookBranches(BaseTree* tree);

};


#endif
