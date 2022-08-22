#ifndef ISOTRACKOBJECT_H
#define ISOTRACKOBJECT_H

#include "ParticleObject.h"


#define ISOTRACK_EXTRA_VARIABLES \
ISOTRACK_VARIABLE(bool, isPFcand, 0) \
ISOTRACK_VARIABLE(bool, isFromLostTrack, 0) \
ISOTRACK_VARIABLE(bool, isHighPurityTrack, 0) \
ISOTRACK_VARIABLE(int, fromPV, 0) \
ISOTRACK_VARIABLE(float, pfRelIso03_chg, 0) \
ISOTRACK_VARIABLE(float, pfRelIso03_all, 0) \
ISOTRACK_VARIABLE(float, miniPFRelIso_chg, 0) \
ISOTRACK_VARIABLE(float, miniPFRelIso_all, 0) \
ISOTRACK_VARIABLE(float, dxy, 0) \
ISOTRACK_VARIABLE(float, dz, 0)


class IsotrackVariables{
public:
#define ISOTRACK_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  ISOTRACK_EXTRA_VARIABLES;
#undef ISOTRACK_VARIABLE

  IsotrackVariables();
  IsotrackVariables(IsotrackVariables const& other);
  IsotrackVariables& operator=(const IsotrackVariables& other);

  void swap(IsotrackVariables& other);

};

class IsotrackObject : public ParticleObject{
public:
  IsotrackVariables extras;

  IsotrackObject();
  IsotrackObject(int id);
  IsotrackObject(int id, LorentzVector_t const& mom_);
  IsotrackObject(const IsotrackObject& other);
  IsotrackObject& operator=(const IsotrackObject& other);
  ~IsotrackObject();

  void swap(IsotrackObject& other);

};

#endif
