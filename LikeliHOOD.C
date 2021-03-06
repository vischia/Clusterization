#include "LikeliHOOD.h"
#include "tdrstyle.C"
#include <TFile.h>
#include "TROOT.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include "TRandom3.h"
#include "TMath.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "THStack.h"
#include "TCanvas.h"
#include <sstream>
#include <TString.h>
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"
#include <TLegend.h>
#include <TLatex.h>
#include <stdlib.h>

std::ostream &operator<<(std::ostream &os, Point const &point) {
  return os << "(" << point.fX << ", " << point.fY << ")";
}

LikeliHOOD::LikeliHOOD(TString inputFile, TString outputTag, Int_t nTargetBins, Int_t nLep, TString fileType, Int_t trial):
  inputFile_(inputFile),
  outputTag_(outputTag),
  nTargetBins_(nTargetBins),
  nLep_(nLep),
  trial_(trial),
  fileType(fileType)
{
  ReadFromFiles();
  StartTheThing();

}

LikeliHOOD::LikeliHOOD(TString iDir, Int_t nTargetBins, Int_t nLep, TString fileType, Int_t trial):
  inputFile_(""),
  iDir_(iDir),
  outputTag_(""),
  nTargetBins_(nTargetBins),
  nLep_(nLep),
  trial_(trial),
  fileType(fileType)
{
  ReadFromFiles();
  StartTheThing();

}

void LikeliHOOD::ReadFromFiles()
{
  if(inputFile_!="")
    {
      readFromRootFiles();
    }
  else if ( fileType == "txt"){
    readFromTxTFiles();
  }
  else if (fileType == "root"){
    readFromRootFiles();
  }
  else{
    cout << "[LikeliHOOD] Please select a correct file type" << endl;
    cout << "Script will crash in..." << endl;
    cout << "...3" << endl;
    sleep(1);
    cout << "...2" << endl;
    sleep(1);
    cout << "...1" << endl;
    sleep(1);
  }
}

void LikeliHOOD::StartTheThing()
{
  MakeLikeliHood();
  GETCUM();
  StoreToFile();
}


