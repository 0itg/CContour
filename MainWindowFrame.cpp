#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/aui/aui.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/spinctrl.h>

#include "Event_IDs.h"
#include "InputPlane.h"
#include "MainWindowFrame.h"
#include "OutputPlane.h"
#include "ToolPanel.h"

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindowFrame, wxFrame)
//EVT_MENU(ID_Hello, MainWindowFrame::OnHello)
EVT_MENU(wxID_EXIT, MainWindowFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
EVT_TOOL(ID_Select, MainWindowFrame::OnButtonSelectionTool)
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
    : wxFrame(NULL, wxID_ANY, title, pos, size, style) {
   wxImage::AddHandler(new wxPNGHandler);
   aui.SetManagedWindow(this);

   SetMinSize(wxSize(400, 250));
   auto menuFile = new wxMenu;
   /* menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");*/
   // menuFile->AppendSeparator();
   menuFile->Append(wxID_EXIT);
   auto menuHelp = new wxMenu;
   menuHelp->Append(wxID_ABOUT);
   auto menuBar = new wxMenuBar;
   menuBar->Append(menuFile, "&File");
   menuBar->Append(menuHelp, "&Help");
   SetMenuBar(menuBar);

   // Need an intermediate window to hold the sizer for the complex planes.
   auto Cplanes =
       new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
   Cplanes->SetBackgroundColour(this->GetBackgroundColour());
   input  = new InputPlane(Cplanes);
   output = new OutputPlane(Cplanes, input);

   // Drawing tools. Picking one sets the type of contour to draw.

   auto toolbar = new wxToolBar(this, ID_toolBar);
   toolbar->SetToolBitmapSize(wxSize(24, 24));
   toolbar->AddTool(ID_Select, "Select Contour",
                    wxBitmap(wxT("icons/tool-pointer.png"), wxBITMAP_TYPE_PNG),
                    wxNullBitmap, wxITEM_RADIO,
                    "Select contour for numerical editing");
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
   toolbar->ToggleTool(ID_Circle, true);
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
   auto colorCtrl =
       new wxColourPickerCtrl(toolbar, ID_Color_Picker, wxColor(0, 0, 200));
   toolbar->AddControl(colorCtrl);

   toolbar->AddStretchableSpace();
   toolbar->SetDoubleBuffered(true); // Prevents annoying flickering

   // Resolution controls. ContourResCtrl sets the number of interpolated
   // points on all transformed contours. GridResCtrl does the same for
   // transformed grid lines.

   auto cResText = new wxStaticText(toolbar, wxID_ANY, "Contour res: ");
   toolbar->AddControl(cResText);
   auto cResCtrl = new wxSpinCtrl(
       toolbar, ID_ContourResCtrl, std::to_string(input->GetRes()),
       wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, 20, 10000,
       input->GetRes());
   toolbar->AddControl(cResCtrl);

   toolbar->AddSeparator();
   auto gResText = new wxStaticText(toolbar, wxID_ANY, "Grid res: ");
   toolbar->AddControl(gResText);
   auto gResCtrl =
       new wxSpinCtrl(toolbar, ID_GridResCtrl, std::to_string(output->GetRes()),
                      wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, 20,
                      10000, output->GetRes());
   toolbar->AddControl(gResCtrl);

   toolbar->AddSeparator();

   // Function entry. The user enters a function of z, which is
   // hardcoded at the independent variable for the moment.
   // The output plane is responsible for parsing the function.

   auto fnText = new wxStaticText(toolbar, wxID_ANY, "f(z) = ");
   toolbar->AddControl(fnText);
   auto funcEntry = new wxTextCtrl(
       toolbar, ID_Function_Entry, wxString("z*z"), wxDefaultPosition,
       wxSize(toolbar->GetSize().x / 4, wxDefaultSize.y), wxTE_PROCESS_ENTER);
   toolbar->AddControl(funcEntry);
   toolbar->AddSeparator();

   SetToolBar(toolbar);
   toolbar->Realize();

   auto statBar = CreateStatusBar(2);

   input->SetColorPicker(colorCtrl);
   input->SetResCtrl(cResCtrl);
   input->SetStatusBar(statBar);

   output->SetFuncInput(funcEntry);
   output->SetResCtrl(gResCtrl);
   output->SetStatusBar(statBar);

   output->Refresh(); // Forces it to show mapped inputs.
   auto cPlaneSizer = new wxBoxSizer(wxHORIZONTAL);
   wxSizerFlags PlaneFlags(1);
   PlaneFlags.Shaped().Border(wxALL, 10).Center();

   auto axisAndCtrlPtPanel =
       new AxisAndCtrlPointPanel(this, ID_ToolPanel, wxDefaultPosition,
                                 wxSize(100, this->GetClientSize().y));
   axisAndCtrlPtPanel->SetInputPlane(input);
   axisAndCtrlPtPanel->SetOutputPlane(output);
   axisAndCtrlPtPanel->PopulateAxisTextCtrls();
   input->SetToolPanel(axisAndCtrlPtPanel);
   output->SetToolPanel(axisAndCtrlPtPanel);

   auto varEditPanel =
       new VariableEditPanel(this, ID_ToolPanel, wxDefaultPosition,
                             wxSize(100, this->GetClientSize().y));
   varEditPanel->SetOutputPlane(output);
   output->SetVarPanel(varEditPanel);
   varEditPanel->PopulateVarTextCtrls(output->f);

   cPlaneSizer->Add(input, PlaneFlags);
   cPlaneSizer->Add(output, PlaneFlags);
   Cplanes->SetSizer(cPlaneSizer);

   aui.AddPane(Cplanes,
               wxAuiPaneInfo().Center().BestSize(1200, 700).MinSize(350, 200));
   aui.AddPane(axisAndCtrlPtPanel,
               wxAuiPaneInfo().Left().BestSize(150, 700).MinSize(20, 20));
   aui.AddPane(varEditPanel,
               wxAuiPaneInfo().Right().BestSize(150, 700).MinSize(20, 20));
   aui.Update();
}

inline void MainWindowFrame::OnExit(wxCommandEvent& event) {
   Close(true);
}

inline void MainWindowFrame::OnAbout(wxCommandEvent& event) {
   wxMessageBox("Lorem Ipsum", "Complex Contour Visualizer",
                wxOK | wxICON_INFORMATION);
}

inline void MainWindowFrame::OnButtonSelectionTool(wxCommandEvent& event) {
   input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpSelectionTool, input);
}

