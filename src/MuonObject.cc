#include <algorithm>
#include <utility>
#include <cmath>
#include "MuonObject.h"
#include "AK4JetObject.h"
#include "TVector3.h"


MuonVariables::MuonVariables(){
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE
}
MuonVariables::MuonVariables(MuonVariables const& other){
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE
}
void MuonVariables::swap(MuonVariables& other){
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE
}
MuonVariables& MuonVariables::operator=(const MuonVariables& other){
  MuonVariables tmp(other);
  swap(tmp);
  return *this;
}

MuonObject::MuonObject() :
  ParticleObject(),
  extras()
{}
MuonObject::MuonObject(int id_) :
  ParticleObject(id_),
  extras()
{}
MuonObject::MuonObject(int id_, LorentzVector_t const& momentum_) :
  ParticleObject(id_, momentum_),
  extras()
{}
MuonObject::MuonObject(const MuonObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void MuonObject::swap(MuonObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
MuonObject& MuonObject::operator=(const MuonObject& other){
  MuonObject tmp(other);
  swap(tmp);
  return *this;
}
MuonObject::~MuonObject(){}

IvyParticle::LorentzVector_t::Scalar MuonObject::ptrel() const{
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
IvyParticle::LorentzVector_t::Scalar MuonObject::ptratio() const{
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