void LikeliHOOD::readFromRootFiles()
{
  // Todo:
  // Change file and branch names
  fTTbar.clear(); fTTH.clear(); fTTW.clear();
  TFile* f    = 0;
  TTree* tree = 0;
  TBranch *bX = 0;
  TBranch *bY = 0;
  TBranch *bW = 0;

  Float_t x = 0.;
  Float_t y = 0.;
  Float_t w = 0.;

  if(inputFile_=="")
    {
      bool MEM(true);
      TString tree3l( MEM ? "2017-02-28_12.00/trees_for_opt_binning_3l_v6_withMEM" : "2017-02-23_22.07/trees_for_opt_binning_3l_v6");
      
      TString treesVersion(nLep_==3 ? tree3l :  "2017-02-23_22.07/trees_for_opt_binning_2lss_v6");
      
      TString memString(MEM ? "_withMEM" : "");
      
      TString kinTTB(nLep_==3 ? "kinMVA_3l_ttbar"       : "kinMVA_2lss_ttbar_withBDTv8");
      TString kinTTV(nLep_==3 ? Form("kinMVA_3l_ttV%s",memString.Data()) : "kinMVA_2lss_ttV_withHj"     );
      
      TString treeName("t");
      
      //cout << "[LikeliHOOD] Opening file: " << Form("data/%s/ev_%s_TT_FR_TT.root", treesVersion.Data(), (nLep_==3 ? "3l" : "2lss" )) << endl;
      cout << "[LikeliHOOD] Opening file: " << "data/"<< iDir_ << "/outfile_" << (nLep_==3 ? "3l" : "2lss") << "_TT.root" << endl;
      // Reading ttbar
      ///// f = TFile::Open( Form("data/%s/ev_%s_TT_FR_TT.root", treesVersion.Data(), (nLep_==3 ? "3l" : "2lss" )), "READ" );
      f = TFile::Open( "data/" + iDir_ + "/outfile_" + (nLep_==3 ? "3l" : "2lss") + "_TT.root");
      tree = (TTree*) f->Get(treeName);
      //// tree->SetBranchAddress(kinTTB, &x);
      //// tree->SetBranchAddress((nLep_==3 ? "kinMVA_3l_ttV" : kinTTV), &y); 
      tree->SetBranchAddress((nLep_==3 ? "kinMVA_3l_ttbar" : "kinMVA_2lss_ttbar_withBDTrTT"), &x );
      tree->SetBranchAddress((nLep_==3 ? "kinMVA_3l_ttV"   : "kinMVA_2lss_ttV_withHj_rTT"  ), &y );
      tree->SetBranchAddress("_weight_", &w);
      cout << "weight" << endl;
      for (int entr = 0; entr < tree->GetEntries(); entr++){
        tree->GetEntry(entr);
        Point point = Point(x,(nLep_==3 ? 0.04804 + 0.6902*y : y),2*w); // Result of temporary fit for average dependence
        ///// cout << "x, y, w: " << x << ", " << y << ", " << w << endl;
        if (entr % 2 == 0)
          fTTbar.push_back(point);
        else
          fTTbarMC.push_back(point);
      }
      f->Close();
      cout << "TTbar events " << fTTbar.size() << endl;
      
      
      // Reading ttH
      ///// f = TFile::Open(Form("data/%s/ev_%s_TTHnobb_pow.root", treesVersion.Data(), (nLep_==3 ? "3l" : "2lss" )), "READ" );
      f = TFile::Open( "data/" + iDir_ + "/outfile_" + (nLep_==3 ? "3l" : "2lss") + "_TTHnobb_pow.root");
      tree = (TTree*) f->Get(treeName);
      ///// tree->SetBranchAddress(kinTTB, &x);
      ///// tree->SetBranchAddress(kinTTV, &y);
      tree->SetBranchAddress((nLep_==3 ? "kinMVA_3l_ttbar" : "kinMVA_2lss_ttbar_withBDTrTT"), &x );
      tree->SetBranchAddress((nLep_==3 ? "kinMVA_3l_ttV"   : "kinMVA_2lss_ttV_withHj_rTT"  ), &y );
      tree->SetBranchAddress("_weight_", &w);
      for (int entr = 0; entr < tree->GetEntries(); entr++){
        tree->GetEntry(entr);
        Point point = Point(x,y,2*w);
        ///// cout << "x, y, w: " << x << ", " << y << ", " << w << endl;
        if (entr % 2 == 0)
          fTTH.push_back(point);
        else
          fTTHMC.push_back(point);
      }
      f->Close();
      
      if(nLep_==3 && false)
        {
          // Reading ttH ext
          f = TFile::Open(Form("data/%s/ev_%s_TTHnobb_pow_offset1.root", treesVersion.Data(), (nLep_==3 ? "3l" : "2lss" )), "READ" );
          tree = (TTree*) f->Get(treeName);
          tree->SetBranchAddress(kinTTB, &x);
          tree->SetBranchAddress(kinTTV, &y);
          tree->SetBranchAddress("_weight_", &w);
          for (int entr = 0; entr < tree->GetEntries(); entr++){
            tree->GetEntry(entr);
            Point point = Point(x,y,2*w);
            ///// cout << "x, y, w: " << x << ", " << y << ", " << w << endl;
            if (entr % 2 == 0)
              fTTH.push_back(point);
            else
              fTTHMC.push_back(point);
          }
          f->Close();
        }
      cout << "TTH events " << fTTH.size() << endl;

      // Reading ttV
      ///// f = TFile::Open(Form("data/%s/ev_%s_TTV.root", treesVersion.Data(), (nLep_==3 ? "3l" : "2lss" )), "READ");
      f = TFile::Open( "data/" + iDir_ + "/outfile_" + (nLep_==3 ? "3l" : "2lss") + "_TTV.root");
      tree = (TTree*) f->Get(treeName);
      ///// tree->SetBranchAddress(kinTTB, &x);
      ///// tree->SetBranchAddress(kinTTV, &y);
      tree->SetBranchAddress((nLep_==3 ? "kinMVA_3l_ttbar" : "kinMVA_2lss_ttbar_withBDTrTT"), &x );
      tree->SetBranchAddress((nLep_==3 ? "kinMVA_3l_ttV"   : "kinMVA_2lss_ttV_withHj_rTT"  ), &y );
      tree->SetBranchAddress("_weight_", &w);
      for (int entr = 0; entr < tree->GetEntries(); entr++){
        tree->GetEntry(entr);
        Point point = Point(x,y,2*w);
        ///// cout << "x, y, w: " << x << ", " << y << ", " << w << endl;
        if (entr % 2 == 0)
          fTTW.push_back(point);
        else
          fTTWMC.push_back(point);
      }
      f->Close();
      
      if(nLep_==3 && false)
        {
          // Reading ttV ext
          f = TFile::Open(Form("data/%s/ev_%s_TTV_offset1.root", treesVersion.Data(), (nLep_==3 ? "3l" : "2lss" )), "READ");
          tree = (TTree*) f->Get(treeName);
          tree->SetBranchAddress(kinTTB, &x);
          tree->SetBranchAddress(kinTTV, &y);
          tree->SetBranchAddress("_weight_", &w);
          for (int entr = 0; entr < tree->GetEntries(); entr++){
            tree->GetEntry(entr);
            Point point = Point(x,y,2*w);
            ///// cout << "x, y, w: " << x << ", " << y << ", " << w << endl;
            if (entr % 2 == 0)
              fTTW.push_back(point);
            else
              fTTWMC.push_back(point);
          }
          f->Close();
        }
      cout << "TTW events " << fTTW.size() << endl;
    }
  else // Charged Higgs
    {
      // Reading ttbar
      cout << "[LikeliHOOD] Input file given (chhiggs case). Opening file " << inputFile_ << "..." << endl;
      f = TFile::Open(inputFile_, "READ");
      tree = (TTree*) f->Get("tOu");
      
      Int_t issig;
      
      auto b_HF    = tree->GetBranch("HF"    ); b_HF   ->SetAddress( &x    );
      auto b_LF    = tree->GetBranch("LF"    ); b_LF   ->SetAddress( &y    );
      auto b_wgt   = tree->GetBranch("wgt"   ); b_wgt  ->SetAddress( &w    );
      auto b_issig = tree->GetBranch("issig" ); b_issig->SetAddress( &issig);
      
      auto nentries = tree->GetEntries();
      
      vector<Point> tempSig;
      vector<Point> tempBkg;

      for (Int_t entr = 0; entr < nentries; ++entr){
        
        b_HF    ->GetEvent(entr);
        b_LF    ->GetEvent(entr);
        b_wgt   ->GetEvent(entr);
        b_issig ->GetEvent(entr);
        
        tree->GetEntry(entr);
        Point point = Point(x,y,w);
        
        if(issig==1) tempSig.push_back(point);
        else if(issig==0) tempBkg.push_back(point);
      }
      
      auto nsig = tempSig.size();
      auto nbkg = tempBkg.size();
      
      for(Int_t iSig=0; iSig<nsig; ++iSig)
        { 
          if (iSig % 2 == 0)
            {
              fTTH.push_back(tempSig[iSig]);
            }
          else
            {
              fTTHMC.push_back(tempSig[iSig]);
            }
        }
      for(Int_t iBkg=0; iBkg<nbkg; ++iBkg)
        {
          if (iBkg % 2 == 0)
            {
              fTTbar.push_back(tempBkg[iBkg]);
            }
          else
            {
              fTTbarMC.push_back(tempBkg[iBkg]);
            }
        }
      
      f->Close();
    }
}

