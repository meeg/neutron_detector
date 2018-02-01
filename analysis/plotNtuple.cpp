#include <stdarg.h>
#include <iostream>
#include <TTree.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TBranch.h>
#include <TH2D.h>
using namespace std;

int main ( int argc, char **argv ) {

    TString inname;
    char outfilename[100];
    int c;
    while ((c = getopt(argc,argv,"ho:")) !=-1)
        switch (c)
        {
            case 'h':
                printf("-h: print this help\n");
                printf("-o: use specified output filename\n");
                return(0);
                break;
            case 'o':
                inname = optarg;
                break;
            //case 'n':
                //flip_channels = false;
                //break;
            //case 'g':
                //force_cal_grp = true;
                //cal_grp = atoi(optarg);
                //break;
            case '?':
                printf("Invalid option or missing option argument; -h to list options\n");
                return(1);
            default:
                abort();
        }

    // Root file is the first and only arg
    if ( argc-optind != 1 ) {
        cout << "Usage: plotNtuple data_file\n";
        return(1);
    }

    // if output filename was not specified, set it based on the input filename
    if (inname=="")
    {
        inname=argv[optind];
        inname.ReplaceAll(".root","");
        if (inname.Contains('/')) { // write output to current directory, even if input file is elsewhere
            inname.Remove(0,inname.Last('/')+1);
        }
    }

    // Open file filled by Geant4 simulation 
    TFile f(argv[optind]);

    gROOT->Reset();
    gROOT->SetStyle("Plain");
    gStyle->SetOptStat(0);

    // Create a canvas and divide it into 2x2 pads
    TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);

    sprintf(outfilename,"%s_events.pdf[",inname.Data());
    c1->Print(outfilename);
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

    for (Int_t indx = 0; indx < ntuple->GetEntries(); indx++) {
        ntuple->GetEntry(indx);
        int nHits = 0;
        if (coreETot > 1.0) {
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
                hist_coreE->GetZaxis()->SetRange(0,2.0);
                sprintf(outfilename,"%s_events.pdf",inname.Data());
                c1->Print(outfilename);
            }
        }
    }
    sprintf(outfilename,"%s_events.pdf]",inname.Data());
    c1->Print(outfilename);
}  
