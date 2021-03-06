/// \file CheckTopologies.C
/// Macros to test the generation of a dictionary of topologies. Three dictionaries are generated: one with signal-cluster only, one with noise-clusters only and one with all the clusters.

#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <TAxis.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TNtuple.h>
#include <TString.h>
#include <TStyle.h>
#include <TTree.h>
#include <TStopwatch.h>
#include <fstream>
#include <string>

#include "MathUtils/Utils.h"
#include "ITSBase/GeometryTGeo.h"
#include "ITSMFTReconstruction/BuildTopologyDictionary.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include "DataFormatsITSMFT/ClusterTopology.h"
#include "DataFormatsITSMFT/TopologyDictionary.h"
#include "DataFormatsITSMFT/ROFRecord.h"
#include "ITSMFTSimulation/Hit.h"
#include "MathUtils/Cartesian3D.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "DetectorsCommonDataFormats/NameConf.h"
#include <unordered_map>

#endif

void CheckTopologies(std::string clusfile = "o2clus_its.root", std::string hitfile = "o2sim_HitsITS.root", std::string inputGeom = "")
{
  using namespace o2::base;
  using namespace o2::its;

  using o2::itsmft::BuildTopologyDictionary;
  using o2::itsmft::ClusterTopology;
  using o2::itsmft::CompClusterExt;
  using o2::itsmft::Hit;
  using ROFRec = o2::itsmft::ROFRecord;
  using MC2ROF = o2::itsmft::MC2ROFRecord;
  using HitVec = std::vector<Hit>;
  using MC2HITS_map = std::unordered_map<uint64_t, int>; // maps (track_ID<<16 + chip_ID) to entry in the hit vector

  std::vector<HitVec*> hitVecPool;
  std::vector<MC2HITS_map> mc2hitVec;

  const int QEDSourceID = 99; // Clusters from this MC source correspond to QED electrons

  TStopwatch sw;
  sw.Start();

  std::ofstream output_check("check_topologies.txt");

  // Geometry
  o2::base::GeometryManager::loadGeometry(inputGeom);
  auto gman = o2::its::GeometryTGeo::Instance();
  gman->fillMatrixCache(o2::utils::bit2Mask(o2::TransformType::T2L, o2::TransformType::T2GRot,
                                            o2::TransformType::L2G)); // request cached transforms

  // Hits
  TFile* fileH = nullptr;
  TTree* hitTree = nullptr;
  std::vector<Hit>* hitArray = nullptr;

  if (!hitfile.empty() && !gSystem->AccessPathName(hitfile.c_str())) {
    fileH = TFile::Open(hitfile.data());
    hitTree = (TTree*)fileH->Get("o2sim");
    hitTree->SetBranchAddress("ITSHit", &hitArray);
    mc2hitVec.resize(hitTree->GetEntries());
    hitVecPool.resize(hitTree->GetEntries(), nullptr);
  }

  // Clusters
  TFile* FileCl = TFile::Open(clusfile.data());
  TTree* clusTree = (TTree*)FileCl->Get("o2sim");
  std::vector<CompClusterExt>* clusArr = nullptr;
  clusTree->SetBranchAddress("ITSClusterComp", &clusArr);
  std::vector<unsigned char>* patternsPtr = nullptr;
  auto pattBranch = clusTree->GetBranch("ITSClusterPatt");
  if (pattBranch) {
    pattBranch->SetAddress(&patternsPtr);
  }

  // ROFrecords
  std::vector<ROFRec> rofRecVec, *rofRecVecP = &rofRecVec;
  clusTree->SetBranchAddress("ITSClustersROF", &rofRecVecP);

  // Cluster MC labels
  o2::dataformats::MCTruthContainer<o2::MCCompLabel>* clusLabArr = nullptr;
  std::vector<MC2ROF> mc2rofVec, *mc2rofVecP = &mc2rofVec;
  if (hitTree && clusTree->GetBranch("ITSClusterMCTruth")) {
    clusTree->SetBranchAddress("ITSClusterMCTruth", &clusLabArr);
    clusTree->SetBranchAddress("ITSClustersMC2ROF", &mc2rofVecP);
  }
  clusTree->GetEntry(0);

  Int_t nevCl = clusTree->GetEntries(); // clusters in cont. readout may be grouped as few events per entry
  Int_t nevH = 0;                       // hits are stored as one event per entry
  if (hitTree) {
    nevH = hitTree->GetEntries();
  }
  int ievC = 0, ievH = 0;
  int lastReadHitEv = -1;

  // Topologies dictionaries: 1) all clusters 2) signal clusters only 3) noise clusters only
  BuildTopologyDictionary completeDictionary;
  BuildTopologyDictionary signalDictionary;
  BuildTopologyDictionary noiseDictionary;

  for (ievC = 0; ievC < nevCl; ievC++) {
    clusTree->GetEvent(ievC);

    int nROFRec = (int)rofRecVec.size();
    std::vector<int> mcEvMin(nROFRec, hitTree ? hitTree->GetEntries() : 0), mcEvMax(nROFRec, -1);

    if (clusLabArr) { // >> build min and max MC events used by each ROF
      for (int imc = mc2rofVec.size(); imc--;) {
        const auto& mc2rof = mc2rofVec[imc];
        if (mc2rof.rofRecordID < 0) {
          continue; // this MC event did not contribute to any ROF
        }
        for (int irfd = mc2rof.maxROF - mc2rof.minROF + 1; irfd--;) {
          int irof = mc2rof.rofRecordID + irfd;
          if (mcEvMin[irof] > imc) {
            mcEvMin[irof] = imc;
          }
          if (mcEvMax[irof] < imc) {
            mcEvMax[irof] = imc;
          }
        }
      }
    } // << build min and max MC events used by each ROF

    for (int irof = 0; irof < nROFRec; irof++) {
      const auto& rofRec = rofRecVec[irof];
      rofRec.print();
      if (clusLabArr) { // >> read and map MC events contributing to this ROF
        for (int im = mcEvMin[irof]; im <= mcEvMax[irof]; im++) {
          if (!hitVecPool[im]) {
            hitTree->SetBranchAddress("ITSHit", &hitVecPool[im]);
            hitTree->GetEntry(im);
            auto& mc2hit = mc2hitVec[im];
            const auto* hitArray = hitVecPool[im];
            for (int ih = hitArray->size(); ih--;) {
              const auto& hit = (*hitArray)[ih];
              uint64_t key = (uint64_t(hit.GetTrackID()) << 32) + hit.GetDetectorID();
              mc2hit.emplace(key, ih);
            }
          }
        }
      } // << cache MC events contributing to this ROF

      auto pattIdx = patternsPtr->cbegin();
      for (int icl = 0; icl < rofRec.getNEntries(); icl++) {
        int clEntry = rofRec.getFirstEntry() + icl; // entry of icl-th cluster of this ROF in the vector of clusters
        // do we read MC data?
        const auto& cluster = (*clusArr)[clEntry];
        ClusterTopology topology;
        o2::itsmft::ClusterPattern pattern(pattIdx);
        topology.setPattern(pattern);
        //output_check << "iEv: " << ievC << " / " << nevCl << " iCl: " << clEntry << " / " <<  clusArr->size() << std::endl;
        // output_check << topology << std::endl;

        const auto locC = o2::itsmft::TopologyDictionary::getClusterCoordinates(cluster, pattern);
        float dx = BuildTopologyDictionary::IgnoreVal * 2, dz = BuildTopologyDictionary::IgnoreVal * 2; // to not use unassigned dx,dy
        if (clusLabArr) {
          const auto& lab = (clusLabArr->getLabels(clEntry))[0];
          auto srcID = lab.getSourceID();
          if (lab.isValid() && srcID != QEDSourceID) { // use MC truth info only for non-QED and non-noise clusters
            int trID = lab.getTrackID();
            const auto& mc2hit = mc2hitVec[lab.getEventID()];
            const auto* hitArray = hitVecPool[lab.getEventID()];
            Int_t chipID = cluster.getSensorID();
            uint64_t key = (uint64_t(trID) << 32) + chipID;
            auto hitEntry = mc2hit.find(key);
            if (hitEntry != mc2hit.end()) {
              const auto& hit = (*hitArray)[hitEntry->second];
              auto locH = gman->getMatrixL2G(chipID) ^ (hit.GetPos()); // inverse conversion from global to local
              auto locHsta = gman->getMatrixL2G(chipID) ^ (hit.GetPosStart());
              locH.SetXYZ(0.5 * (locH.X() + locHsta.X()), 0.5 * (locH.Y() + locHsta.Y()), 0.5 * (locH.Z() + locHsta.Z()));
              const auto locC = o2::itsmft::TopologyDictionary::getClusterCoordinates(cluster, pattern);
              dx = locH.X() - locC.X();
              dz = locH.Z() - locC.Z();
            } else {
              printf("Failed to find MC hit entry for Tr:%d chipID:%d\n", trID, chipID);
              continue;
            }
            signalDictionary.accountTopology(topology, dx, dz);
          } else {
            noiseDictionary.accountTopology(topology, dx, dz);
          }
        }
        completeDictionary.accountTopology(topology, dx, dz);
      } // Loop over clusters of a single ROF
    }   // loop over ROFs
  }     // loop over eventually multiple entries (TFs)

  auto dID = o2::detectors::DetID::ITS;

  completeDictionary.setThreshold(0.0001);
  completeDictionary.groupRareTopologies();
  completeDictionary.printDictionaryBinary(o2::base::NameConf::getDictionaryFileName(dID, "", ".bin"));
  completeDictionary.printDictionary(o2::base::NameConf::getDictionaryFileName(dID, "", ".txt"));
  completeDictionary.saveDictionaryRoot(o2::base::NameConf::getDictionaryFileName(dID, "", ".root"));

  TFile histogramOutput("histograms.root", "recreate");
  TCanvas* cComplete = new TCanvas("cComplete", "Distribution of all the topologies");
  cComplete->cd();
  cComplete->SetLogy();
  TH1F* hComplete = nullptr;
  o2::itsmft::TopologyDictionary::getTopologyDistribution(completeDictionary.getDictionary(), hComplete, "hComplete");
  hComplete->SetDirectory(0);
  hComplete->Draw("hist");
  hComplete->Write();
  cComplete->Write();
  TCanvas* cNoise = nullptr;
  TCanvas* cSignal = nullptr;
  TH1F* hNoise = nullptr;
  TH1F* hSignal = nullptr;

  if (clusLabArr) {
    noiseDictionary.setThreshold(0.0001);
    noiseDictionary.groupRareTopologies();
    noiseDictionary.printDictionaryBinary(o2::base::NameConf::getDictionaryFileName(dID, "noise", ".bin"));
    noiseDictionary.printDictionary(o2::base::NameConf::getDictionaryFileName(dID, "noise", ".txt"));
    noiseDictionary.saveDictionaryRoot(o2::base::NameConf::getDictionaryFileName(dID, "noise", ".root"));
    signalDictionary.setThreshold(0.0001);
    signalDictionary.groupRareTopologies();
    signalDictionary.printDictionaryBinary(o2::base::NameConf::getDictionaryFileName(dID, "signal", ".bin"));
    signalDictionary.printDictionary(o2::base::NameConf::getDictionaryFileName(dID, "signal", ".txt"));
    signalDictionary.saveDictionaryRoot(o2::base::NameConf::getDictionaryFileName(dID, "signal", ".root"));
    cNoise = new TCanvas("cNoise", "Distribution of noise topologies");
    cNoise->cd();
    cNoise->SetLogy();
    o2::itsmft::TopologyDictionary::getTopologyDistribution(noiseDictionary.getDictionary(), hNoise, "hNoise");
    hNoise->SetDirectory(0);
    hNoise->Draw("hist");
    histogramOutput.cd();
    hNoise->Write();
    cNoise->Write();
    cSignal = new TCanvas("cSignal", "cSignal");
    cSignal->cd();
    cSignal->SetLogy();
    o2::itsmft::TopologyDictionary::getTopologyDistribution(signalDictionary.getDictionary(), hSignal, "hSignal");
    hSignal->SetDirectory(0);
    hSignal->Draw("hist");
    histogramOutput.cd();
    hSignal->Write();
    cSignal->Write();
  }
  sw.Print();
}
