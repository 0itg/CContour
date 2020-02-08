#pragma once

class InputPlane;

class MainWindowFrame : public wxFrame
{
public:
    MainWindowFrame(const wxString& title, const wxPoint& pos,
        const wxSize& size, const long style = wxDEFAULT_FRAME_STYLE);
private:
    InputPlane* input;
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnToolbarContourSelect(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};