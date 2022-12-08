#include <algorithm>
#include <utility>
#include "GenJetObject.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctionsCore.h"


GenJetVariables::GenJetVariables(){
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  GENJET_EXTRA_VARIABLES;
#undef GENJET_VARIABLE
}
GenJetVariables::GenJetVariables(GenJetVariables const& other){
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  GENJET_EXTRA_VARIABLES;
#undef GENJET_VARIABLE
}
void GenJetVariables::swap(GenJetVariables& other){
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  GENJET_EXTRA_VARIABLES;
#undef GENJET_VARIABLE
}
GenJetVariables& GenJetVariables::operator=(const GenJetVariables& other){
  GenJetVariables tmp(other);
  swap(tmp);
  return *this;
}

GenJetObject::GenJetObject() :
  ParticleObject(),
  extras()
{}
GenJetObject::GenJetObject(LorentzVector_t const& momentum_) :
  ParticleObject(0, momentum_),
  extras()
{}
GenJetObject::GenJetObject(const GenJetObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void GenJetObject::swap(GenJetObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
GenJetObject& GenJetObject::operator=(const GenJetObject& other){
  GenJetObject tmp(other);
  swap(tmp);
  return *this;
}
GenJetObject::~GenJetObject(){}
