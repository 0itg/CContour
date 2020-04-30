#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/filepicker.h>

#include <fstream>

#include "ContourParametric.h"
#include "DialogCreateParametricCurve.h"
#include "DialogExportImage.h"
#include "InputPlane.h"
#include "MainWindowFrame.h"
#include "OutputPlane.h"

// Note: FreeImage causes some memory leak false positives:
// 1x 104-byte, 1x 16-byte, 30x 48-byte. #1695-1726 if that holds constant.
#include <FreeImage.h>

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindowFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainWindowFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainWindowFrame::OnAbout)
EVT_MENU(ID_NumCtrlPanel, MainWindowFrame::OnShowNumCtrlWin)
EVT_MENU(ID_VarEditPanel, MainWindowFrame::OnShowVarWin)
EVT_MENU(ID_AnimPanel, MainWindowFrame::OnShowAnimWin)
EVT_TOOL(ID_Select, MainWindowFrame::OnButtonSelectionTool)
EVT_TOOL(ID_Rotate, MainWindowFrame::OnButtonRotationTool)
EVT_TOOL(ID_Scale, MainWindowFrame::OnButtonScaleTool)
EVT_TOOL_RANGE(ID_Circle, ID_Point, MainWindowFrame::OnToolbarContourSelect)
EVT_TOOL(ID_Paintbrush, MainWindowFrame::OnButtonPaintbrush)
EVT_TOOL(ID_Parametric, MainWindowFrame::OnButtonParametricCurve)
EVT_TOOL(ID_Color_Randomizer, MainWindowFrame::OnButtonColorRandomizer)
EVT_COLOURPICKER_CHANGED(ID_Color_Picker, MainWindowFrame::OnColorPicked)
EVT_TOOL_RANGE(ID_Show_Axes,ID_Show_Zeros, MainWindowFrame::OnShowVarious)
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
EVT_MENU(ID_Export_Anim, MainWindowFrame::OnExportAnimatedGif)
EVT_MENU(ID_Export_Image, MainWindowFrame::OnExportImage)
EVT_MENU(wxID_UNDO, MainWindowFrame::OnUndo)
EVT_MENU(wxID_REDO, MainWindowFrame::OnRedo)
EVT_AUI_PANE_CLOSE(MainWindowFrame::OnAuiPaneClose)
wxEND_EVENT_TABLE();
// clang-format on

