#include "ElectronObject.h"

#include <algorithm> // used for swap
#include <utility> // used for swap

//#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"  may not need this

ElectronVariables::ElectronVariables(){
#define ELECTRON_VARIABLE(TYPE, NAME) this->NAME=0;
  ELECTRON_VARIABLES;
#undef ELECTRON_VARIABLE
}
ElectronVariables::ElectronVariables(ElectronVariables const& other){
#define ELECTRON_VARIABLE(TYPE, NAME) this->NAME=other.NAME;
  ELECTRON_VARIABLES;
#undef ELECTRON_VARIABLE
}
void ElectronVariables::swap(ElectronVariables& other){
#define ELECTRON_VARIABLE(TYPE, NAME) std::swap(this->NAME, other.NAME);
  ELECTRON_VARIABLES;
#undef ELECTRON_VARIABLE
}
ElectronVariables& ElectronVariables::operator=(const ElectronVariables& other){
  ElectronVariables tmp(other);
  swap(tmp);
  return *this;
}

ElectronObject::ElectronObject() :
  ParticleObject(),
  extras()
{}
ElectronObject::ElectronObject(int id_) :
  ParticleObject(id_),
  extras()
{}
ElectronObject::ElectronObject(int id_, LorentzVector_t const& momentum_) :
  ParticleObject(id_, momentum_),
  extras()
{}
ElectronObject::ElectronObject(const ElectronObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void ElectronObject::swap(ElectronObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
ElectronObject& ElectronObject::operator=(const ElectronObject& other){
  ElectronObject tmp(other);
  swap(tmp);
  return *this;
}
ElectronObject::~ElectronObject(){}