void LikeliHOOD::readFromTxTFiles()
{
  cout << "WARNING!!!! READING FROM OBSOLETE TXT FILES" << endl;
  return;

  fTTbar.clear(); fTTH.clear(); fTTW.clear();
  ifstream f;
  nLep_==3 ? f.open("data/ttbar3l.txt") : f.open("data/ttbar.txt");
  Int_t count = 0;
  while (f){
    Double_t x = 0; Double_t y = 0; Double_t w = 0;
    f >> y >> x >> w; //
    if ( TMath::Abs(x) > 2.) continue;
    if ( TMath::Abs(y) > 2.) continue;
    Point point = Point(x,y,2*w);
    if (count%2 == 0)
      fTTbar.push_back(point);
    else
      fTTbarMC.push_back(point);
    count++;
  }
  f.close();
  cout << "TTbar events " << fTTbar.size() << endl;

  nLep_==3 ? f.open("data/tth3l.txt") : f.open("data/tth.txt");
  while (f){
    Double_t x(0), y(0), w(0), z(0);
    f >> y >> x >> w >> z;
    if (w == 0) continue;
    if ( TMath::Abs(x) > 2.) continue;
    if ( TMath::Abs(y) > 2.) continue;

    w *= z;
    Point point = Point(x,y,2*w);
    if (count%2 == 0)
      fTTH.push_back(point);
    else
      fTTHMC.push_back(point);
    count++;
  }
  f.close();
  cout << "TTH events " << fTTH.size() << endl;

  nLep_==3 ? f.open("data/ttv3l.txt") : f.open("data/ttv.txt");
  while (f){
    Double_t x = 0; Double_t y = 0; Double_t w = 0;
    f >> y >> x >> w;
    if (w == 0) continue;
    if ( TMath::Abs(x) > 2.) continue;
    if ( TMath::Abs(y) > 2.) continue;

    Point point = Point(x,y,2*w);
    if (count%2 == 0)
      fTTW.push_back(point);
    else
      fTTWMC.push_back(point);
    count++;
  }
  f.close();
  cout << "TTW events " << fTTW.size() << endl;
}

