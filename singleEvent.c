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
   TGNumberEntry     *inputEvent;
   TGCheckButton     *rawADC;
   TGCheckButton     *pedSubADC;
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

   // option for drawing raw or pedestal-subtracted adc
   rawADC = new TGCheckButton(hframe, "Raw ADC",1);
   rawADC->SetState(kButtonUp);
   hframe->AddFrame(rawADC, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

   pedSubADC = new TGCheckButton(hframe, "Pedestal-Subtracted ADC",1);
   pedSubADC->SetState(kButtonDown);
   hframe->AddFrame(pedSubADC, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

   // input box for event number
   inputEvent = new TGNumberEntry(hframe, 100, 5);
   inputEvent->SetLimits(TGNumberFormat::kNELLimitMinMax,0,299); // only 300 events
   hframe->AddFrame(inputEvent, new TGLayoutHints(kLHintsCenterX, 5,5,3,4));

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
   fMain->SetWindowName("Input Event Number");

   // Map all subwindows of main frame
   fMain->MapSubwindows();

   // Initialize the layout algorithm
   fMain->Resize(fMain->GetDefaultSize());

   // Map main frame
   fMain->MapWindow();
}

void MyMainFrame::DoDraw() {
   FILE *fp;                                   // pointer for reading ped-subtracted data file
   FILE *fp_ped;                               // pointer for reading pedestal data file
   Long_t event = inputEvent->GetIntNumber();  // event number (out of 300 events) 
   int adcX, adcY;                             // ped-subtracted adc value of each strip at X, Y direction
   int pedX, pedY;                             // mean pedestal value for each strip at X, Y direction
   float pedXstd, pedYstd;                     // std deviation value for pedestal for each strip

   // open file
   if ((fp = fopen("./data/apv_2021020803.txt", "rt")) == NULL || (fp_ped = fopen("./data/ped_2021020803.txt", "rt")) == NULL)
   {
     printf("Problem with opening file...\n");
     return;
   }

   // create histograms for pedestal-subtracted data first
   if (gROOT->FindObject("histoPedSubX")) delete gROOT->FindObject("histoPedSubX");
   if (gROOT->FindObject("histoPedSubY")) delete gROOT->FindObject("histoPedSubY");

   const char* histoNameX = ("Horizontal Strips (Event number = " + std::to_string(event) + ")").c_str();
   TH1F *histoPedSubX = new TH1F("histoPedSubX", histoNameX, 256, 0, 256);

   const char* histoNameY = ("Vertical Strips (Event number = " + std::to_string(event) + ")").c_str();
   TH1F *histoPedSubY = new TH1F("histoPedSubY", histoNameY, 256, 0, 256);

   histoPedSubX->SetStats(0);
   histoPedSubY->SetStats(0);
   histoPedSubX->Reset();
   histoPedSubY->Reset();

   histoPedSubX->SetXTitle("Channel ID (Strip Number)");
   histoPedSubY->SetXTitle("Channel ID (Strip Number)");
   histoPedSubX->SetYTitle("ADC");
   histoPedSubY->SetYTitle("ADC");

   // create histograms for raw adc data
   if (gROOT->FindObject("histoRawX")) delete gROOT->FindObject("histoRawX");
   if (gROOT->FindObject("histoRawY")) delete gROOT->FindObject("histoRawY");

   TH1F* histoRawX = (TH1F*)histoPedSubX->Clone("histoRawX"); 
   TH1F* histoRawY = (TH1F*)histoPedSubY->Clone("histoRawY"); 

   // skip the strip data for events that are not of our interest
   for (int i = 0; i < 512*event; i++) fscanf(fp, "%*d");

   // fill histograms for x-direction strips
   for (int ch = 0; ch < 256; ch++)
   {
     fscanf(fp, "%d", &adcX);
     histoPedSubX->Fill(ch, adcX);

     fscanf(fp_ped, "%d %f", &pedX, &pedXstd);
     histoRawX->Fill(ch, adcX+pedX);
   }

   // fill histograms for y-direction strips
   for (int ch = 0; ch < 256; ch++)
   {
     fscanf(fp, "%d", &adcY);
     histoPedSubY->Fill(ch, adcY);

     fscanf(fp_ped, "%d %f", &pedY, &pedYstd);
     histoRawY->Fill(ch, adcY+pedY);
   }

   // close the file
   fclose(fp);
   fclose(fp_ped);

   // draw on canvas
   if (gROOT->FindObject("c1")) delete gROOT->FindObject("c1");
   c1 = new TCanvas("c1", "Cosmic Ray Muon Detection with GEM", 800, 800);
   c1->Divide(1,2);

   auto legend = new TLegend(0.6,0.8,0.9,0.9);

   if (pedSubADC->IsDown() && rawADC->IsDown()) {  
      c1->cd(1);
      histoPedSubX->SetMaximum(histoRawX->GetMaximum()*1.3);
      histoPedSubX->Draw("HIST");
      histoRawX->SetLineColor(kRed);
      histoRawX->Draw("HIST SAME");
     
      legend->AddEntry(histoPedSubX,"Pedestal Subtracted ADC","l");
      legend->AddEntry(histoRawX,"Raw ADC","l");
      legend->Draw("SAME");

      c1->cd(2);
      histoPedSubY->SetMaximum(histoRawY->GetMaximum()*1.3);
      histoPedSubY->Draw("HIST");
      histoRawY->SetLineColor(kRed);
      histoRawY->Draw("HIST SAME");

      legend->Draw("SAME");
   }
   else if (!(pedSubADC->IsDown()) && rawADC->IsDown()) {
      c1->cd(1);
      histoRawX->SetLineColor(kRed);
      histoRawX->SetYTitle("Raw ADC");
      histoRawX->SetMaximum(histoRawX->GetMaximum()*1.2);
      histoRawX->Draw("HIST");
      c1->cd(2);
      histoRawY->SetLineColor(kRed);
      histoRawY->SetYTitle("Raw ADC");
      histoRawY->SetMaximum(histoRawY->GetMaximum()*1.2);
      histoRawY->Draw("HIST");

      legend->AddEntry(histoRawX,"Raw ADC","l");
      legend->AddEntry(histoRawY,"Raw ADC","l");
   }
   else if (pedSubADC->IsDown() && !(rawADC->IsDown())) {
      c1->cd(1);
      histoPedSubX->SetYTitle("Pedestal Subtracted ADC");
      histoPedSubX->SetMaximum(histoPedSubX->GetMaximum()*1.2);
      histoPedSubX->Draw("HIST");
      c1->cd(2);
      histoPedSubY->SetYTitle("Pedestal Subtracted ADC");
      histoPedSubY->SetMaximum(histoPedSubY->GetMaximum()*1.2);
      histoPedSubY->Draw("HIST");
 
      legend->AddEntry(histoPedSubX,"Pedestal-Subtracted ADC","l");
      legend->AddEntry(histoPedSubY,"Pedestal-Subtracted ADC","l");
   }

   c1->Modified();
   c1->Update();
}

void MyMainFrame::DoSave() {
   const char* savedFileName = ("event_"+std::to_string(inputEvent->GetIntNumber())+".pdf").c_str();
   
   if (pedSubADC->IsDown() && rawADC->IsDown()) savedFileName = (std::string("raw+ped-sub_")+savedFileName).c_str();
   else if (!(pedSubADC->IsDown()) && rawADC->IsDown()) savedFileName = (std::string("raw_")+savedFileName).c_str();
   else if (pedSubADC->IsDown() && !(rawADC->IsDown())) savedFileName = (std::string("ped-sub_")+savedFileName).c_str(); 

   c1->SaveAs(savedFileName);
}

MyMainFrame::~MyMainFrame() {
   // Clean up used widgets: frames, buttons, layout hints
   fMain->Cleanup();
   delete fMain;
}

void singleEvent() {
   // Popup the GUI...
   new MyMainFrame(gClient->GetRoot(),800,40);
}
