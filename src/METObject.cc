#include <algorithm>
#include <utility>
#include <cmath>
#include "METObject.h"


using namespace std;


METVariables::METVariables(){
#define MET_VARIABLE(TYPE, NAME) this->NAME=0;
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE
}
METVariables::METVariables(METVariables const& other){
#define MET_VARIABLE(TYPE, NAME) this->NAME=other.NAME;
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE
}
void METVariables::swap(METVariables& other){
#define MET_VARIABLE(TYPE, NAME) std::swap(this->NAME, other.NAME);
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE
}
METVariables& METVariables::operator=(const METVariables& other){
  METVariables tmp(other);
  swap(tmp);
  return *this;
}

METObject::METObject() :
  extras(),
  currentXYshift(0, 0, 0, 0)
{}
METObject::METObject(const METObject& other) :
  extras(other.extras),
  currentXYshift(other.currentXYshift)
{}
void METObject::swap(METObject& other){
  extras.swap(other.extras);
  std::swap(currentXYshift, other.currentXYshift);
}
METObject& METObject::operator=(const METObject& other){
  METObject tmp(other);
  swap(tmp);
  return *this;
}
METObject::~METObject(){}

void METObject::getPtPhi(float& pt, float& phi) const{
  ParticleObject::LorentzVector_t tmp_p4;

  auto const& pt_ref = extras.pt;
  auto const& phi_ref = extras.phi;
  tmp_p4 = ParticleObject::LorentzVector_t(pt_ref*std::cos(phi_ref), pt_ref*std::sin(phi_ref), 0., 0.);

  tmp_p4 += currentXYshift;

  pt = tmp_p4.Pt();
  phi = tmp_p4.Phi();
}
ParticleObject::LorentzVector_t::Scalar METObject::met() const{
  float pt, phi;
  getPtPhi(pt, phi);
  return pt;
}
ParticleObject::LorentzVector_t::Scalar METObject::phi() const{
  float pt, phi;
  getPtPhi(pt, phi);
  return phi;
}
ParticleObject::LorentzVector_t::Scalar METObject::px() const{
  float pt, phi;
  getPtPhi(pt, phi);
  return pt * std::cos(phi);
}
ParticleObject::LorentzVector_t::Scalar METObject::py() const{
  float pt, phi;
  getPtPhi(pt, phi);
  return pt * std::sin(phi);
}
ParticleObject::LorentzVector_t METObject::p4() const{
  float pt, phi;
  getPtPhi(pt, phi);
  return constructP4FromPtPhi(pt, phi);
}

ParticleObject::LorentzVector_t METObject::constructP4FromPtPhi(ParticleObject::LorentzVector_t::Scalar pt, ParticleObject::LorentzVector_t::Scalar phi){
  return ParticleObject::LorentzVector_t(pt * std::cos(phi), pt * std::sin(phi), 0., pt);
}
ParticleObject::LorentzVector_t METObject::constructP4FromPxPy(ParticleObject::LorentzVector_t::Scalar px, ParticleObject::LorentzVector_t::Scalar py){
  return ParticleObject::LorentzVector_t(px, py, 0., std::sqrt(std::pow(px, 2) + std::pow(py, 2)));
}
