#include <algorithm>
#include <utility>
#include "LHEParticleObject.h"


LHEParticleVariables::LHEParticleVariables(){
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  LHEPARTICLE_EXTRA_VARIABLES;
#undef LHEPARTICLE_VARIABLE
}
LHEParticleVariables::LHEParticleVariables(LHEParticleVariables const& other){
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  LHEPARTICLE_EXTRA_VARIABLES;
#undef LHEPARTICLE_VARIABLE
}
void LHEParticleVariables::swap(LHEParticleVariables& other){
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  LHEPARTICLE_EXTRA_VARIABLES;
#undef LHEPARTICLE_VARIABLE
}
LHEParticleVariables& LHEParticleVariables::operator=(const LHEParticleVariables& other){
  LHEParticleVariables tmp(other);
  swap(tmp);
  return *this;
}

LHEParticleObject::LHEParticleObject() :
  ParticleObject(),
  st(0),
  extras()
{}
LHEParticleObject::LHEParticleObject(int id_, int st_) :
  ParticleObject(id_),
  st(st_),
  extras()
{}
LHEParticleObject::LHEParticleObject(int id_, int st_, LorentzVector_t const& momentum_) :
  ParticleObject(id_, momentum_),
  st(st_),
  extras()
{}
LHEParticleObject::LHEParticleObject(const LHEParticleObject& other) :
  ParticleObject(other),
  st(other.st),
  extras(other.extras)
{}
void LHEParticleObject::swap(LHEParticleObject& other){
  ParticleObject::swap(other);
  std::swap(st, other.st);
  extras.swap(other.extras);
}
LHEParticleObject& LHEParticleObject::operator=(const LHEParticleObject& other){
  LHEParticleObject tmp(other);
  swap(tmp);
  return *this;
}
LHEParticleObject::~LHEParticleObject(){}
