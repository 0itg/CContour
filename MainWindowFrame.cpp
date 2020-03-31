#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/filepicker.h>

#include <fstream>
#include <FreeImage.h>

#include "ContourParametric.h"
#include "InputPlane.h"
#include "MainWindowFrame.h"
#include "OutputPlane.h"

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindowFrame, wxFrame)
//EVT_MENU(ID_Hello, MainWindowFrame::OnHello)
EVT_MENU(wxID_EXIT, MainWindowFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
EVT_MENU(ID_NumCtrlPanel, MainWindowFrame::OnShowNumCtrlWin)
EVT_MENU(ID_VarEditPanel, MainWindowFrame::OnShowVarWin)
EVT_MENU(ID_AnimPanel, MainWindowFrame::OnShowAnimWin)
EVT_TOOL(ID_Select, MainWindowFrame::OnButtonSelectionTool)
EVT_TOOL_RANGE(ID_Circle, ID_Line, MainWindowFrame::OnToolbarContourSelect)
EVT_TOOL(ID_Paintbrush, MainWindowFrame::OnButtonPaintbrush)
EVT_TOOL(ID_Parametric, MainWindowFrame::OnButtonParametricCurve)
EVT_TOOL(ID_Color_Randomizer, MainWindowFrame::OnButtonColorRandomizer)
EVT_COLOURPICKER_CHANGED(ID_Color_Picker, MainWindowFrame::OnColorPicked)
EVT_TOOL_RANGE(ID_Show_Axes,ID_Show_Grid, MainWindowFrame::OnShowAxes_ShowGrid)
EVT_SPINCTRL(ID_GridResCtrl, MainWindowFrame::OnGridResCtrl)
EVT_TEXT_ENTER(ID_GridResCtrl, MainWindowFrame::OnGridResCtrl)
EVT_SPINCTRL(ID_ContourResCtrl, MainWindowFrame::OnContourResCtrl)
EVT_TEXT_ENTER(ID_ContourResCtrl, MainWindowFrame::OnContourResCtrl)
EVT_TEXT_ENTER(ID_Function_Entry, MainWindowFrame::OnFunctionEntry)
EVT_TOOL(ID_Play, MainWindowFrame::OnPlayButton)
EVT_TOOL(ID_Pause, MainWindowFrame::OnPauseButton)
EVT_MENU(wxID_OPEN, MainWindowFrame::OnOpen)
EVT_MENU(wxID_SAVE, MainWindowFrame::OnSave)
EVT_MENU(wxID_SAVEAS, MainWindowFrame::OnSaveAs)
EVT_MENU(ID_Export_Image, MainWindowFrame::OnExportAnimatedGif)
EVT_MENU(wxID_UNDO, MainWindowFrame::OnUndo)
EVT_MENU(wxID_REDO, MainWindowFrame::OnRedo)
EVT_AUI_PANE_CLOSE(MainWindowFrame::OnAuiPaneClose)
wxEND_EVENT_TABLE();
// clang-format on

MainWindowFrame::MainWindowFrame(const wxString& title, const wxPoint& pos,
                                 const wxSize& size, const long style)
    : wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
    //Magick::InitializeMagick(NULL);
    wxImage::AddHandler(new wxPNGHandler);
    aui.SetManagedWindow(this);

    SetMinSize(wxSize(400, 250));
    auto menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN);
    menuFile->Append(wxID_SAVE);
    menuFile->Append(wxID_SAVEAS);
    menuFile->Append(ID_Export_Image, "Export Animated GIF...");
    menuFile->Append(wxID_EXIT);

    menuEdit = new wxMenu;

    menuEdit->Append(wxID_UNDO);
    menuEdit->Append(wxID_REDO);
    menuEdit->FindItem(wxID_UNDO)->Enable(false);
    menuEdit->FindItem(wxID_REDO)->Enable(false);

    history.SetMenu(menuEdit);

    menuWindow = new wxMenu;
    menuWindow->AppendCheckItem(ID_NumCtrlPanel, "&Numerical Controls");
    menuWindow->AppendCheckItem(ID_VarEditPanel, "&Variables");
    menuWindow->AppendCheckItem(ID_AnimPanel, "&Animation Controls");
    menuWindow->Check(ID_NumCtrlPanel, true);
    menuWindow->Check(ID_VarEditPanel, true);
    menuWindow->Check(ID_AnimPanel, true);

    auto menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    auto menuBar = new wxMenuBar;

    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuWindow, "&Window");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    // Need an intermediate window to hold the sizer for the complex planes.
    auto Cplanes =
        new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    Cplanes->SetBackgroundColour(this->GetBackgroundColour());
    input  = new InputPlane(Cplanes);
    output = new OutputPlane(Cplanes, input);

    // Drawing tools. Picking one sets the type of contour to draw.

    toolbar = new wxToolBar(this, ID_Toolbar);
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

    // Creates a contour from a parametric curve.

    toolbar->AddTool(
        ID_Parametric, "Parametric Curve",
        wxBitmap(wxT("icons/parametric-curve.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_NORMAL, "Opens a Parametric Curve dialog");
    toolbar->AddSeparator();
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
        wxNullBitmap, wxITEM_CHECK,
        "Randomizes color after a contour is drawn");
    toolbar->ToggleTool(ID_Color_Randomizer, true);
    auto colorCtrl =
        new wxColourPickerCtrl(toolbar, ID_Color_Picker, wxColor(0, 0, 200));
    toolbar->AddControl(colorCtrl);

    toolbar->AddSeparator();

    // Pause and play buttons for animations

    toolbar->AddTool(ID_Play, "Play Animations",
                     wxBitmap(wxT("icons/play.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Play Animations");
    toolbar->AddTool(ID_Pause, "Pause Animations",
                     wxBitmap(wxT("icons/pause.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Pause Animations");

    toolbar->ToggleTool(ID_Pause, true);

    toolbar->AddStretchableSpace();
    toolbar->SetDoubleBuffered(true); // Prevents annoying flickering

    // Resolution controls. ContourResCtrl sets the number of interpolated
    // points on all transformed contours. GridResCtrl does the same for
    // transformed grid lines.

    auto cResText = new wxStaticText(toolbar, wxID_ANY, "Contour res: ");
    toolbar->AddControl(cResText);
    cResCtrl = new wxSpinCtrl(
        toolbar, ID_ContourResCtrl, std::to_string(input->GetRes()),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, 20, 10000,
        input->GetRes());
    toolbar->AddControl(cResCtrl);

    toolbar->AddSeparator();
    auto gResText = new wxStaticText(toolbar, wxID_ANY, "Grid res: ");
    toolbar->AddControl(gResText);
    gResCtrl = new wxSpinCtrl(
        toolbar, ID_GridResCtrl, std::to_string(output->GetRes()),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, 20, 10000,
        output->GetRes());
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

    input->SetToolbar(toolbar);
    input->SetColorPicker(colorCtrl);
    input->SetResCtrl(cResCtrl);
    input->SetStatusBar(statBar);
    input->SetCommandHistory(&history);
    input->SetFocus();

    output->SetToolbar(toolbar);
    output->SetFuncInput(funcEntry);
    output->SetResCtrl(gResCtrl);
    output->SetCommandHistory(&history);
    output->SetStatusBar(statBar);

    output->Refresh(); // Forces it to show mapped inputs.
    auto cPlaneSizer = new wxBoxSizer(wxHORIZONTAL);
    wxSizerFlags PlaneFlags(1);
    PlaneFlags.Shaped().Border(wxALL, 10).Center();

    numCtrlPanel =
        new NumCtrlPanel(this, ID_NumCtrlPanel, wxDefaultPosition,
                         wxSize(100, this->GetClientSize().y), input, output);
    numCtrlPanel->PopulateAxisTextCtrls();
    numCtrlPanel->SetCommandHistory(&history);
    input->SetToolPanel(numCtrlPanel);
    output->SetToolPanel(numCtrlPanel);

    varEditPanel =
        new VariableEditPanel(this, ID_VarEditPanel, wxDefaultPosition,
                              wxSize(100, this->GetClientSize().y), output);
    output->SetVarPanel(varEditPanel);
    varEditPanel->Populate(output->f);
    varEditPanel->SetCommandHistory(&history);

    animPanel = new AnimPanel(this, ID_AnimPanel, wxDefaultPosition,
                              wxSize(this->GetClientSize().x, 100), input);
    animPanel->SetCommandHistory(&history);
    input->SetAnimPanel(animPanel);

    cPlaneSizer->Add(input, PlaneFlags);
    cPlaneSizer->Add(output, PlaneFlags);
    Cplanes->SetSizer(cPlaneSizer);

    aui.AddPane(Cplanes, wxAuiPaneInfo()
                             .PaneBorder(false)
                             .Center()
                             .BestSize(1200, 700)
                             .MinSize(350, 200)
                             .CloseButton(false));
    aui.AddPane(numCtrlPanel, wxAuiPaneInfo()
                                  .Caption("Numerical Controls")
                                  .Left()
                                  .BestSize(150, 700)
                                  .MinSize(50, 50)
                                  .TopDockable(false)
                                  .BottomDockable(false)
                                  .FloatingSize(180, 500));
    aui.AddPane(varEditPanel, wxAuiPaneInfo()
                                  .Caption("Function variables")
                                  .Right()
                                  .BestSize(150, 700)
                                  .MinSize(50, 50)
                                  .TopDockable(false)
                                  .BottomDockable(false)
                                  .FloatingSize(180, 500));
    aui.AddPane(animPanel, wxAuiPaneInfo()
                               .Caption("Animations")
                               .Bottom()
                               .BestSize(700, 100)
                               .MinSize(50, 50)
                               .RightDockable(false)
                               .LeftDockable(false)
                               .FloatingSize(1200, 200));
    aui.Update();
}

inline void MainWindowFrame::OnExit(wxCommandEvent& event) { Close(true); }

inline void MainWindowFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Lorem Ipsum", "Complex Contour Visualizer",
                 wxOK | wxICON_INFORMATION);
}

inline void MainWindowFrame::OnButtonSelectionTool(wxCommandEvent& event)
{
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpSelectionTool, input);
}

inline void MainWindowFrame::OnToolbarContourSelect(wxCommandEvent& event)
{
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpContourTools, input);
    input->SetContourType(event.GetId());
}

inline void MainWindowFrame::OnColorPicked(wxColourPickerEvent& col)
{
    input->OnColorPicked(col);
}

inline void MainWindowFrame::OnButtonColorRandomizer(wxCommandEvent& event)
{
    input->OnColorRandomizer(event);
}

inline void MainWindowFrame::OnButtonPaintbrush(wxCommandEvent& event)
{
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpPaintbrush, input);
}

