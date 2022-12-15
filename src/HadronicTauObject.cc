#include <algorithm>
#include <utility>
#include <cmath>
#include "HadronicTauObject.h"
#include "AK4JetObject.h"
#include "TVector3.h"


HadronicTauVariables::HadronicTauVariables(){
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE
}
HadronicTauVariables::HadronicTauVariables(HadronicTauVariables const& other){
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE
}
void HadronicTauVariables::swap(HadronicTauVariables& other){
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE
}
HadronicTauVariables& HadronicTauVariables::operator=(const HadronicTauVariables& other){
  HadronicTauVariables tmp(other);
  swap(tmp);
  return *this;
}

HadronicTauObject::HadronicTauObject() :
  ParticleObject(),
  extras()
{}
HadronicTauObject::HadronicTauObject(int id_) :
  ParticleObject(id_),
  extras()
{}
HadronicTauObject::HadronicTauObject(int id_, LorentzVector_t const& momentum_) :
  ParticleObject(id_, momentum_),
  extras()
{}
HadronicTauObject::HadronicTauObject(const HadronicTauObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void HadronicTauObject::swap(HadronicTauObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
HadronicTauObject& HadronicTauObject::operator=(const HadronicTauObject& other){
  HadronicTauObject tmp(other);
  swap(tmp);
  return *this;
}
HadronicTauObject::~HadronicTauObject(){}
