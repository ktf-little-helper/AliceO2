// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file MatCell.h
/// \brief Declarations for material properties of the cell (voxel)

#ifndef ALICEO2_MATCELL_H
#define ALICEO2_MATCELL_H

#include "AliTPCCommonRtypes.h"
#include "AliTPCCommonDef.h"

namespace o2
{
namespace Base
{

struct MatCell {
  // small struct to hold <X/X0> and <rho> of the voxel

  static constexpr int NParams = 2; // number of material parameters described
  float meanRho;                    ///< mean density, g/cm^3
  float meanX2X0;                   ///< fraction of radiaton lenght

  MatCell() : meanRho(0.f), meanX2X0(0.f) {}
  MatCell(const MatCell& src) : meanRho(src.meanRho), meanX2X0(src.meanX2X0) {}

  void scale(float scale)
  {
    meanRho *= scale;
    meanX2X0 *= scale;
  }

  ClassDefNV(MatCell, 1);
};

struct MatBudget : MatCell {

  // small struct to hold <X/X0>, <rho> and length traversed by track in the voxel
  static constexpr int NParams = 3; // number of material parameters described
  float length;                     ///< length in material

  MatBudget() : length(0.f) {}
  MatBudget(const MatBudget& src) : MatCell(src), length(src.length) {}

  void scale(float scale)
  {
    MatCell::scale(scale);
    length *= scale;
  }

  ClassDefNV(MatBudget, 1);
};

} // namespace Base
} // namespace o2

#endif