void MainWindowFrame::OnButtonParametricCurve(wxCommandEvent& event)
{
    wxDialog ParametricCreate(this, wxID_ANY, "Create a Parametric Curve");
    wxBoxSizer sizer(wxVERTICAL);
    wxSizerFlags flags(1);
    flags.Expand().Border(wxALL, 3);

    wxStaticText enterName(&ParametricCreate, wxID_ANY, "Curve name");
    sizer.Add(&enterName, flags);
    wxTextCtrl nameCtrl(&ParametricCreate, wxID_ANY, "Parametric Curve");
    sizer.Add(&nameCtrl, flags);

    wxStaticText enterFunc(&ParametricCreate, wxID_ANY, "Function f(t)");
    sizer.Add(&enterFunc, flags);
    wxTextCtrl funcCtrl(&ParametricCreate, wxID_ANY, "4*t*exp(4pi*i*t)");
    sizer.Add(&funcCtrl, flags);

    wxStaticText enterTStart(&ParametricCreate, wxID_ANY, "t Start");
    sizer.Add(&enterTStart, flags);
    wxSpinCtrlDouble tStartCtrl(&ParametricCreate, wxID_ANY, "0",
                                wxDefaultPosition, wxDefaultSize,
                                wxSP_ARROW_KEYS, -1e10, 1e10, 0, 0.1);
    sizer.Add(&tStartCtrl, flags);

    wxStaticText enterTEnd(&ParametricCreate, wxID_ANY, "t End");
    sizer.Add(&enterTEnd, flags);
    wxSpinCtrlDouble tEndCtrl(&ParametricCreate, wxID_ANY, "1.0",
                              wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,
                              -1e10, 1e10, 1, 0.1);
    sizer.Add(&tEndCtrl, flags);
    sizer.Add(ParametricCreate.CreateButtonSizer(wxOK | wxCANCEL),
              wxSizerFlags(1).Border(wxALL, 3).CenterHorizontal());
    ParametricCreate.SetSizerAndFit(&sizer);

    if (ParametricCreate.ShowModal() == wxID_OK)
    {
        auto C = std::make_shared<ContourParametric>(
            funcCtrl.GetValue(), input->GetRes(), input->color,
            nameCtrl.GetValue(), tStartCtrl.GetValue(), tEndCtrl.GetValue());
        input->AddContour(C);
        history.RecordCommand(std::make_unique<CommandAddContour>(input, C));
        if (input->randomizeColor) input->color = input->RandomColor();
        input->Update();
        input->Refresh();
        output->Update();
        output->Refresh();
        animPanel->UpdateComboBoxes();
    }
    // Necessary to stop wxWidgets from deleting stack items twice.
    ParametricCreate.SetSizer(NULL, false);
}

