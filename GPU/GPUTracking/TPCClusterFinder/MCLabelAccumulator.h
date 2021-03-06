// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file MCLabelAccumulator.h
/// \author Felix Weiglhofer

#ifndef O2_GPU_MC_LABEL_ACCUMULATOR_H
#define O2_GPU_MC_LABEL_ACCUMULATOR_H

#include "clusterFinderDefs.h"
#include "Array2D.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include <bitset>
#include <vector>

namespace GPUCA_NAMESPACE
{
namespace dataformats
{
template <typename T>
class MCTruthContainer;
}

namespace gpu
{

class GPUTPCClusterFinder;
struct GPUTPCClusterMCInterim;

class MCLabelAccumulator
{

 public:
  MCLabelAccumulator(GPUTPCClusterFinder&);

  void collect(const ChargePos&, tpccf::Charge);

  bool engaged() const { return mLabels != nullptr && mOutput != nullptr; }

  void commit(tpccf::Row, uint, uint);

 private:
  using MCLabelContainer = o2::dataformats::MCTruthContainer<o2::MCCompLabel>;

  Array2D<const uint> mIndexMap;
  const MCLabelContainer* mLabels = nullptr;
  GPUTPCClusterMCInterim* mOutput = nullptr;

  std::bitset<64> mMaybeHasLabel;
  std::vector<o2::MCCompLabel> mClusterLabels;
};

} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