MainWindowFrame::MainWindowFrame(const wxString& title, const wxPoint& pos,
                                 const wxSize& size, const long style)
    : wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxJPEGHandler);
    aui.SetManagedWindow(this);

    SetMinSize(wxSize(400, 250));
    auto menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN);
    menuFile->Append(wxID_SAVE);
    menuFile->Append(wxID_SAVEAS);
    menuFile->Append(ID_Export_Image, "Export Image...");
    menuFile->Append(ID_Export_Anim, "Export Animation...");
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
    toolbar->AddTool(ID_Rotate, "Rotate Contour",
                     wxBitmap(wxT("icons/rotation.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Rotates selected contour");
    toolbar->AddTool(ID_Scale, "Scale Contour",
                     wxBitmap(wxT("icons/scale.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Scales selected contour");
    toolbar->AddTool(ID_Circle, "Circular Contour",
                     wxBitmap(wxT("icons/draw-ellipse.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Draws a circular contour");
    toolbar->AddTool(ID_Rect, "Rectangular Contour",
                     wxBitmap(wxT("icons/draw-rectangle.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Draws a rectangular contour");
    toolbar->AddTool(ID_Polygon, "Polygonal Contour",
                     wxBitmap(wxT("icons/draw-polygon.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Draws a polygonal contour");
    toolbar->AddTool(ID_Line, "Line Contour",
                     wxBitmap(wxT("icons/draw-line.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Draws a straight line");
    toolbar->AddTool(ID_Point, "Point",
                     wxBitmap(wxT("icons/draw-point.png"), wxBITMAP_TYPE_PNG),
                     wxNullBitmap, wxITEM_RADIO, "Draws a point");

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
    cResCtrl = new wxSpinCtrl(toolbar, ID_ContourResCtrl,
                              std::to_string(input->GetRes()),
                              wxDefaultPosition, wxDefaultSize,
                              wxTE_PROCESS_ENTER, 20, 10000, input->GetRes());
    toolbar->AddControl(cResCtrl);

    toolbar->AddSeparator();
    auto gResText = new wxStaticText(toolbar, wxID_ANY, "Grid res: ");
    toolbar->AddControl(gResText);
    gResCtrl = new wxSpinCtrl(toolbar, ID_GridResCtrl,
                              std::to_string(output->GetRes()),
                              wxDefaultPosition, wxDefaultSize,
                              wxTE_PROCESS_ENTER, 20, 10000, output->GetRes());
    toolbar->AddControl(gResCtrl);

    toolbar->AddTool(ID_Show_Zeros, "Calculate zeros/poles",
        wxBitmap(wxT("icons/zero-finder.png"), wxBITMAP_TYPE_PNG),
        wxNullBitmap, wxITEM_CHECK, "Automatically calculate zeros and poles. "
                        "Warning: Algorithm does not handle branch points, and "
                        "May produce spurious zeros/poles near them");
    toolbar->ToggleTool(ID_Show_Zeros, true);

    toolbar->AddSeparator();

    // Function entry. The user enters a function of z, which is
    // hardcoded at the independent variable for the moment.
    // The output plane is responsible for parsing the function.

    auto fnText = new wxStaticText(toolbar, wxID_ANY, "f(z) = ");
    toolbar->AddControl(fnText);
    auto funcEntry = new wxTextCtrl(
        toolbar, ID_Function_Entry, wxString("z*z"), wxDefaultPosition,
        wxSize(toolbar->GetSize().x / 5, wxDefaultSize.y), wxTE_PROCESS_ENTER);
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

MainWindowFrame::~MainWindowFrame()
{
    aui.UnInit();
    for (auto& T : threads)
        T.join();
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
    input->Bind(wxEVT_MOTION, &InputPlane::OnMouseMovingIdle, input);
}

void MainWindowFrame::OnButtonRotationTool(wxCommandEvent& event)
{
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpRotationTool, input);
    input->Bind(wxEVT_MOTION, &InputPlane::OnMouseMovingRotationTool, input);
}

void MainWindowFrame::OnButtonScaleTool(wxCommandEvent& event)
{
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpScaleTool, input);
    input->Bind(wxEVT_MOTION, &InputPlane::OnMouseMovingScaleTool, input);
}

inline void MainWindowFrame::OnToolbarContourSelect(wxCommandEvent& event)
{
    input->Bind(wxEVT_LEFT_UP, &InputPlane::OnMouseLeftUpContourTools, input);
    input->Bind(wxEVT_MOTION, &InputPlane::OnMouseMoving, input);
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
    input->Bind(wxEVT_MOTION, &InputPlane::OnMouseMoving, input);
}

void MainWindowFrame::OnButtonParametricCurve(wxCommandEvent& event)
{
    DialogCreateParametricCurve ParCreate(
        this, "Parametric Curve " + std::to_string(++input->ParametricCount));

    if (ParCreate.ShowModal() == wxID_OK)
    {
        auto C = std::make_shared<ContourParametric>(
            ParCreate.funcCtrl.GetValue(), input->GetRes(), input->color,
            ParCreate.nameCtrl.GetValue(), ParCreate.tStartCtrl.GetValue(),
            ParCreate.tEndCtrl.GetValue());
        input->AddContour(C);
        history.RecordCommand(std::make_unique<CommandAddContour>(input, C));
        if (input->randomizeColor) input->color = input->RandomColor();
        input->Update();
        input->Refresh();
        output->Update();
        output->Refresh();
        animPanel->UpdateComboBoxes();
    }
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

inline void MainWindowFrame::OnShowVarious(wxCommandEvent& event)
{
    input->OnShowVarious(event);
    output->OnShowVarious(event);
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
    input->animating     = false;
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
    DialogExportImage Export(this, saveFileName, saveFilePath, input, true);

    std::string inputFilePath;
    std::string outputFilePath;
    size_t xRes;
    size_t yRes;
    double dur;
    double frameLen;
    unsigned long frameTime;
    bool saveInputAnim;
    bool saveOutputAnim;

    if (Export.ShowModal() == wxID_OK)
    {
        xRes           = Export.xResCtrl.GetValue();
        yRes           = Export.yResCtrl.GetValue();
        dur            = Export.durCtrl.GetValue();
        frameLen       = 1 / Export.fpsCtrl.GetValue();
        inputFilePath  = Export.inputAnimFile.GetPath();
        outputFilePath = Export.outputAnimFile.GetPath();
        saveInputAnim  = Export.inCheck.IsChecked();
        saveOutputAnim = Export.outCheck.IsChecked();
    }
    else
    {
        return;
    }

    frameTime = 1000 * frameLen + 0.5;

    // Copy the program state so the animation can be created ip a
    // separate thread. Reusing serialization code for simplicity.
    // Do it once per animation so that they can be run simultaneously.
    // FreeImage is single-threaded, and GIF generation is slow,
    // so this is a net gain.

    std::unique_ptr<InputPlane> tempIn1, tempIn2;
    std::unique_ptr<OutputPlane> tempOut1, tempOut2;

    auto copyAppState = [&](std::unique_ptr<InputPlane>& ip,
                            std::unique_ptr<OutputPlane>& op) {
        ip  = std::make_unique<InputPlane>(this);
        op = std::make_unique<OutputPlane>(this, ip.get());
        ip->AddOutputPlane(op.get());

        std::stringstream ss;
        boost::archive::text_oarchive oa(ss);
        oa << *output << *input;
        boost::archive::text_iarchive ia(ss);
        ia >> *op >> *ip;
        ip->UpdateGrid();
        ip->RecalcAll();

        ip->SetClientSize(xRes, yRes);
        op->SetClientSize(xRes, yRes);
        ip->Hide();
        op->Hide();
    };

    // src is the plane from which the animation is generated.
    // Input and output planes must both be passed to this function, and
    // either order is allowed.
    auto saveAnimation = [=](std::unique_ptr<ComplexPlane> src,
                             std::unique_ptr<ComplexPlane> other,
                             std::string filePath) {
        auto frame = wxBitmap(xRes, yRes);

        FIMULTIBITMAP* frames = FreeImage_OpenMultiBitmap(
            FIF_GIF, filePath.c_str(), TRUE, FALSE, 0, BMP_SAVE_RLE);

        for (double t = 0; t < dur; t += frameLen)
        {
            src->DrawFrame(frame, t);
            auto frameIMG = frame.ConvertToImage();

            // Gives horizontal line width. Pitch may include padding, but
            // wxWidgets doesn't use any.
            int pitch = 3 * xRes;

            // Formula to calculate width with padding if necessary ip future:
            // int pitch = ((((24 * xRes) + 31) / 32) * 4);

            // FreeImage stores images ip BGRA order, and due to a bug doesn't
            // respect its own color masks, so we have to swap B and R manually.
            // this lambda is an adaptation of FreeImage's SwapRedandBlue32
            // utility function, not included ip the vcpkg build.
            auto SwapRedBlue = [=](BYTE* dib) {
                for (size_t y = 0; y < yRes; ++y, dib += pitch)
                {
                    for (BYTE* pixel = (BYTE*)dib; pixel < dib + pitch;
                         pixel += 3)
                    {
                        pixel[0] ^= pixel[2];
                        pixel[2] ^= pixel[0];
                        pixel[0] ^= pixel[2];
                    }
                }
            };

            SwapRedBlue((BYTE*)frameIMG.GetData());

            FIBITMAP* dib = FreeImage_ConvertFromRawBits(
                frameIMG.GetData(), xRes, yRes, pitch, 24, 0, 0, 0);
            FIBITMAP* frameGIF = FreeImage_ColorQuantize(dib, FIQ_WUQUANT);
            FreeImage_SetMetadata(FIMD_ANIMATION, frameGIF, NULL, NULL);

            FITAG* tag = FreeImage_CreateTag();
            if (tag)
            {
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
        threads.emplace_back(saveAnimation, std::move(tempOut1),
                             std::move(tempIn1), outputFilePath);
    }
    if (saveInputAnim)
    {
        copyAppState(tempIn2, tempOut2);
        threads.emplace_back(saveAnimation, std::move(tempIn2),
                             std::move(tempOut2), inputFilePath);
    }
}

void MainWindowFrame::OnExportImage(wxCommandEvent& event)
{
    std::string exportPath;
    DialogExportImage Export(this, saveFileName, saveFilePath, input, false);

    std::string inputFilePath;
    std::string outputFilePath;
    size_t xRes;
    size_t yRes;
    bool saveInputAnim;
    bool saveOutputAnim;
    std::string InputFileType, OutputFileType;

    // auto getExt = [](const std::string & str)
    //{
    //    size_t lastindex = str.find_last_of(".");
    //    if (lastindex != std::string::npos)
    //        return str.substr(lastindex, std::string::npos);
    //    else return std::string();
    //};

    static std::unordered_map<std::string, wxBitmapType> fileTypes = {
        {"", wxBITMAP_TYPE_PNG},
        {"png", wxBITMAP_TYPE_PNG},
        {"bmp", wxBITMAP_TYPE_BMP},
        {"jpg", wxBITMAP_TYPE_JPEG}};

    if (Export.ShowModal() == wxID_OK)
    {
        xRes           = Export.xResCtrl.GetValue();
        yRes           = Export.yResCtrl.GetValue();
        inputFilePath  = Export.inputAnimFile.GetPath();
        outputFilePath = Export.outputAnimFile.GetPath();
        saveInputAnim  = Export.inCheck.IsChecked();
        saveOutputAnim = Export.outCheck.IsChecked();
        InputFileType  = Export.inputAnimFile.GetFileName().GetExt();
        OutputFileType = Export.outputAnimFile.GetFileName().GetExt();
        if (!InputFileType.length())
        {
            inputFilePath += ".png";
            InputFileType = "png";
        }
        if (!OutputFileType.length())
        {
            outputFilePath += ".png";
            OutputFileType = "png";
        }
    }
    else
    {
        Export.SetSizer(NULL, false);
        return;
    }
    // Necessary to stop wxWidgets from deleting stack items twice.
    Export.SetSizer(NULL, false);

    wxBitmap image(xRes, yRes);
    if (saveInputAnim)
    {
        input->DrawFrame(image);
        image.SaveFile(inputFilePath, fileTypes[InputFileType]);
    }
    if (saveOutputAnim)
    {
        output->DrawFrame(image);
        image.SaveFile(outputFilePath, fileTypes[OutputFileType]);
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
    // Order is important here, since objects ip input may have pointers
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
    output->CalcZerosAndPoles();
    output->movedViewPort = true;
    for (auto TP : std::initializer_list<ToolPanel*>{numCtrlPanel, varEditPanel,
                                                     animPanel})
    {
        TP->Update();
        TP->Refresh();
        TP->Populate();
    }
}