inline void MainWindowFrame::OnFunctionEntry(wxCommandEvent& event)
{
    output->OnFunctionEntry(event);
}

inline void MainWindowFrame::OnGridResCtrl(wxSpinEvent& event)
{
    output->OnGridResCtrl(event);
}

inline void MainWindowFrame::OnGridResCtrl(wxCommandEvent& event)
{
    output->OnGridResCtrl(event);
}

inline void MainWindowFrame::OnContourResCtrl(wxSpinEvent& event)
{
    input->OnContourResCtrl(event);
}

inline void MainWindowFrame::OnContourResCtrl(wxCommandEvent& event)
{
    input->OnContourResCtrl(event);
}

inline void MainWindowFrame::OnShowAxes_ShowGrid(wxCommandEvent& event)
{
    input->OnShowAxes_ShowGrid(event);
    output->OnShowAxes_ShowGrid(event);
}

inline void MainWindowFrame::OnShowNumCtrlWin(wxCommandEvent& event)
{
    aui.GetPane(numCtrlPanel).Show(event.IsChecked());
    aui.Update();
}

inline void MainWindowFrame::OnShowVarWin(wxCommandEvent& event)
{
    aui.GetPane(varEditPanel).Show(event.IsChecked());
    aui.Update();
}

void MainWindowFrame::OnShowAnimWin(wxCommandEvent& event)
{
    aui.GetPane(animPanel).Show(event.IsChecked());
    aui.Update();
}

