#include <algorithm>
#include <utility>
#include "TriggerObject.h"


TriggerObjectVariables::TriggerObjectVariables(){
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) this->NAME=0;
  TRIGGEROBJECT_EXTRA_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE
}
TriggerObjectVariables::TriggerObjectVariables(TriggerObjectVariables const& other){
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) this->NAME=other.NAME;
  TRIGGEROBJECT_EXTRA_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE
}
void TriggerObjectVariables::swap(TriggerObjectVariables& other){
#define TRIGGEROBJECT_VARIABLE(TYPE, NAME) std::swap(this->NAME, other.NAME);
  TRIGGEROBJECT_EXTRA_VARIABLES;
#undef TRIGGEROBJECT_VARIABLE
}
TriggerObjectVariables& TriggerObjectVariables::operator=(const TriggerObjectVariables& other){
  TriggerObjectVariables tmp(other);
  swap(tmp);
  return *this;
}

TriggerObject::TriggerObject() :
  ParticleObject(),
  extras()
{}
TriggerObject::TriggerObject(int id_) :
  ParticleObject(id_),
  extras()
{}
TriggerObject::TriggerObject(int id_, LorentzVector_t const& momentum_) :
  ParticleObject(id_, momentum_),
  extras()
{}
TriggerObject::TriggerObject(const TriggerObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void TriggerObject::swap(TriggerObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
TriggerObject& TriggerObject::operator=(const TriggerObject& other){
  TriggerObject tmp(other);
  swap(tmp);
  return *this;
}
