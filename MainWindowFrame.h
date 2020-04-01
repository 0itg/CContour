#pragma once
#include "Commands.h"
#include <wx/aui/aui.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <thread>

class InputPlane;
class OutputPlane;
class NumCtrlPanel;
class VariableEditPanel;
class AnimPanel;
class CommandHistory;

class MainWindowFrame : public wxFrame
{
  public:
    MainWindowFrame(const wxString& title, const wxPoint& pos,
                    const wxSize& size,
                    const long style = wxDEFAULT_FRAME_STYLE);
    ~MainWindowFrame() { aui.UnInit(); for (auto& T : threads) T.join(); }

    CommandHistory history;

  private:
    InputPlane* input;
    OutputPlane* output;
    NumCtrlPanel* numCtrlPanel;
    VariableEditPanel* varEditPanel;
    AnimPanel* animPanel;
    wxMenu* menuWindow;
    wxMenu* menuEdit;
    wxToolBar* toolbar;
    wxSpinCtrl* cResCtrl;
    wxSpinCtrl* gResCtrl;
    std::string saveFileName = "";
    std::string saveFilePath = "";
    std::vector<std::thread> threads;

    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnButtonSelectionTool(wxCommandEvent& event);
    void OnToolbarContourSelect(wxCommandEvent& event);
    void OnColorPicked(wxColourPickerEvent& col);
    void OnButtonColorRandomizer(wxCommandEvent& event);
    void OnButtonPaintbrush(wxCommandEvent& event);
    void OnButtonParametricCurve(wxCommandEvent& event);
    void OnFunctionEntry(wxCommandEvent& event);
    void OnGridResCtrl(wxSpinEvent& event);
    void OnGridResCtrl(wxCommandEvent& event);
    void OnContourResCtrl(wxSpinEvent& event);
    void OnContourResCtrl(wxCommandEvent& event);
    void OnShowAxes_ShowGrid(wxCommandEvent& event);
    void OnShowNumCtrlWin(wxCommandEvent& event);
    void OnShowVarWin(wxCommandEvent& event);
    void OnShowAnimWin(wxCommandEvent& event);
    void OnAuiPaneClose(wxAuiManagerEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnPlayButton(wxCommandEvent& event);
    void OnPauseButton(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnExportAnimatedGif(wxCommandEvent& event);
    void OnExportImage(wxCommandEvent& event);

    void AnimOnIdle(wxIdleEvent& idle);

    void Save(std::string& path);
    void Load(std::string& path);

    void RefreshAll();

    wxAuiManager aui;

    wxDECLARE_EVENT_TABLE();
};