inline void MainWindowFrame::OnAuiPaneClose(wxAuiManagerEvent& event)
{
    menuWindow->Check(event.pane->window->GetId(), false);
    menuWindow->UpdateUI();
}

void MainWindowFrame::OnOpen(wxCommandEvent& event)
{
    wxFileDialog open(this, "Open...", "", "", "TXT files (*.txt)|*.txt",
                      wxFD_OPEN);
    if (open.ShowModal() == wxID_OK)
    {
        saveFileName = open.GetFilename();
        saveFilePath = open.GetPath();
        Load(saveFilePath);
    }
}

void MainWindowFrame::OnSave(wxCommandEvent& event)
{
    if (saveFilePath != "")
        Save(saveFilePath);
    else
    {
        wxCommandEvent dummy;
        OnSaveAs(dummy);
    }
}

void MainWindowFrame::OnSaveAs(wxCommandEvent& event)
{
    wxFileDialog save(this, "Save as...", "", "contour_graph.txt",
                      "TXT files (*.txt)|*.txt",
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (save.ShowModal() == wxID_OK)
    {
        saveFileName = save.GetFilename();
        saveFilePath = save.GetPath();
        Save(saveFilePath);
    }
}

void MainWindowFrame::OnPlayButton(wxCommandEvent& event)
{
    input->animating = true;
    input->animTimer.Start(1000);
    Bind(wxEVT_IDLE, &MainWindowFrame::AnimOnIdle, this);
}

void MainWindowFrame::OnPauseButton(wxCommandEvent& event)
{
    input->animating = false;
    input->movedViewPort = true;
    input->animTimer.Pause();
    varEditPanel->Refresh();
    numCtrlPanel->Refresh();
}

void MainWindowFrame::OnUndo(wxCommandEvent& event)
{
    history.undo();
    RefreshAll();
}

void MainWindowFrame::OnRedo(wxCommandEvent& event)
{
    history.redo();
    RefreshAll();
}

void MainWindowFrame::OnExportAnimatedGif(wxCommandEvent& event)
{
    wxDialog Export(this, wxID_ANY, "Export Image", wxDefaultPosition,
        wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    wxFlexGridSizer sizer(7, 3, 0, 0);
    wxSizerFlags flagsLeft(1);
    flagsLeft.Border(wxALL, 3).Proportion(0);
    wxSizerFlags flagsRight(1);
    flagsRight.Border(wxALL, 3).Proportion(1).Expand();

    auto clientSize = input->GetClientSize();
    double defaultDur = input->GetLongestAnimDur();
    std::string inputFileName;
    std::string outputFileName;
    std::string inputFilePath;
    std::string outputFilePath;
    if (saveFileName.length())
    {
        auto removeExt = [](std::string str)
        {
            size_t lastindex = str.find_last_of(".");
            if (lastindex != std::string::npos)
                return str.substr(0, lastindex);
            else return str;
        };
        inputFileName  = removeExt(saveFilePath) + "_input.gif";
        outputFileName = removeExt(saveFilePath) + "_output.gif";
    }
    size_t xRes;
    size_t yRes;
    double dur;
    double frameLen;
    bool saveInputAnim;
    bool saveOutputAnim;
    wxBitmap image;

    sizer.AddGrowableCol(2);

    wxCheckBox inCheck(&Export, wxID_ANY, "");
    sizer.Add(&inCheck, flagsLeft);
    wxStaticText inputFileLabel(&Export, wxID_ANY, "Export Input Animation: ");
    sizer.Add(&inputFileLabel, flagsLeft);
    wxFilePickerCtrl inputAnimFile(&Export, wxID_ANY, inputFileName,
        "Export Animation", "Animated GIF (*.gif)|*.gif",
        wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT
        | wxFLP_USE_TEXTCTRL);
    sizer.Add(&inputAnimFile, flagsRight);
    wxCheckBox outCheck(&Export, wxID_ANY, "");
    sizer.Add(&outCheck, flagsLeft);
    wxStaticText outputFileLabel(&Export, wxID_ANY, "Export Output Animation: ");
    sizer.Add(&outputFileLabel, flagsLeft);
    wxFilePickerCtrl outputAnimFile(&Export, wxID_ANY, outputFileName,
        "Export Animation", "Animated GIF (*.gif)|*.gif",
        wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT
        | wxFLP_USE_TEXTCTRL);
    sizer.Add(&outputAnimFile, flagsRight);
    sizer.AddSpacer(0);
    wxStaticText xResLabel(&Export, wxID_ANY, "x Resolution: ");
    sizer.Add(&xResLabel, flagsLeft);
    wxSpinCtrl xResCtrl(&Export, wxID_ANY, std::to_string(clientSize.x),
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, clientSize.x);
    sizer.Add(&xResCtrl, flagsRight);
    sizer.AddSpacer(0);
    wxStaticText yResLabel(&Export, wxID_ANY, "y Resolution: ");
    sizer.Add(&yResLabel, flagsLeft);
    wxSpinCtrl yResCtrl(&Export, wxID_ANY, std::to_string(clientSize.y),
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, clientSize.y);
    sizer.Add(&yResCtrl, flagsRight);
    sizer.AddSpacer(0);
    wxStaticText durLabel(&Export, wxID_ANY, "Duration: ");
    sizer.Add(&durLabel, flagsLeft);
    wxSpinCtrlDouble durCtrl(&Export, wxID_ANY, std::to_string(defaultDur),
        wxDefaultPosition, xResCtrl.GetSize(), wxSP_ARROW_KEYS, 0, 10000, defaultDur);
    sizer.Add(&durCtrl, flagsRight);
    sizer.AddSpacer(0);
    wxStaticText fpsLabel(&Export, wxID_ANY, "Frame Rate: ");
    sizer.Add(&fpsLabel, flagsLeft);
    wxSpinCtrlDouble fpsCtrl(&Export, wxID_ANY, "30",
        wxDefaultPosition, xResCtrl.GetSize(), wxSP_ARROW_KEYS, 1, 1000, 30);
    sizer.Add(&fpsCtrl, flagsRight);
    sizer.AddSpacer(0);
    sizer.Add(Export.CreateButtonSizer(wxOK | wxCANCEL),
        wxSizerFlags(1).Border(wxALL, 3).Align(wxALIGN_RIGHT));

    Export.SetSizerAndFit(&sizer);
    if (Export.ShowModal() == wxID_OK)
    {
        xRes = xResCtrl.GetValue();
        yRes = yResCtrl.GetValue();
        dur = durCtrl.GetValue();
        frameLen = 1 / fpsCtrl.GetValue();
        image.Create(xRes, yRes);
        inputFilePath  = inputAnimFile.GetPath();
        outputFilePath = outputAnimFile.GetPath();
        saveInputAnim  = inCheck.IsChecked();
        saveOutputAnim = outCheck.IsChecked();
    }
    else
    {
        Export.SetSizer(NULL, false);
        return;
    }

    DWORD frameTime = 1000 * frameLen + 0.5;

    // Necessary to stop wxWidgets from deleting stack items twice.
    Export.SetSizer(NULL, false);

    // Copy the program state so the animation can be created in a
    // separate thread. Reusing serialization code for simplicity.
    // Do it once per animation so that they can be run simultaneously.
    // FreeImage is single-threaded, and GIF generation is slow,
    // so this is a net gain.

    std::unique_ptr<InputPlane> tempIn1, tempIn2;
    std::unique_ptr<OutputPlane> tempOut1, tempOut2;

    auto copyAppState = [&](std::unique_ptr<InputPlane>& tempInput,
        std::unique_ptr<OutputPlane>& tempOutput) {
            tempInput = std::make_unique<InputPlane>(this);
            tempOutput = std::make_unique<OutputPlane>(this, tempInput.get());
            tempInput->AddOutputPlane(tempOutput.get());

            std::stringstream ss;
            boost::archive::text_oarchive oa(ss);
            oa << *output << *input;
            boost::archive::text_iarchive ia(ss);
            ia >> *tempOutput >> *tempInput;
            tempInput->UpdateGrid();
            tempInput->RecalcAll();

            tempInput->SetClientSize(xRes, yRes);
            tempOutput->SetClientSize(xRes, yRes);
            tempInput->Hide();
            tempOutput->Hide();
    };

    //tempInput->DrawFrame(image);
    //image.SaveFile(name, wxBITMAP_TYPE_PNG);
    //src->DrawFrame(image);
    //image.SaveFile(name2, wxBITMAP_TYPE_PNG);


    // src is the plane from which the animation is generated.
    // Input and output planes must both be passed to this function, and
    // either order is allowed.
    auto saveAnimation = [=](std::unique_ptr<ComplexPlane> src,
        std::unique_ptr<ComplexPlane> other, std::string filePath)
    {
        auto frame = wxBitmap(xRes, yRes);

        FIMULTIBITMAP* frames = FreeImage_OpenMultiBitmap(FIF_GIF,
            filePath.c_str(), TRUE, FALSE, 0, BMP_SAVE_RLE);

        for (double t = 0; t < dur; t += frameLen)
        {
            src->DrawFrame(frame, t);
            auto frameIMG = frame.ConvertToImage();

            // Gives horizontal line width. Pitch may include padding, but
            // wxWidgets doesn't use any.
            int pitch = 3 * xRes;

            // Formula to calculate width with padding if necessary in future:
            //int pitch = ((((24 * xRes) + 31) / 32) * 4);

            // FreeImage stores images in BGRA order, and due to a bug doesn't 
            // respect its own color masks, so we have to swap B and R manually.
            // this lambda is an adaptation of FreeImage's SwapRedandBlue32
            // utility function, not included in the vcpkg build.
            auto SwapRedBlue = [=](BYTE* dib) {
                for (size_t y = 0; y < yRes; ++y, dib += pitch)
                {
                    for (BYTE* pixel = (BYTE*)dib; pixel < dib + pitch; pixel += 3)
                        {
                            pixel[0] ^= pixel[2];
                            pixel[2] ^= pixel[0];
                            pixel[0] ^= pixel[2];
                        }
                    }
            };

            SwapRedBlue((BYTE*)frameIMG.GetData());

            FIBITMAP* dib = FreeImage_ConvertFromRawBits(frameIMG.GetData(),
                xRes, yRes, pitch, 24, 0, 0, 0);
            FIBITMAP* frameGIF = FreeImage_ColorQuantize(dib, FIQ_WUQUANT);
            FreeImage_SetMetadata(FIMD_ANIMATION, frameGIF, NULL, NULL);

            FITAG* tag = FreeImage_CreateTag();
            if (tag) {
                FreeImage_SetTagKey(tag, "FrameTime");
                FreeImage_SetTagType(tag, FIDT_LONG);
                FreeImage_SetTagCount(tag, 1);
                FreeImage_SetTagLength(tag, 4);
                FreeImage_SetTagValue(tag, &frameTime);
                FreeImage_SetMetadata(FIMD_ANIMATION, frameGIF,
                    FreeImage_GetTagKey(tag), tag);
                FreeImage_DeleteTag(tag);
            }
            FreeImage_AppendPage(frames, frameGIF);
            FreeImage_Unload(dib);
            FreeImage_Unload(frameGIF);
        }
        FreeImage_CloseMultiBitmap(frames);
    };
    if (saveOutputAnim)
    {
        copyAppState(tempIn1, tempOut1);
        threads.emplace_back(saveAnimation,
            std::move(tempOut1), std::move(tempIn1), outputFilePath);
    }
    if (saveInputAnim)
    {
        copyAppState(tempIn2, tempOut2);
        threads.emplace_back(saveAnimation,
            std::move(tempIn2), std::move(tempOut2), inputFilePath);
    }
}

void MainWindowFrame::AnimOnIdle(wxIdleEvent& idle)
{
    if (input->animating) { input->Redraw(); }
}

void MainWindowFrame::Save(std::string& path)
{
    std::ofstream ofs(path);
    boost::archive::text_oarchive oa(ofs);
    // Order is important here, since objects in input may have pointers
    // to output.f which need to be initialized after f. 
    oa << *output << *input;
}

void MainWindowFrame::Load(std::string& path)
{
    input->PrepareForLoadFromFile();
    output->PrepareForLoadFromFile();
    try
    {
        std::ifstream ifs(path);
        boost::archive::text_iarchive ia(ifs);
        ia >> *output >> *input;
    }
    catch (std::exception error)
    {
        wxMessageBox(error.what(), "Error Loading File");
    }
    input->RefreshShowAxes_ShowGrid();
    cResCtrl->SetValue(input->GetRes());
    varEditPanel->Populate(output->f);
    RefreshAll();
    numCtrlPanel->PopulateAxisTextCtrls();
    history.Clear();
}

void MainWindowFrame::RefreshAll()
{
    input->Update();
    input->Refresh();
    input->RecalcAll();
    input->UpdateGrid();
    output->Update();
    output->Refresh();
    output->RefreshFuncText();
    output->movedViewPort = true;
    for (auto TP : std::initializer_list<ToolPanel*>
        { numCtrlPanel, varEditPanel, animPanel })
    {
        TP->Update();
        TP->Refresh();
        TP->Populate();
    }
}