#ifndef TRIGGEROBJECT_H
#define TRIGGEROBJECT_H

#include <vector>
#include <utility>
#include "ParticleObject.h"
#include "ParticleObjectHelpers.h"


enum TriggerObjectType{
  TriggerPhoton=22,
  TriggerElectron=11,
  TriggerMuon=13,
  TriggerTau=15,
  TriggerJet=1,
  TriggerFatJet=6,
  TriggerMET=2,
  TriggerMHT=4,
  TriggerHT=3
};


#define TRIGGEROBJECT_EXTRA_VARIABLES \
TRIGGEROBJECT_VARIABLE(int, filterBits)
#define TRIGGEROBJECT_MOMENTUM_VARIABLES \
TRIGGEROBJECT_VARIABLE(int, id) \
TRIGGEROBJECT_VARIABLE(float, pt) \
TRIGGEROBJECT_VARIABLE(float, eta) \
TRIGGEROBJECT_VARIABLE(float, phi) \
TRIGGEROBJECT_VARIABLE(float, mass)

#define TRIGGEROBJECT_VARIABLES \
TRIGGEROBJECT_EXTRA_VARIABLES \
TRIGGEROBJECT_MOMENTUM_VARIABLES


class TriggerObjectVariables{
public:
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) TYPE NAME;
  TRIGGEROBJECT_EXTRA_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE

  TriggerObjectVariables();
  TriggerObjectVariables(TriggerObjectVariables const& other);
  TriggerObjectVariables& operator=(const TriggerObjectVariables& other);

  void swap(TriggerObjectVariables& other);

};

class TriggerObject : public ParticleObject{
public:
  TriggerObjectVariables extras;

  TriggerObject();
  TriggerObject(int id_);
  TriggerObject(int id_, LorentzVector_t const& mom_);
  TriggerObject(const TriggerObject& other);
  TriggerObject& operator=(const TriggerObject& other);
  ~TriggerObject(){}

  void swap(TriggerObject& other);

  TriggerObjectType getTriggerObjectType() const{ return static_cast<TriggerObjectType>(this->id); }
  bool isTriggerObjectType(TriggerObjectType const& TOtype) const{ return (this->getTriggerObjectType() == TOtype); }

  template<typename T> static void getMatchedPhysicsObjects(
    std::vector<TriggerObject const*> const& allPassedTOs, std::vector<TriggerObjectType> const& TOreqs, double const& dRmatch,
    std::vector<T*> const& objlist, std::vector<T*>& res,
    std::vector< std::pair<T*, TriggerObject const*> >* matchingLegs=nullptr
  );

};

template<typename T> void TriggerObject::getMatchedPhysicsObjects(
  std::vector<TriggerObject const*> const& allPassedTOs, std::vector<TriggerObjectType> const& TOreqs, double const& dRmatch,
  std::vector<T*> const& objlist, std::vector<T*>& res,
  std::vector< std::pair<T*, TriggerObject const*> >* matchingLegs
){
  res.reserve(objlist.size());
  if (matchingLegs) matchingLegs->reserve(objlist.size());

  std::vector<TriggerObject const*> hltFlaggedObjs;
  for (auto const& to_obj:allPassedTOs){
    for (TriggerObjectType const& TOreq:TOreqs){
      if (to_obj->isTriggerObjectType(TOreq)){ hltFlaggedObjs.push_back(to_obj); break; }
    }
  }

  std::unordered_map<TriggerObject const*, T*> TO_physobj_map;
  ParticleObjectHelpers::matchParticles(
    ParticleObjectHelpers::kMatchBy_DeltaR, dRmatch,
    hltFlaggedObjs.cbegin(), hltFlaggedObjs.cend(),
    objlist.cbegin(), objlist.cend(),
    TO_physobj_map
  );
  for (auto const& it:TO_physobj_map){
    if (it.second){
      res.push_back(it.second);
      if (matchingLegs) matchingLegs->emplace_back(it.second, it.first);
    }
  }

  ParticleObjectHelpers::sortByGreaterPt(res);
}


#endif
