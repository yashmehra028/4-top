#ifndef ELECTRONOBJECT_H
#define ELECTRONOBJECT_H

#include "ParticleObject.h"


#define ELECTRON_EXTRA_VARIABLES \
ELECTRON_VARIABLE(bool, convVeto) \
ELECTRON_VARIABLE(unsigned char, lostHits) \
ELECTRON_VARIABLE(unsigned char, jetNDauCharged) \
ELECTRON_VARIABLE(int, tightCharge) \
ELECTRON_VARIABLE(float, hoe) \
ELECTRON_VARIABLE(float, eInvMinusPInv) \
ELECTRON_VARIABLE(float, sieie) \
ELECTRON_VARIABLE(float, dxy) \
ELECTRON_VARIABLE(float, dz) \
ELECTRON_VARIABLE(float, sip3d) \
ELECTRON_VARIABLE(float, mvaFall17V2noIso) \
ELECTRON_VARIABLE(float, miniPFRelIso_all) \
ELECTRON_VARIABLE(float, miniPFRelIso_chg) \
ELECTRON_VARIABLE(float, pfRelIso03_all) \
ELECTRON_VARIABLE(float, jetPtRelv2) \
ELECTRON_VARIABLE(float, jetRelIso) \
ELECTRON_VARIABLE(float, deltaEtaSC)


class ElectronVariables{
public:
#define ELECTRON_VARIABLE(TYPE, NAME) TYPE NAME;
  ELECTRON_EXTRA_VARIABLES;
#undef ELECTRON_VARIABLE

  ElectronVariables();
  ElectronVariables(ElectronVariables const& other);
  ElectronVariables& operator=(const ElectronVariables& other);

  void swap(ElectronVariables& other);
};

class ElectronObject : public ParticleObject{
public:
  ElectronVariables extras;

  ElectronObject();
  ElectronObject(int id_);
  ElectronObject(int id_, LorentzVector_t const& mom_);
  ElectronObject(const ElectronObject& other);
  ElectronObject& operator=(const ElectronObject& other);
  ~ElectronObject();

  void swap(ElectronObject& other);

  float etaSC() const{ return this->extras.deltaEtaSC + this->eta(); }

  LorentzVector_t::Scalar ptrel() const;
  LorentzVector_t::Scalar ptratio() const;

};

#endif
