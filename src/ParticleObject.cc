#include <algorithm>
#include <utility>
#include <IvyFramework/IvyDataTools/interface/HelperFunctions.h>
#include "ParticleObject.h"


ParticleObject::ParticleObject() :
  IvyParticle()
{}
ParticleObject::ParticleObject(int id_) :
  IvyParticle(id_)
{}
ParticleObject::ParticleObject(int id_, LorentzVector_t const& momentum_) :
  IvyParticle(id_, momentum_)
{}
ParticleObject::ParticleObject(const ParticleObject& other) :
  IvyParticle(other)
{}

void ParticleObject::swap(ParticleObject& other){
  std::swap(this->mvascore_ext_map, other.mvascore_ext_map);
  IvyParticle::swap(other);
}

void ParticleObject::setExternalMVAScore(int const& key, float const& val){ mvascore_ext_map[key] = val; }
bool ParticleObject::getExternalMVAScore(int const& key, float& val) const{
  auto it_mvascore_ext_map = mvascore_ext_map.find(key);
  if (it_mvascore_ext_map==mvascore_ext_map.cend()) return false;
  else{
    val = it_mvascore_ext_map->second;
    return true;
  }
}

ParticleObject* ParticleObject::getFirstMotherInChain_matchPDGId(ParticleObject* part, int const& id_req){
  ParticleObject* res = nullptr;
  if (part){
    if (part->pdgId()==id_req) res = part;
    else{ for (auto const& mpart:part->getMothers()){ if (mpart && !res) res = getFirstMotherInChain_matchPDGId(dynamic_cast<ParticleObject*>(mpart), id_req); } }
  }
  return res;
}
