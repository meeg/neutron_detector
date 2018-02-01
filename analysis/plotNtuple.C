// ROOT macro file for plotting example B4 ntuple
// 
// Can be run from ROOT session:
// root[0] .x plotNtuple.C

{
  gROOT->Reset();
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  
  // Draw histos filled by Geant4 simulation 
  //   

  // Open file filled by Geant4 simulation 
  TFile f("B4.root");

  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Print("blah.pdf[");
  //c1->Divide(2,2);
  TH2D* hist_coreE = new TH2D("coreE","energy deposition map",100,-0.5,99.5,100,-0.5,99.5);

  char title[100];
  
  // Get ntuple
  TTree* ntuple = (TTree*)f.Get("B4");
  std::vector<double> *coreEVec = 0;
  TBranch *b_coreEVec = 0;
  ntuple->SetBranchAddress("EgapVec", &coreEVec, &b_coreEVec);
  double coreETot;
  ntuple->SetBranchAddress("Egap", &coreETot);
  // Draw Eabs histogram in the pad 1
  for (Int_t indx = 0; indx < ntuple->GetEntries(); indx++) {
      ntuple->GetEntry(indx);
      int nHits = 0;
      if (coreETot > 0) {
          hist_coreE->Reset();
          sprintf(title,"energy deposition map, total energy %f",coreETot);
          hist_coreE->SetTitle(title);
          for (int ix = 0;ix<100;ix++) {
              for (int iy = 0;iy<100;iy++) {
                  if (coreEVec->at(ix+100*iy)!=0) nHits++;
                  hist_coreE->Fill(ix,iy,coreEVec->at(ix+100*iy));
              }
          }
          if (nHits>1){
              hist_coreE->Draw("colz");
              c1->Print("blah.pdf");
          }
      }
  }
  c1->Print("blah.pdf]");
}  
