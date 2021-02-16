#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>

class MyMainFrame {
   RQ_OBJECT("MyMainFrame")
private:
   TGMainFrame       *fMain;
   TGNumberEntry     *inputThres;
   TCanvas           *c1;
public:
   MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
   virtual ~MyMainFrame();
   void DoDraw();
   void DoSave();
};
MyMainFrame::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h) {
   // Create a main frame
   fMain = new TGMainFrame(p,w,h);

   // Create a horizontal frame widget with buttons
   TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,800,40);

   // input box for energy threshold
   inputThres = new TGNumberEntry(hframe, 100, 5);
   hframe->AddFrame(inputThres, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

   // draw button
   TGTextButton *draw = new TGTextButton(hframe,"&Draw");
   draw->Connect("Clicked()","MyMainFrame",this,"DoDraw()");
   hframe->AddFrame(draw, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

   // save button
   TGTextButton *save = new TGTextButton(hframe,"&Save");
   save->Connect("Clicked()","MyMainFrame",this,"DoSave()");
   hframe->AddFrame(save, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

   // exit button
   TGTextButton *exit = new TGTextButton(hframe,"&Exit","gApplication->Terminate(0)");
   hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

   fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX, 2,2,2,2));

   // Set a name to the main frame
   fMain->SetWindowName("Input Threshold");

   // Map all subwindows of main frame
   fMain->MapSubwindows();

   // Initialize the layout algorithm
   fMain->Resize(fMain->GetDefaultSize());

   // Map main frame
   fMain->MapWindow();
}

void MyMainFrame::DoDraw() {
   FILE *fp;                                   // reads file
   Long_t thres = inputThres->GetIntNumber();  // energy threshold 
   int nevent;                                 // # of total events
   int adcX, adcY;                             // adc value of each strip at X, Y direction
   int hitX, hitY, hitadcX, hitadcY;

   // open file
   if ((fp = fopen("./data/apv_2021020803.txt", "rt")) == NULL)
   {
     printf("Problem with opening file...\n");
     return;
   }

   // create 2D histogram
   if (gROOT->FindObject("2D_histo")) delete gROOT->FindObject("2D_histo");
   
   const char* histoName = ("Hit Distribution (Threshold = " + std::to_string(thres) + ")").c_str();
   TH2F *histo = new TH2F("2D_histo", histoName, 256, 0, 256, 256, 0, 256);

   histo->SetMarkerColor(kRed);
   histo->SetMarkerStyle(20);
   histo->Reset();

   fscanf(fp, "%d", &nevent);
   for (int i = 0; i < nevent; i++)
   {
    hitadcX = -1000;
    hitX = -1;
    for (int ch = 0; ch < 256; ch++)
    {
      fscanf(fp, "%d", &adcX);
      if ((adcX > thres/2) && (adcX > hitadcX))
      {
        hitadcX = adcX;
        hitX = ch;
      }
    }

    hitadcY = -1000;
    hitY = -1;
    for (int ch = 0; ch < 256; ch++)
    {
      fscanf(fp, "%d", &adcY);
      if ((adcY > thres) && (adcY > hitadcY))
      {
        hitadcY = adcY;
        hitY = ch;
      }
    }
    if ((hitX > -1) && (hitY > -1)) histo->Fill(hitX, hitY); // Fill Hit
   }
   fclose(fp);

   // draw on canvas
   if (gROOT->FindObject("c1")) delete gROOT->FindObject("c1");
   c1 = new TCanvas("c1", "Cosmic Ray Muon Detection with GEM", 800, 800);
  
   histo->Draw("P");

   gStyle->SetPalette(kRainBow);
   gStyle->SetOptStat(10);

   c1->Modified();
   c1->Update();
}

void MyMainFrame::DoSave() {
   const char* savedFileName = ("thres_"+std::to_string(inputThres->GetIntNumber())+".pdf").c_str();
   c1->SaveAs(savedFileName);
}

MyMainFrame::~MyMainFrame() {
   // Clean up used widgets: frames, buttons, layout hints
   fMain->Cleanup();
   delete fMain;
}

void hits2D() {
   // Popup the GUI...
   new MyMainFrame(gClient->GetRoot(),800,40);
}
