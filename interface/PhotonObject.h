#ifndef PHOTONOBJECT_H
#define PHOTONOBJECT_H

#include <string>

#include "ParticleObject.h"


#define PHOTON_EXTRA_VARIABLES \
PHOTON_VARIABLE(bool, isScEtaEB) \
PHOTON_VARIABLE(bool, isScEtaEE) \
PHOTON_VARIABLE(bool, cutBased)


class PhotonVariables{
public:
#define PHOTON_VARIABLE(TYPE, NAME) TYPE NAME;
  PHOTON_EXTRA_VARIABLES;
#undef PHOTON_VARIABLE

  PhotonVariables();
  PhotonVariables(PhotonVariables const& other);
  PhotonVariables& operator=(const PhotonVariables& other);

  void swap(PhotonVariables& other);

};

class PhotonObject : public ParticleObject{
public:
  PhotonVariables extras;

  PhotonObject();
  PhotonObject(LorentzVector_t const& mom_);
  PhotonObject(const PhotonObject& other);
  PhotonObject& operator=(const PhotonObject& other);
  ~PhotonObject();

  void swap(PhotonObject& other);

  bool isEB() const{ return this->extras.isScEtaEB; }
  bool isEE() const{ return this->extras.isScEtaEE; }
  bool isEBEEGap() const{ return !(this->isEB() || this->isEE()); }
  bool isAnyGap() const{ return this->isEBEEGap(); } // Not entirely correct, but assume it is for now because nano... :/
  bool isGap() const{ return this->isAnyGap(); }

  float etaSC() const{ return this->eta(); } // NanoAOD still does not have a deltaEtaSC branch...

};

#endif
