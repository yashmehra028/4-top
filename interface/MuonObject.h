#ifndef MUONOBJECT_H
#define MUONOBJECT_H

#include "ParticleObject.h"


#define MUON_EXTRA_VARIABLES \
MUON_VARIABLE(bool, looseId) \
MUON_VARIABLE(bool, mediumId) \
MUON_VARIABLE(unsigned char, jetNDauCharged) \
MUON_VARIABLE(float, segmentComp) \
MUON_VARIABLE(float, miniPFRelIso_all) \
MUON_VARIABLE(float, miniPFRelIso_chg) \
MUON_VARIABLE(float, pfRelIso03_all) \
MUON_VARIABLE(float, dxy) \
MUON_VARIABLE(float, dz) \
MUON_VARIABLE(float, sip3d) \
MUON_VARIABLE(float, jetPtRelv2) \
MUON_VARIABLE(float, jetRelIso) \
MUON_VARIABLE(float, ptErr)


class MuonVariables{
public:
#define MUON_VARIABLE(TYPE, NAME) TYPE NAME;
  MUON_EXTRA_VARIABLES;
#undef MUON_VARIABLE

  MuonVariables();
  MuonVariables(MuonVariables const& other);
  MuonVariables& operator=(const MuonVariables& other);

  void swap(MuonVariables& other);
};

class MuonObject : public ParticleObject{
public:
  MuonVariables extras;

  MuonObject();
  MuonObject(int id_);
  MuonObject(int id_, LorentzVector_t const& mom_);
  MuonObject(const MuonObject& other);
  MuonObject& operator=(const MuonObject& other);
  ~MuonObject();

  void swap(MuonObject& other);

  LorentzVector_t::Scalar ptrel() const;
  LorentzVector_t::Scalar ptratio() const;

};

#endif
