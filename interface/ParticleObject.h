#ifndef PARTICLEOBJECT_H
#define PARTICLEOBJECT_H

#include <IvyFramework/IvyDataTools/interface/IvyDataTypes.h>
#include <IvyFramework/IvyDataTools/interface/IvyParticle.h>


class ParticleObject : public IvyParticle{
public:
  ParticleObject();
  ParticleObject(int id_);
  ParticleObject(int id_, LorentzVector_t const& mom_);
  ParticleObject(const ParticleObject& other);
  virtual ~ParticleObject(){}

  // Swap and assignment operators are not virtual; they bring more complication than necessary, so they are implemented in the derived classes.
  void swap(ParticleObject& other);

  virtual LorentzVector_t::Scalar ptrel() const{ return 0; }
  virtual LorentzVector_t::Scalar ptratio() const{ return 0; }

};

#endif
