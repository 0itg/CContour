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
#include "InputPlane.h"
#include "OutputPlane.h"

wxBEGIN_EVENT_TABLE(MainWindowFrame, wxFrame)
//EVT_MENU(ID_Hello, MainWindowFrame::OnHello)
EVT_MENU(wxID_EXIT, MainWindowFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
EVT_TOOL_RANGE(ID_Circle, ID_Line, MainWindowFrame::OnToolbarContourSelect)
EVT_TOOL(ID_Paintbrush, MainWindowFrame::OnButtonPaintbrush)
EVT_TOOL(ID_Color_Randomizer, MainWindowFrame::OnButtonColorRandomizer)
EVT_COLOURPICKER_CHANGED(ID_Color_Picker, MainWindowFrame::OnColorPicked)
EVT_TEXT_ENTER(ID_Function_Entry, MainWindowFrame::OnFunctionEntry)
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
    toolBar->AddTool(ID_Line, "Line Contour",
        wxBitmap(wxT("icons/draw-line.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_RADIO, "Draws a straight line");
    toolBar->AddTool(ID_Paintbrush, "Paintbrush",
        wxBitmap(wxT("icons/paint-brush.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_RADIO, "Recolor a contour");
    toolBar->AddSeparator();

    toolBar->AddTool(ID_Color_Randomizer, "Color Randomizer",
        wxBitmap(wxT("icons/color-randomizer.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_CHECK,
        "Randomizes color after a contour is drawn");
    toolBar->ToggleTool(ID_Color_Randomizer, true);
    wxColourPickerCtrl* colorCtrl =
        new wxColourPickerCtrl(toolBar, ID_Color_Picker, wxColor(0, 0, 200));
    toolBar->AddControl(colorCtrl);
    wxTextCtrl* funcEntry = new wxTextCtrl(toolBar, ID_Function_Entry,
        wxString("z*z"), wxDefaultPosition,
        wxSize(toolBar->GetSize().x / 4, wxDefaultSize.y), wxTE_PROCESS_ENTER);
    toolBar->AddStretchableSpace();
    toolBar->AddControl(funcEntry);

    SetToolBar(toolBar);
    toolBar->Realize();

    wxStatusBar* statBar = CreateStatusBar(2);

    input = new InputPlane(this);
    input->SetColorPicker(colorCtrl);
    output = new OutputPlane(this, input);
    output->SetFuncInput(funcEntry);
    output->Refresh(); // Forces it to show mapped inputs.
    wxBoxSizer* ComplexPlanes = new wxBoxSizer(wxHORIZONTAL);
    wxSizerFlags PlaneFlags(1);
    PlaneFlags.Shaped().Border(wxALL, 10).Center();
    ComplexPlanes->Add(input, PlaneFlags);
    ComplexPlanes->Add(output, PlaneFlags);
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
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpContourTools, input);
    input->SetContourType(event.GetId());
}
void MainWindowFrame::OnColorPicked(wxColourPickerEvent& col)
{
    input->OnColorPicked(col);
}

void MainWindowFrame::OnButtonColorRandomizer(wxCommandEvent& event)
{
    input->OnColorRandomizer(event);
}

void MainWindowFrame::OnButtonPaintbrush(wxCommandEvent& event)
{
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpPaintbrush, input);
}

void MainWindowFrame::OnFunctionEntry(wxCommandEvent& event)
{
    output->OnFunctionEntry(event);
}
