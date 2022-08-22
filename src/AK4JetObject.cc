#include <algorithm>
#include <utility>
#include "AK4JetObject.h"


AK4JetVariables::AK4JetVariables(){
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE
}
AK4JetVariables::AK4JetVariables(AK4JetVariables const& other){
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE
}
void AK4JetVariables::swap(AK4JetVariables& other){
#define AK4JET_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  AK4JET_EXTRA_VARIABLES;
#undef AK4JET_VARIABLE
}
AK4JetVariables& AK4JetVariables::operator=(const AK4JetVariables& other){
  AK4JetVariables tmp(other);
  swap(tmp);
  return *this;
}

AK4JetLowPtVariables::AK4JetLowPtVariables(){
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  AK4JET_LOWPT_EXTRA_VARIABLES;
#undef AK4JET_LOWPT_VARIABLE
}
AK4JetLowPtVariables::AK4JetLowPtVariables(AK4JetLowPtVariables const& other){
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  AK4JET_LOWPT_EXTRA_VARIABLES;
#undef AK4JET_LOWPT_VARIABLE
}
void AK4JetLowPtVariables::swap(AK4JetLowPtVariables& other){
#define AK4JET_LOWPT_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  AK4JET_LOWPT_EXTRA_VARIABLES;
#undef AK4JET_LOWPT_VARIABLE
}
AK4JetLowPtVariables& AK4JetLowPtVariables::operator=(const AK4JetLowPtVariables& other){
  AK4JetLowPtVariables tmp(other);
  swap(tmp);
  return *this;
}

AK4JetObject::AK4JetObject() :
  ParticleObject(),
  extras(),
  extras_lowpt()
{}
AK4JetObject::AK4JetObject(LorentzVector_t const& momentum_) :
  ParticleObject(0, momentum_),
  extras(),
  extras_lowpt()
{}
AK4JetObject::AK4JetObject(const AK4JetObject& other) :
  ParticleObject(other),
  extras(other.extras),
  extras_lowpt(other.extras_lowpt)
{}
void AK4JetObject::swap(AK4JetObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
  extras_lowpt.swap(other.extras_lowpt);
}
AK4JetObject& AK4JetObject::operator=(const AK4JetObject& other){
  AK4JetObject tmp(other);
  swap(tmp);
  return *this;
}
AK4JetObject::~AK4JetObject(){}


