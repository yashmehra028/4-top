#ifndef INPUTCHUNKSPLITTER_H
#define INPUTCHUNKSPLITTER_H

#include <cassert>
#include "SamplesCore.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


void splitInputEventsIntoChunks(
  bool const& isData, int const& nEntries, // Specifications needed from the input tree
  int const& ichunk, int const& nchunks, // Split info.
  int& eventIndex_begin, int& eventIndex_end // Event idendifier ranges [) for the sim. event range, and [] for the data run range.
){
  using namespace std;
  using namespace IvyStreamHelpers;

  eventIndex_begin = -1;
  eventIndex_end = -1;
  if (nchunks>0){
    if (!isData){
      int ev_inc = static_cast<int>(static_cast<double>(nEntries)/static_cast<double>(nchunks));
      int ev_rem = nEntries - ev_inc*nchunks;
      eventIndex_begin = ev_inc*ichunk + std::min(ev_rem, ichunk);
      eventIndex_end = ev_inc*(ichunk+1) + std::min(ev_rem, ichunk+1);
      IVYout << "splitInputEventsIntoChunks: A simulation loop will proceed. The requested event range is [" << eventIndex_begin << ", " << eventIndex_end << ")." << endl;
    }
    else{
      if (!SampleHelpers::testDataPeriodIsLikeData(SampleHelpers::getDataPeriod())){
        IVYerr << "splitInputEventsIntoChunks: When you run on data and turn on event splitting, you MUST set the data period to the matching data set era.\n";
        IVYerr << "This is because event splitting in data is done based on RUN ranges, not EVENT ranges, and the correct run range needs to be picked up." << endl;
        assert(0);
      }

      // Assign the range over run numbers
      double const lumi_total = SampleHelpers::getIntegratedLuminosity(SampleHelpers::getDataPeriod());
      auto const& runnumber_lumi_pairs = SampleHelpers::getRunNumberLumiPairsForDataPeriod(SampleHelpers::getDataPeriod());
      int const nruns_total = runnumber_lumi_pairs.size();
      int const nruns_inc = static_cast<int>(static_cast<double>(nruns_total) / static_cast<double>(nchunks));
      int const nruns_rem = nruns_total - nruns_inc*nchunks;

      int const idx_firstRun = nruns_inc*ichunk + std::min(nruns_rem, ichunk);
      int const idx_firstRun_next = nruns_inc*(ichunk+1) + std::min(nruns_rem, ichunk+1);
      eventIndex_begin = (idx_firstRun>=nruns_total ? 0 : (int) runnumber_lumi_pairs.at(idx_firstRun).first);
      eventIndex_end = (idx_firstRun_next-1>=nruns_total || idx_firstRun_next<=idx_firstRun ? 0 : (int) runnumber_lumi_pairs.at(idx_firstRun_next-1).first);

      double lumi_acc = 0;
      for (auto const& runnumber_lumi_pair:runnumber_lumi_pairs){
        if (
          (eventIndex_begin<0 || eventIndex_begin<=(int) runnumber_lumi_pair.first)
          &&
          (eventIndex_end<0 || eventIndex_end>=(int) runnumber_lumi_pair.first)
          ) lumi_acc += runnumber_lumi_pair.second;
      }

      IVYout << "splitInputEventsIntoChunks: A real data loop will proceed. The requested run range is [" << eventIndex_begin << ", " << eventIndex_end << "]. Total luminosity covered will be " << lumi_acc << " / " << lumi_total << "." << endl;
    }
  }
}


#endif
