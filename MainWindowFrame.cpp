#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/dcbuffer.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/spinctrl.h"
//#include "wx/aui/auibar.h"

#include "Event_IDs.h"
#include "InputPlane.h"
#include "MainWindowFrame.h"
#include "OutputPlane.h"

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindowFrame, wxFrame)
//EVT_MENU(ID_Hello, MainWindowFrame::OnHello)
EVT_MENU(wxID_EXIT, MainWindowFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
EVT_TOOL_RANGE(ID_Circle, ID_Line, MainWindowFrame::OnToolbarContourSelect)
EVT_TOOL(ID_Paintbrush, MainWindowFrame::OnButtonPaintbrush)
EVT_TOOL(ID_Color_Randomizer, MainWindowFrame::OnButtonColorRandomizer)
EVT_COLOURPICKER_CHANGED(ID_Color_Picker, MainWindowFrame::OnColorPicked)
EVT_TOOL_RANGE(ID_Show_Axes,ID_Show_Grid, MainWindowFrame::OnShowAxes_ShowGrid)
EVT_SPINCTRL(ID_GridResCtrl, MainWindowFrame::OnGridResCtrl)
EVT_TEXT_ENTER(ID_GridResCtrl, MainWindowFrame::OnGridResCtrl)
EVT_SPINCTRL(ID_ContourResCtrl, MainWindowFrame::OnContourResCtrl)
EVT_TEXT_ENTER(ID_ContourResCtrl, MainWindowFrame::OnContourResCtrl)
EVT_TEXT_ENTER(ID_Function_Entry, MainWindowFrame::OnFunctionEntry)
wxEND_EVENT_TABLE();
// clang-format on

MainWindowFrame::MainWindowFrame(const wxString& title, const wxPoint& pos,
                                 const wxSize& size, const long style)
    : wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
   wxImage::AddHandler(new wxPNGHandler);

   this->SetMinSize(wxSize(400, 250));
   wxMenu* menuFile = new wxMenu;
   /* menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");*/
   // menuFile->AppendSeparator();
   menuFile->Append(wxID_EXIT);
   wxMenu* menuHelp = new wxMenu;
   menuHelp->Append(wxID_ABOUT);
   wxMenuBar* menuBar = new wxMenuBar;
   menuBar->Append(menuFile, "&File");
   menuBar->Append(menuHelp, "&Help");
   SetMenuBar(menuBar);

   // Drawing tools. Picking one sets the type of contour to draw.

   wxToolBar* toolbar = new wxToolBar(this, ID_toolBar);
   toolbar->SetToolBitmapSize(wxSize(24, 24));
   toolbar->AddTool(ID_Circle, "Circular Contour",
                    wxBitmap(wxT("icons/draw-ellipse.png"), wxBITMAP_TYPE_PNG),
                    wxNullBitmap, wxITEM_RADIO, "Draws a circular contour");
   toolbar->AddTool(
       ID_Rect, "Rectangular Contour",
       wxBitmap(wxT("icons/draw-rectangle.png"), wxBITMAP_TYPE_PNG),
       wxNullBitmap, wxITEM_RADIO, "Draws a rectangular contour");
   toolbar->AddTool(ID_Polygon, "Polygonal Contour",
                    wxBitmap(wxT("icons/draw-polygon.png"), wxBITMAP_TYPE_PNG),
                    wxNullBitmap, wxITEM_RADIO, "Draws a polygonal contour");
   toolbar->AddTool(ID_Line, "Line Contour",
                    wxBitmap(wxT("icons/draw-line.png"), wxBITMAP_TYPE_PNG),
                    wxNullBitmap, wxITEM_RADIO, "Draws a straight line");

   // Paintbrush recolors the highlighted contour with the current color.

   toolbar->AddTool(ID_Paintbrush, "Paintbrush",
                    wxBitmap(wxT("icons/paint-brush.png"), wxBITMAP_TYPE_PNG),
                    wxNullBitmap, wxITEM_RADIO, "Recolor a contour");
   toolbar->AddSeparator();

   // Toggle axes and grid lines.

   toolbar->AddTool(ID_Show_Axes, "Show/Hide Axes",
                    wxBitmap(wxT("icons/axis.png"), wxBITMAP_TYPE_PNG),
                    wxNullBitmap, wxITEM_CHECK, "Show or Hide Axes");
   toolbar->ToggleTool(ID_Show_Axes, true);
   toolbar->AddTool(ID_Show_Grid, "Show/Hide Grid",
                    wxBitmap(wxT("icons/grid.png"), wxBITMAP_TYPE_PNG),
                    wxNullBitmap, wxITEM_CHECK, "Show or Hide Grid");
   toolbar->ToggleTool(ID_Show_Grid, true);
   toolbar->AddSeparator();

   // Color tools. Randomizer, if toggled, picks a new color
   // after any contour is drawn. Color Picker lets the user
   // choose the current color.

   toolbar->AddTool(
       ID_Color_Randomizer, "Color Randomizer",
       wxBitmap(wxT("icons/color-randomizer.png"), wxBITMAP_TYPE_PNG),
       wxNullBitmap, wxITEM_CHECK, "Randomizes color after a contour is drawn");
   toolbar->ToggleTool(ID_Color_Randomizer, true);
   wxColourPickerCtrl* colorCtrl =
       new wxColourPickerCtrl(toolbar, ID_Color_Picker, wxColor(0, 0, 200));
   toolbar->AddControl(colorCtrl);

   toolbar->AddStretchableSpace();

   // Resolution controls. ContourResCtrl sets the number of interpolated
   // points on all transformed contours. GridResCtrl does the same for
   // transformed grid lines.

   wxStaticText* cResText =
       new wxStaticText(toolbar, wxID_ANY, "Contour res: ");
   toolbar->AddControl(cResText);
   wxSpinCtrl* cResCtrl =
       new wxSpinCtrl(toolbar, ID_ContourResCtrl, wxT("100"), wxDefaultPosition,
                      wxDefaultSize, wxTE_PROCESS_ENTER, 20, 10000, 100);
   toolbar->AddControl(cResCtrl);

   toolbar->AddSeparator();
   wxStaticText* gResText = new wxStaticText(toolbar, wxID_ANY, "Grid res: ");
   toolbar->AddControl(gResText);
   wxSpinCtrl* gResCtrl =
       new wxSpinCtrl(toolbar, ID_GridResCtrl, wxT("100"), wxDefaultPosition,
                      wxDefaultSize, wxTE_PROCESS_ENTER, 20, 10000, 100);
   toolbar->AddControl(gResCtrl);

   toolbar->AddSeparator();

   // Function entry. The user enters a function of z, which is
   // hardcoded at the independent variable for the moment.
   // The output plane is responsible for parsing the function.

   wxStaticText* fnText = new wxStaticText(toolbar, wxID_ANY, "f(z) = ");
   toolbar->AddControl(fnText);
   wxTextCtrl* funcEntry = new wxTextCtrl(
       toolbar, ID_Function_Entry, wxString("z*z"), wxDefaultPosition,
       wxSize(toolbar->GetSize().x / 4, wxDefaultSize.y), wxTE_PROCESS_ENTER);
   toolbar->AddControl(funcEntry);
   toolbar->AddSeparator();

   SetToolBar(toolbar);
   toolbar->Realize();

   wxStatusBar* statBar = CreateStatusBar(2);

   // Create input and output planes, and connect various controls to them.

   input = new InputPlane(this);
   input->SetColorPicker(colorCtrl);
   input->SetResCtrl(cResCtrl);

   output = new OutputPlane(this, input);
   output->SetFuncInput(funcEntry);
   output->SetResCtrl(gResCtrl);

   output->Refresh(); // Forces it to show mapped inputs.
   wxBoxSizer* ComplexPlanes = new wxBoxSizer(wxHORIZONTAL);
   wxSizerFlags PlaneFlags(1);
   PlaneFlags.Shaped().Border(wxALL, 10).Center();
   ComplexPlanes->Add(input, PlaneFlags);
   ComplexPlanes->Add(output, PlaneFlags);
   SetSizer(ComplexPlanes);
}

void MainWindowFrame::OnExit(wxCommandEvent& event) { Close(true); }
void MainWindowFrame::OnAbout(wxCommandEvent& event)
{
   wxMessageBox("Lorem Ipsum", "Complex Contour Visualizer",
                wxOK | wxICON_INFORMATION);
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

void MainWindowFrame::OnGridResCtrl(wxSpinEvent& event)
{
   output->OnGridResCtrl(event);
}

void MainWindowFrame::OnGridResCtrl(wxCommandEvent& event)
{
   output->OnGridResCtrl(event);
}

void MainWindowFrame::OnContourResCtrl(wxSpinEvent& event)
{
   input->OnContourResCtrl(event);
}

void MainWindowFrame::OnContourResCtrl(wxCommandEvent& event)
{
   input->OnContourResCtrl(event);
}

void MainWindowFrame::OnShowAxes_ShowGrid(wxCommandEvent& event)
{
   input->OnShowAxes_ShowGrid(event);
   output->OnShowAxes_ShowGrid(event);
}