void LikeliHOOD::MakeLikeliHood()
{

  Int_t initialSplitting(nLep_==3 ? 10 : 20);
  if(inputFile_!="") initialSplitting=20;

  hSig = new TH2F("hSig","",initialSplitting,-1.,1.,initialSplitting,-1.,1.);
  hBkg = new TH2F("hBkg","",initialSplitting,-1.,1.,initialSplitting,-1.,1.);
  
  for (auto& pt : fTTbar){
    hBkg->Fill(pt.fX, pt.fY, pt.fW);
  }
  for (auto& pt : fTTW){
    hBkg->Fill(pt.fX, pt.fY, pt.fW);
  }
  for (auto& pt : fTTH){
    hSig->Fill(pt.fX, pt.fY, pt.fW);
  }

  hSig->Scale( 1. / hSig->GetMaximum());
  hBkg->Scale( 1. / hBkg->GetMaximum());
  
  for (int ix = 1; ix < hSig->GetXaxis()->GetNbins()  +1; ++ix){
    for (int iy = 1; iy < hSig->GetYaxis()->GetNbins()+1; ++iy){
      int bin = hSig->GetBin(ix,iy);
      //hSig->SetBinContent(bin, TMath::Max(1e-5,hSig->GetBinContent(bin)));
      //hBkg->SetBinContent(bin, TMath::Max(1e-5,hBkg->GetBinContent(bin)));
      hSig->SetBinContent(bin, TMath::Max(0.,hSig->GetBinContent(bin)));
      hBkg->SetBinContent(bin, TMath::Max(0.,hBkg->GetBinContent(bin)));
    }
  }

  TCanvas* c1 = new TCanvas();
  hSig->Smooth(1,"k5b");
  hSig->Draw("colz");
  TCanvas* c2 = new TCanvas();
  hBkg->Smooth(1,"k5b");
  hBkg->Draw("colz");

  hRtio = (TH2F*) hSig->Clone("hRtio");
  hRtio->Divide(hBkg);
  TCanvas* c3 = new TCanvas();
  gPad->SetLogz();
  hRtio->Draw("colz");
  

  return;
}

Double_t LikeliHOOD::GetLikeLiHood(Point pt){
  //cout << "Getting likelihood " << pt << " " << hRtio->GetBinContent(hRtio->FindBin(pt.fX,pt.fY)) << endl;
  return hRtio->GetBinContent(hRtio->FindBin(pt.fX,pt.fY));
}

