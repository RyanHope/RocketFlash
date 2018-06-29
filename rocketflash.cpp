#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/filepicker.h>
#include <wx/notebook.h>

#include <ftdi.h>

#include "rocket.xpm"

class RocketFlash : public wxApp
{
public:
    virtual bool OnInit();
    int FindDevices();
    struct ftdi_context *ftdi;
    struct ftdi_device_list *devlist, *curdev;
};

class RocketFlashFrame : public wxFrame
{
public:
    RocketFlash *app;
    wxChoice *devicelist;
    RocketFlashFrame(RocketFlash& app, const wxString& title);

    void RefreshDeviceList();

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    wxDECLARE_EVENT_TABLE();
};
#include <wx/notebook.h>


enum
{
    Minimal_Quit = wxID_EXIT,
    Minimal_About = wxID_ABOUT
};

wxBEGIN_EVENT_TABLE(RocketFlashFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  RocketFlashFrame::OnQuit)
    EVT_MENU(Minimal_About, RocketFlashFrame::OnAbout)
wxEND_EVENT_TABLE()

IMPLEMENT_APP(RocketFlash)

int RocketFlash::FindDevices()
{
  int ret;
  if (this->ftdi && ((ret = ftdi_usb_find_all(this->ftdi, &this->devlist, 0, 0)) < 0))
  {
    fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(this->ftdi));
  }
  return ret;
}

bool RocketFlash::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    if ((this->ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
    }
    this->FindDevices();

    RocketFlashFrame *frame = new RocketFlashFrame(*this, "RocketFlash");
    frame->Show(true);
    return true;
}

void RocketFlashFrame::RefreshDeviceList()
{
  char manufacturer[128], description[128], serial[128];
  int i = 0;
  this->devicelist->Clear();
  for (this->app->curdev = this->app->devlist; this->app->curdev != NULL; i++)
  {
    if (ftdi_usb_get_strings(this->app->ftdi, this->app->curdev->dev, manufacturer, 128, description, 128, serial, 128) < 0)
    {
        fprintf(stderr, "ftdi_usb_get_strings failed: (%s)\n", ftdi_get_error_string(this->app->ftdi));
    }
    else
    {
      char label[512];
      sprintf(label, "%s - %s - %s", manufacturer, description, serial);
      this->devicelist->Append(label);
    }
    this->app->curdev = this->app->curdev->next;
  }
  this->devicelist->SetSelection(0);
}

RocketFlashFrame::RocketFlashFrame(RocketFlash& app, const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)
{
    this->app = &app;
    SetIcon(wxICON(rocket));
    wxMenu *fileMenu = new wxMenu;
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");
    fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit this program");
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar(2);
    SetStatusText("Welcome to RocketFlash!");

    wxPanel *panel = new wxPanel(this, -1);

    wxBoxSizer *mainbox = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer *topgrid = new  wxFlexGridSizer(2,10,10);
    topgrid->AddGrowableCol(1,1);

    wxStaticText *devicesl =  new wxStaticText(panel, wxID_ANY, wxT("Device list:"));
    topgrid->Add(devicesl, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 0);
    devicelist = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
    this->RefreshDeviceList();
    topgrid->Add(devicelist, 1, wxEXPAND | wxRIGHT, 0);
    mainbox->Add(topgrid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 10);

    wxNotebook *nb = new wxNotebook(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
    wxPanel *readpanel = new wxPanel(nb, wxID_ANY);
    wxBoxSizer *readpanelbox = new wxBoxSizer(wxVERTICAL);
    wxFilePickerCtrl *rbinpicker = new wxFilePickerCtrl(readpanel, wxID_ANY, wxEmptyString, wxT("Pick a file to save ECU bin to..."), wxT("*.bin"), wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT | wxFLP_USE_TEXTCTRL | wxFLP_SMALL);
    readpanelbox->Add(rbinpicker, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    readpanel->SetSizer(readpanelbox);
    nb->AddPage(readpanel, wxT("Read"));
    wxPanel *writepanel = new wxPanel(nb, wxID_ANY);
    wxBoxSizer *writepanelbox = new wxBoxSizer(wxVERTICAL);
    wxFilePickerCtrl *wbinpicker = new wxFilePickerCtrl(writepanel, wxID_ANY, wxEmptyString, wxT("Pick an ECU bin file to flash..."), wxT("*.bin"), wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST | wxFLP_SMALL);
    writepanelbox->Add(wbinpicker, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    writepanel->SetSizer(writepanelbox);
    nb->AddPage(writepanel, wxT("Write/Recover"));
    wxPanel *debugpanel = new wxPanel(nb, wxID_ANY);
    nb->AddPage(debugpanel, wxT("Debug"));
    mainbox->Add(nb, 1, wxEXPAND, 0);

    wxTextCtrl *log = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_DONTWRAP);
    mainbox->Add(log, 0, wxEXPAND, 0);

    panel->SetSizer(mainbox);

    Centre();
}

void RocketFlashFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void RocketFlashFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox("RocketFlash is an opensource tool\nfor flashing crotch rocket ECUs.",
                 "About RocketFlash",
                 wxOK | wxICON_INFORMATION,
                 this);
}