inline void MainWindowFrame::OnToolbarContourSelect(wxCommandEvent& event) {
   input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpContourTools, input);
   input->SetContourType(event.GetId());
}

inline void MainWindowFrame::OnColorPicked(wxColourPickerEvent& col) {
   input->OnColorPicked(col);
}

inline void MainWindowFrame::OnButtonColorRandomizer(wxCommandEvent& event) {
   input->OnColorRandomizer(event);
}

inline void MainWindowFrame::OnButtonPaintbrush(wxCommandEvent& event) {
   input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpPaintbrush, input);
}

inline void MainWindowFrame::OnFunctionEntry(wxCommandEvent& event) {
   output->OnFunctionEntry(event);
}

inline void MainWindowFrame::OnGridResCtrl(wxSpinEvent& event) {
   output->OnGridResCtrl(event);
}

inline void MainWindowFrame::OnGridResCtrl(wxCommandEvent& event) {
   output->OnGridResCtrl(event);
}

inline void MainWindowFrame::OnContourResCtrl(wxSpinEvent& event) {
   input->OnContourResCtrl(event);
}

inline void MainWindowFrame::OnContourResCtrl(wxCommandEvent& event) {
   input->OnContourResCtrl(event);
}

inline void MainWindowFrame::OnShowAxes_ShowGrid(wxCommandEvent& event) {
   input->OnShowAxes_ShowGrid(event);
   output->OnShowAxes_ShowGrid(event);
}
