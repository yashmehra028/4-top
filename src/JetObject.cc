#include "JetObject.h"

#include <algorithm> // used for swap
#include <utility> // used for swap

//#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"  may not need this

JetVariables::JetVariables(){
#define JET_VARIABLE(TYPE, NAME) this->NAME=0;
  JET_VARIABLES;
#undef JET_VARIABLE
}
JetVariables::JetVariables(JetVariables const& other){
#define JET_VARIABLE(TYPE, NAME) this->NAME=other.NAME;
  JET_VARIABLES;
#undef JET_VARIABLE
}
void JetVariables::swap(JetVariables& other){
#define JET_VARIABLE(TYPE, NAME) std::swap(this->NAME, other.NAME);
  JET_VARIABLES;
#undef JET_VARIABLE
}
JetVariables& JetVariables::operator=(const JetVariables& other){
  JetVariables tmp(other);
  swap(tmp);
  return *this;
}

JetObject::JetObject() :
  ParticleObject(),
  extras()
{}
JetObject::JetObject(int id_) :
  ParticleObject(id_),
  extras()
{}
JetObject::JetObject(int id_, LorentzVector_t const& momentum_) :
  ParticleObject(id_, momentum_),
  extras()
{}
JetObject::JetObject(const JetObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void JetObject::swap(JetObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
JetObject& JetObject::operator=(const JetObject& other){
  JetObject tmp(other);
  swap(tmp);
  return *this;
}
JetObject::~JetObject(){}


