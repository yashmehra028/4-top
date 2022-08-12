#include <algorithm>
#include <utility>
#include <cmath>
#include "ElectronObject.h"
#include "AK4JetObject.h"
#include "TVector3.h"


ElectronVariables::ElectronVariables(){
#define ELECTRON_VARIABLE(TYPE, NAME) this->NAME=0;
  ELECTRON_EXTRA_VARIABLES;
#undef ELECTRON_VARIABLE
}
ElectronVariables::ElectronVariables(ElectronVariables const& other){
#define ELECTRON_VARIABLE(TYPE, NAME) this->NAME=other.NAME;
  ELECTRON_EXTRA_VARIABLES;
#undef ELECTRON_VARIABLE
}
void ElectronVariables::swap(ElectronVariables& other){
#define ELECTRON_VARIABLE(TYPE, NAME) std::swap(this->NAME, other.NAME);
  ELECTRON_EXTRA_VARIABLES;
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

IvyParticle::LorentzVector_t::Scalar ElectronObject::ptrel() const{
  constexpr bool useNanoVars = true;
  if (!useNanoVars){
    AK4JetObject* mother = nullptr;
  for (auto const& mom:mothers){
    mother = dynamic_cast<AK4JetObject*>(mom);
    if (mother) break;
  }
  if (!mother) return 0.;
  TVector3 p3_part = this->p4_TLV().Vect();
  TVector3 p3_jet = mother->p4_TLV().Vect();
  TVector3 p3_diff = p3_jet - p3_part;
  return std::abs(p3_diff.Cross(p3_part).Mag()/p3_diff.Mag());
  }
  else{
    return this->extras.jetPtRelv2;
  }
}
IvyParticle::LorentzVector_t::Scalar ElectronObject::ptratio() const{
  constexpr bool useNanoVars = true;
  if (!useNanoVars){
    AK4JetObject* mother = nullptr;
  for (auto const& mom:mothers){
    mother = dynamic_cast<AK4JetObject*>(mom);
    if (mother) break;
  }
  if (!mother) return 0.;
  return this->pt() / mother->pt();
  }
  else{
    return 1./(this->extras.jetRelIso + 1.);
  }
}
