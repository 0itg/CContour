#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/dcbuffer.h"
//#include "wx/aui/auibar.h"

#include "MainWindowFrame.h"
#include "InputPanel.h"
#include "OutputPanel.h"

wxBEGIN_EVENT_TABLE(MainWindowFrame, wxFrame)
EVT_MENU(ID_Hello, MainWindowFrame::OnHello)
EVT_MENU(wxID_EXIT, MainWindowFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
EVT_TOOL_RANGE(ID_Circle, ID_Line, MainWindowFrame::OnToolbarContourSelect)
wxEND_EVENT_TABLE()

MainWindowFrame::MainWindowFrame(const wxString& title, const wxPoint& pos,
    const wxSize& size, const long style)
    : wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
    wxImage::AddHandler(new wxPNGHandler);

    this->SetMinSize(wxSize(400, 250));
    wxMenu* menuFile = new wxMenu;
   /* menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");*/
    //menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    wxToolBar* toolBar = new wxToolBar(this, ID_toolBar);
    toolBar->SetToolBitmapSize(wxSize(24, 24));
    toolBar->AddTool(ID_Circle, "Circular Contour",
        wxBitmap(wxT("icons/draw-ellipse.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_RADIO, "Draws a circular contour");
    toolBar->AddTool(ID_Rect, "Rectangular Contour",
        wxBitmap(wxT("icons/draw-rectangle.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_RADIO, "Draws a rectangular contour");
    toolBar->AddTool(ID_Polygon, "Polygonal Contour",
        wxBitmap(wxT("icons/draw-polygon.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_RADIO, "Draws a polygonal contour");
    toolBar->AddTool(ID_Line, "Polygonal Contour",
        wxBitmap(wxT("icons/draw-line.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_RADIO, "Draws a straight line");
    SetToolBar(toolBar);
    toolBar->Realize();
    CreateStatusBar();

    InputPlane = new InputPanel(this);
    OutputPanel* OutputPlane = new OutputPanel(this, InputPlane);
    OutputPlane->Refresh(); // Forces it to show mapped inputs.
    wxBoxSizer* ComplexPlanes = new wxBoxSizer(wxHORIZONTAL);
    wxSizerFlags PlaneFlags(1);
    PlaneFlags.Shaped().Border(wxALL, 10).Center();
    ComplexPlanes->Add(InputPlane, PlaneFlags);
    ComplexPlanes->Add(OutputPlane, PlaneFlags);
   SetSizer(ComplexPlanes);
}
void MainWindowFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void MainWindowFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Lorem Ipsum",
        "Complex Contour Visualizer", wxOK | wxICON_INFORMATION);
}
void MainWindowFrame::OnToolbarContourSelect(wxCommandEvent& event)
{
    InputPlane->SetContourStyle(event.GetId());
}
void MainWindowFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}