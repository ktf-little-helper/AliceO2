// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//  TRD calibration class for parameters which are stored pad wise (1.2M entries) //
//  2019 - Ported from various bits of AliRoot (SHTM)                             //
//  originally in TRD run2 parlance, this would be a AliTRDCalPad instantiated    //
//  as a LocalT0                                                                  //
////////////////////////////////////////////////////////////////////////////////////

#include "TRDBase/LocalGainFactor.h"
using namespace o2::trd;
