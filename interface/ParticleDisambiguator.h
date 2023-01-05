#ifndef PARTICLEDISAMBIGUATOR_H
#define PARTICLEDISAMBIGUATOR_H

#include <vector>
#include <memory>
#include "MuonObject.h"
#include "ElectronObject.h"
#include "PhotonObject.h"
#include "HadronicTauObject.h"
#include "AK4JetObject.h"
#include "METObject.h"


class MuonHandler;
class ElectronHandler;
class PhotonHandler;
class HadronicTauHandler;
class JetMETHandler;


class ParticleDisambiguator{
protected:
  void disambiguateParticles(
    std::vector<MuonObject*>*& muons,
    std::vector<ElectronObject*>*& electrons,
    std::vector<PhotonObject*>*& photons,
    std::vector<HadronicTauObject*>*& htaus,
    std::vector<AK4JetObject*>*& ak4jets,
    std::vector<AK4JetObject*>*& ak4jets_masked,
    METObject* met
  ) const;

public:
  ParticleDisambiguator(){}

  void disambiguateParticles(
    MuonHandler* muonHandle,
    ElectronHandler* electronHandle,
    PhotonHandler* photonHandle,
    JetMETHandler* jetHandle,
    HadronicTauHandler* htauHandle = nullptr
  ) const;

};


#endif
