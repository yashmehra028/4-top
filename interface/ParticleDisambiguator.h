#ifndef PARTICLEDISAMBIGUATOR_H
#define PARTICLEDISAMBIGUATOR_H

#include <vector>
#include "MuonObject.h"
#include "ElectronObject.h"
#include "PhotonObject.h"
#include "AK4JetObject.h"


class MuonHandler;
class ElectronHandler;
class PhotonHandler;
class JetMETHandler;


class ParticleDisambiguator{
protected:
  void disambiguateParticles(
    std::vector<MuonObject*>*& muons,
    std::vector<ElectronObject*>*& electrons,
    std::vector<PhotonObject*>*& photons,
    std::vector<AK4JetObject*>*& ak4jets,
    std::vector<AK4JetObject*>*& ak4jets_masked
  );

public:
  ParticleDisambiguator(){};

  void disambiguateParticles(
    MuonHandler* muonHandle,
    ElectronHandler* electronHandle,
    PhotonHandler* photonHandle,
    JetMETHandler* jetHandle
  );

};


#endif
