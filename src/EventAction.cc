//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: EventAction.cc 100946 2016-11-03 11:28:08Z gcosmo $
// 
/// \file EventAction.cc
/// \brief Implementation of the EventAction class

#include "EventAction.hh"
#include "CalorimeterSD.hh"
#include "CalorHit.hh"
#include "Analysis.hh"

#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4UnitsTable.hh"

#include "Randomize.hh"
#include <iomanip>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
 : G4UserEventAction(),
   fCladHCID(-1),
   fCoreHCID(-1)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CalorHitsCollection* 
EventAction::GetHitsCollection(G4int hcID,
                                  const G4Event* event) const
{
  auto hitsCollection 
    = static_cast<CalorHitsCollection*>(
        event->GetHCofThisEvent()->GetHC(hcID));
  
  if ( ! hitsCollection ) {
    G4ExceptionDescription msg;
    msg << "Cannot access hitsCollection ID " << hcID; 
    G4Exception("EventAction::GetHitsCollection()",
      "MyCode0003", FatalException, msg);
  }         

  return hitsCollection;
}    

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::PrintEventStatistics(
                              G4double cladEdep, G4double cladTrackLength,
                              G4double coreEdep, G4double coreTrackLength) const
{
  // print event statistics
  G4cout
     << "   Cladding: total energy: " 
     << std::setw(7) << G4BestUnit(cladEdep, "Energy")
     << "       total track length: " 
     << std::setw(7) << G4BestUnit(cladTrackLength, "Length")
     << G4endl
     << "        Core: total energy: " 
     << std::setw(7) << G4BestUnit(coreEdep, "Energy")
     << "       total track length: " 
     << std::setw(7) << G4BestUnit(coreTrackLength, "Length")
     << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event* /*event*/)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* event)
{  
  // Get hits collections IDs (only once)
  if ( fCladHCID == -1 ) {
    fCladHCID 
      = G4SDManager::GetSDMpointer()->GetCollectionID("CladdingHitsCollection");
    fCoreHCID 
      = G4SDManager::GetSDMpointer()->GetCollectionID("CoreHitsCollection");
  }

  // Get hits collections
  auto cladHC = GetHitsCollection(fCladHCID, event);
  auto coreHC = GetHitsCollection(fCoreHCID, event);

  // Get hit with total values
  auto cladHit = (*cladHC)[cladHC->entries()-1];
  auto coreHit = (*coreHC)[coreHC->entries()-1];
 
  // Print per event (modulo n)
  //
  auto eventID = event->GetEventID();
  auto printModulo = G4RunManager::GetRunManager()->GetPrintProgress();
  if ( ( printModulo > 0 ) && ( eventID % printModulo == 0 ) ) {
    G4cout << "---> End of event: " << eventID << G4endl;     

    PrintEventStatistics(
      cladHit->GetEdep(), cladHit->GetTrackLength() / cladHit->GetEdep(),
      coreHit->GetEdep(), coreHit->GetTrackLength() / coreHit->GetEdep());
  }  
  
  // Fill histograms, ntuple
  //
  fCladEdepVec.clear();
  fCoreEdepVec.clear();
  fCladTrackLengthVec.clear();
  fCoreTrackLengthVec.clear();

  for (int i=0;i<cladHC->entries();i++) {
      fCladEdepVec.push_back((*cladHC)[i]->GetEdep());
      fCladTrackLengthVec.push_back((*cladHC)[i]->GetTrackLength() / (*cladHC)[i]->GetEdep());
  }
  for (int i=0;i<coreHC->entries();i++) {
      fCoreEdepVec.push_back((*coreHC)[i]->GetEdep());
      fCoreTrackLengthVec.push_back((*coreHC)[i]->GetTrackLength() / (*coreHC)[i]->GetEdep());
  }

  // get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();
 
  // fill histograms
  analysisManager->FillH1(0, cladHit->GetEdep());
  analysisManager->FillH1(1, coreHit->GetEdep());
  analysisManager->FillH1(2, cladHit->GetTrackLength() / cladHit->GetEdep());
  analysisManager->FillH1(3, coreHit->GetTrackLength() / coreHit->GetEdep());
  
  // fill ntuple
  analysisManager->FillNtupleDColumn(0, cladHit->GetEdep());
  analysisManager->FillNtupleDColumn(1, coreHit->GetEdep());
  analysisManager->FillNtupleDColumn(2, cladHit->GetTrackLength() / cladHit->GetEdep());
  analysisManager->FillNtupleDColumn(3, coreHit->GetTrackLength() / coreHit->GetEdep());
  analysisManager->AddNtupleRow();  
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
