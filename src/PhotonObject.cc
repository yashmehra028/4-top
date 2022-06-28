#include <algorithm>
#include <utility>
#include "HelperFunctions.h"
#include "PhotonObject.h"


PhotonVariables::PhotonVariables(){
#define PHOTON_VARIABLE(TYPE, NAME) this->NAME=0;
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE
}
PhotonVariables::PhotonVariables(PhotonVariables const& other){
#define PHOTON_VARIABLE(TYPE, NAME) this->NAME=other.NAME;
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE
}
void PhotonVariables::swap(PhotonVariables& other){
#define PHOTON_VARIABLE(TYPE, NAME) std::swap(this->NAME, other.NAME);
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE
}
PhotonVariables& PhotonVariables::operator=(const PhotonVariables& other){
  PhotonVariables tmp(other);
  swap(tmp);
  return *this;
}


PhotonObject::PhotonObject() :
  ParticleObject(),
  extras()
{}
PhotonObject::PhotonObject(LorentzVector_t const& momentum_) :
  ParticleObject(22, momentum_),
  extras()
{}
PhotonObject::PhotonObject(const PhotonObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void PhotonObject::swap(PhotonObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
PhotonObject& PhotonObject::operator=(const PhotonObject& other){
  PhotonObject tmp(other);
  swap(tmp);
  return *this;
}
PhotonObject::~PhotonObject(){}