void LikeliHOOD::GETCUM() // cum for cumulative
{
  TH1F* h = new TH1F("h","",1000,0,10.);
  for (auto& pt : fTTbar){
    h->Fill(GetLikeLiHood(pt));
  }
  for (auto& pt : fTTW){
    h->Fill(GetLikeLiHood(pt));
  }
  TCanvas* c = new TCanvas();
  h->Scale( 1 / h->Integral());
  h->GetCumulative()->Draw();
  int nq(nLep_==3 ? 5 : 8);
  if(nTargetBins_!=0) nq=nTargetBins_;
  Double_t xq[nq+1]; Double_t yq[nq+1];
  for (Int_t i=0;i<nq;i++) xq[i] = Float_t(i)/nq;
  xq[nq] = 0.99999;
  h->GetQuantiles(nq+1,yq,xq);
  // cout << nq << endl;
  // for (Int_t i=0;i<nq+1;i++) cout << xq[i] << " " << yq[i] << endl;
				
  // cout << endl;
  hAuxHisto = new TH1F("hAuxHisto","",nq,yq);
  // hAuxHisto->FillRandom("gaus",100000);
  // hAuxHisto->Draw();

  c->Print(Form("cumulative%s_%s.pdf", ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));
  c->Print(Form("cumulative%s_%s.png", ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));
  c->Print(Form("cumulative%s_%s.root", ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));

  

  TFile* cumulativeStore = TFile::Open(Form("cumulative%s_%dl.root", ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), nLep_),"recreate");
  h->GetCumulative()->Write();
  cumulativeStore->Close();

  return; 
}


Double_t LikeliHOOD::GetCluster(Point pt)
{
  Int_t bin = hAuxHisto->FindBin(GetLikeLiHood(pt))-1;
  if (bin < 0) return 0;
  //  if (bin > hAuxHisto->GetNbinsX()) return hAuxHisto->GetNbinsX();
  if (bin+1 > hAuxHisto->GetNbinsX()) return hAuxHisto->GetNbinsX()-1;
  return bin;
}

