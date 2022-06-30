#ifndef PHOTONSELECTIONHELPERS_H
#define PHOTONSELECTIONHELPERS_H

#include "PhotonObject.h"


namespace PhotonSelectionHelpers{
  enum SelectionBits{
    kKinOnly,

    kPreselectionLoose,
    kPreselectionTight,

    nSelectionBits
  };

  // Kinematic thresholds
  constexpr float ptThr = 20.;
  constexpr float etaThr = 2.5;

  float getIsolationDRmax(PhotonObject const& part);

  void setSelectionBits(PhotonObject& part);
}


#endif
