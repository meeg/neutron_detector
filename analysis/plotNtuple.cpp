#include "plotNtuple.hh"

#include <stdarg.h>
#include <iostream>
#include <algorithm>
#include <queue>

#include <TTree.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TBranch.h>
#include <TH2D.h>
using namespace std;

int main ( int argc, char **argv ) {

    TString inname;
    char outfilename[100];

    //settings
    int nrows = 20;
    int ncols = 20;
    double hit_threshold = 0.1; //MeV threshold for a fiber hit

    double etot_threshold = 0.0; //MeV trigger threshold for total energy in all fibers
    int nhits_threshold = 0; //hit count threshold to plot the event
    unsigned int clustersize_threshold = 3; //cluster size threshold to plot the event


    int c;
    while ((c = getopt(argc,argv,"ho:r:c:t:")) !=-1)
        switch (c)
        {
            case 'h':
                printf("-h: print this help\n");
                printf("-o: use specified output filename\n");
                printf("-r: use specified number of rows (default %d)\n",nrows);
                printf("-c: use specified number of columns (default %d)\n",ncols);
                printf("-t: use specified hit energy threshold (default %f)\n",hit_threshold);
                return(0);
                break;
            case 'o':
                inname = optarg;
                break;
            case 'r':
                nrows = atoi(optarg);
                break;
            case 'c':
                ncols = atoi(optarg);
                break;
            case 't':
                hit_threshold = atof(optarg);
                break;
            //case 'n':
                //flip_channels = false;
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
    TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1100, 1000);
    c1->SetRightMargin(0.13);

    //TPad* pad1 = new TPad ("pad1","The pad",0.05,0.05,0.85,0.95);
    //pad1->Draw();
    //pad1->cd();
    
    // start the output PDF file
    sprintf(outfilename,"%s_events.pdf[",inname.Data());
    c1->Print(outfilename);
    TH2D* hist_coreE = new TH2D("coreE","energy deposition map",ncols,-0.5,ncols-0.5,nrows,-0.5,nrows-0.5);
    hist_coreE->GetXaxis()->SetTitle("Fiber column");
    hist_coreE->GetYaxis()->SetTitle("Fiber row");
    hist_coreE->GetZaxis()->SetTitle("Hit energy [MeV]");

    char title[200];

    // Get ntuple
    TTree* ntuple = (TTree*)f.Get("B4");

    //set up branches
    vector<double> *coreEVec = 0;
    TBranch *b_coreEVec = 0;
    ntuple->SetBranchAddress("EcoreVec", &coreEVec, &b_coreEVec);
    double coreETot;
    ntuple->SetBranchAddress("Ecore", &coreETot);

    vector<vector<int>*> clusterVector;
    queue<int> clustering_queue;

    for (Int_t indx = 0; indx < ntuple->GetEntries(); indx++) { // event loop
        ntuple->GetEntry(indx);
        for (vector<vector<int>*>::iterator it(clusterVector.begin());it != clusterVector.end(); it++) delete (*it); //garbage collect the old clusters
        clusterVector.clear();

        bool *is_clustered = new bool[nrows*ncols];
        int nHits = 0;
        unsigned int maxClusterSize = 0;
        hist_coreE->Reset();
        for (int ix = 0;ix<ncols;ix++) {
            for (int iy = 0;iy<nrows;iy++) {
                int ichannel = iy + nrows * ix;
                //rowNumber + fNofRows * colNumber
                if (coreEVec->at(ichannel) > hit_threshold) {
                    nHits++;
                }
                hist_coreE->Fill(ix,iy,coreEVec->at(ichannel));

                clustering_queue.push(ichannel);
                vector<int> *current_cluster = NULL;
                while (!clustering_queue.empty()) {
                    int newhit = clustering_queue.front();
                    clustering_queue.pop();
                    if (coreEVec->at(newhit) > hit_threshold && !is_clustered[newhit]) { //if the hit is above threshold and has not been clustered
                        if (!current_cluster) {
                            current_cluster = new vector<int>;
                            clusterVector.push_back(current_cluster);
                        }
                        current_cluster->push_back(newhit);
                        is_clustered[newhit] = true;

                        int newix = newhit/nrows;
                        int newiy = newhit%nrows;

                        // enqueue all the neighbors
                        if (newix>0) clustering_queue.push(newiy + nrows * (newix-1));
                        if (newix<ncols-1) clustering_queue.push(newiy + nrows * (newix+1));
                        if (newiy>0) clustering_queue.push((newiy-1) + nrows * newix);
                        if (newiy<nrows-1) clustering_queue.push((newiy+1) + nrows * newix);
                        if (newix>0 && newiy>0) clustering_queue.push((newiy-1) + nrows * (newix-1));
                        if (newix>0 && newiy<nrows-1) clustering_queue.push((newiy+1) + nrows * (newix-1));
                        if (newix<ncols-1 && newiy>0) clustering_queue.push((newiy-1) + nrows * (newix+1));
                        if (newix<ncols-1 && newiy<nrows-1) clustering_queue.push((newiy+1) + nrows * (newix+1));
                    }
                }
                if (current_cluster) {
                    if (current_cluster->size()>maxClusterSize) maxClusterSize = current_cluster->size();
                    //printf("cluster size %d\n",current_cluster->size());
                }
            }
            if (coreETot > etot_threshold && nHits >= nhits_threshold && maxClusterSize >= clustersize_threshold){
                sprintf(title,"hit map, total energy %f MeV, %d hits above %f MeV threshold, %ld clusters (largest has %d hits)",coreETot,nHits,hit_threshold,clusterVector.size(),maxClusterSize);
                hist_coreE->SetTitle(title);
                hist_coreE->SetMaximum(2.0);
                hist_coreE->Draw("colz");
                sprintf(outfilename,"%s_events.pdf",inname.Data());
                c1->Print(outfilename);
            }
        }



    }

    //finish the output PDF file
    sprintf(outfilename,"%s_events.pdf]",inname.Data());
    c1->Print(outfilename);
}  
