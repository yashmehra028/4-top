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

void ParticleObject::swap(ParticleObject& other){ IvyParticle::swap(other); }
