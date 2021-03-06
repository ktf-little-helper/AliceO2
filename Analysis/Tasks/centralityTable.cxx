// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "Analysis/Multiplicity.h"
#include "Analysis/Centrality.h"
#include "TFile.h"
#include "TH1F.h"

using namespace o2;
using namespace o2::framework;

struct CentralityTableTask {
  Produces<aod::Cents> cent;
  TH1F* hCumMultV0M;

  void init(InitContext&)
  {
    // TODO read multiplicity histos from CCDB
    TFile f("multiplicity.root");
    TH1F* hMultV0M = (TH1F*)f.Get("multiplicity/hMultV0M");
    // TODO produce cumulative histos in the post processing macro
    hCumMultV0M = (TH1F*)hMultV0M->GetCumulative(false);
    hCumMultV0M->Scale(100. / hCumMultV0M->GetMaximum());
  }

  void process(aod::Mult const& mult)
  {
    float centV0M = hCumMultV0M->GetBinContent(hCumMultV0M->FindFixBin(mult.multV0M()));
    LOGF(info, "centV0M=%.0f", centV0M);
    // fill centrality columns
    cent(centV0M);
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const&)
{
  return WorkflowSpec{
    adaptAnalysisTask<CentralityTableTask>("centrality-table")};
}