void LikeliHOOD::Test()
{
  TCanvas* c = new TCanvas();
  c->cd();
  setTDRStyle();

  TH1F* hTTbar = new TH1F("hTTbar","",hAuxHisto->GetNbinsX(), -0.5, hAuxHisto->GetNbinsX()-0.5);
  TH1F* hTTW   = new TH1F("hTTW"  ,"",hAuxHisto->GetNbinsX(), -0.5, hAuxHisto->GetNbinsX()-0.5);
  TH1F* hTTH   = new TH1F("hTTH"  ,"",hAuxHisto->GetNbinsX(), -0.5, hAuxHisto->GetNbinsX()-0.5);
  THStack* mc  = new THStack("mc","mc");
  vector<Point>::iterator point;
  cout << "TTbar size is " << fTTbarMC.size() << endl;
  for (point = fTTbarMC.begin(); point != fTTbarMC.end(); ++point)
    hTTbar->Fill( GetCluster( *point), point->fW);
  for (point = fTTWMC.begin(); point != fTTWMC.end(); ++point)
    hTTW->Fill( GetCluster( *point), point->fW);
  for (point = fTTHMC.begin(); point != fTTHMC.end(); ++point)
    hTTH->Fill( GetCluster( *point), point->fW);
  cout << hTTbar->Integral() << " " << hTTbar->GetEntries() << endl;
  hTTbar->SetFillColor( kRed     );
  hTTH->SetFillColor( kBlue    );
  hTTW->SetFillColor( kMagenta );

  mc->Add( hTTbar ); mc->Add(hTTW); mc->Add(hTTH);
  mc->Draw("HIST");
  mc->SetMaximum(1.5* mc->GetMaximum());
  mc->GetHistogram()->GetYaxis()->SetTitle("Expected events/bin");
  mc->GetHistogram()->GetXaxis()->SetTitle("Bin in the bdt1#times bdt2 plane");
  mc->GetHistogram()->GetXaxis()->SetTitleSize(0.05);
  mc->GetHistogram()->GetXaxis()->SetTitleOffset(1.1);
  mc->GetHistogram()->GetYaxis()->SetTitleSize(0.05);
  mc->GetHistogram()->GetYaxis()->SetTitleOffset(1.1);

  TLegend* l = new TLegend(0.7,0.8,0.9,0.9);
  l->AddEntry(hTTH  , "ttH signal", "f");
  l->AddEntry(hTTW  , "ttV"       , "f");
  l->AddEntry(hTTbar, "tt"        , "f");
  l->Draw();

  TLatex latex;
  latex.SetTextSize(0.05);
  latex.SetTextAlign(13);  //align at top
  latex.SetTextFont(62);
  latex.DrawLatexNDC(.1,.95,"CMS Simulation");
  latex.DrawLatexNDC(.7,.95,"#it{36 fb^{-1}}");

  c->Modified();
  c->Update();

  TFile* histofile = TFile::Open(Form("likelihoodBased%s_1d_%s_histos.root",  ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")) , "RECREATE");
  histofile->cd();
  hTTbar->Write();
  hTTH->Write();
  histofile->Close();
  
  c->Print(Form("likelihoodBased%s_1d_%s.pdf",  ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));
  c->Print(Form("likelihoodBased%s_1d_%s.png",  ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));
  c->Print(Form("likelihoodBased%s_1d_%s.root",  ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));
}

Int_t LikeliHOOD::SortedThing(Int_t bin)
{
  for (unsigned int k = 0; k < SoverB.size(); ++k){
    if ( SoverB[k].second == bin ) return k;
  }
  cout << "[SortedThing::WTF]" << endl;
  return -1;
}

void LikeliHOOD::StoreToFile()
{
  TFile* binning = TFile::Open(Form("binning%s_%dl.root", ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), nLep_),"recreate");
  hTargetBinning = new TH2F("hTargetBinning","",100,-1.,1.,100,-1.,1.);
  for (int ix = 1; ix < hTargetBinning->GetXaxis()->GetNbins()  +1; ++ix){
    for (int iy = 1; iy < hTargetBinning->GetYaxis()->GetNbins()+1; ++iy){
      int bin = hTargetBinning->GetBin(ix,iy);
      int k   = GetCluster( Point(hTargetBinning->GetXaxis()->GetBinCenter(ix),
				  hTargetBinning->GetYaxis()->GetBinCenter(iy),-1));
      ///// cout << hTargetBinning->GetXaxis()->GetBinCenter(ix) << " "
      /////   << hTargetBinning->GetYaxis()->GetBinCenter(iy) << " " << k << endl;
      hTargetBinning->SetBinContent(bin,k);
    }
  }
  hTargetBinning->Write();
  binning->Close();
}

void LikeliHOOD::VoronoiPlot()
{
  vector<TGraph*> graphs; graphs.clear();
  vector<Double_t>* X = new vector<Double_t>[hAuxHisto->GetNbinsX()+1];
  vector<Double_t>* Y = new vector<Double_t>[hAuxHisto->GetNbinsX()+1];

  for (Double_t x = -1; x < 1.; x = x + 1e-3){
      for (Double_t y = -1; y < 1.; y = y + 1e-3){
	Int_t k = GetCluster(Point(x,y,-1));
	///cout << k << endl;
	X[k].push_back(x);
	Y[k].push_back(y);
	// cout << "Pushed " << endl;
      }
  }
  
  cout << "Done " << endl;



  TCanvas* c = new TCanvas();
  c->cd();
  setTDRStyle();

  TH1F* hDummy = new TH1F("hDummy","",2,-1,1);
  hDummy->SetBinContent(1, 1.);
  hDummy->SetBinContent(2,-1.);
  hDummy->SetLineColor(kWhite);
  hDummy->GetYaxis()->SetRangeUser(-1.,1.);
  hDummy->GetXaxis()->SetTitle("BDT(ttH,tt)");
  hDummy->GetYaxis()->SetTitle("BDT(ttH,ttV)");
  hDummy->Draw();
  cout << "Done... now plotting" << endl;

  TText t;
  t.SetTextSize(0.08);
  for (unsigned int k = 0; k < hAuxHisto->GetNbinsX(); ++k){
    graphs.push_back(new TGraph( X[k].size(), &X[k][0], &Y[k][0] ));
    graphs[k]->SetMarkerColor(k);
    graphs[k]->SetMarkerStyle(6);
    graphs[k]->Draw("PSAME");

    //t.SetTextColor(k+1);
    //    t.DrawText(fCentroids[k].fX, fCentroids[k].fY, Form("%d",k));

  }

  c->Print(Form("likelihoodBased%s_2d_%s.pdf",  ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));
  c->Print(Form("likelihoodBased%s_2d_%s.png",  ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));
  c->Print(Form("likelihoodBased%s_2d_%s.root",  ((outputTag_=="") ? "" : TString("_"+outputTag_).Data()), (nLep_==3 ? "3l" : "2lss")));

}
