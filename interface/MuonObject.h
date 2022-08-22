#ifndef MUONOBJECT_H
#define MUONOBJECT_H

#include "ParticleObject.h"


#define MUON_EXTRA_VARIABLES \
MUON_VARIABLE(bool, looseId, 0) \
MUON_VARIABLE(bool, mediumId, 0) \
MUON_VARIABLE(unsigned char, jetNDauCharged, 0) \
MUON_VARIABLE(float, segmentComp, 0) \
MUON_VARIABLE(float, miniPFRelIso_all, 0) \
MUON_VARIABLE(float, miniPFRelIso_chg, 0) \
MUON_VARIABLE(float, pfRelIso03_all, 0) \
MUON_VARIABLE(float, dxy, 0) \
MUON_VARIABLE(float, dz, 0) \
MUON_VARIABLE(float, sip3d, 0) \
MUON_VARIABLE(float, jetPtRelv2, 0) \
MUON_VARIABLE(float, jetRelIso, 0) \
MUON_VARIABLE(float, ptErr, 0)


class MuonVariables{
public:
#define MUON_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
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
