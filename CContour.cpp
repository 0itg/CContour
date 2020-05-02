#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "MainWindowFrame.h"

// App which graphs a complex function as a mapping between two complex planes.
// Allows for drawing and editing of contours which are mapped in real time.

class ContourViewer : public wxApp
{
public:
    virtual bool OnInit();
};

wxDECLARE_APP(ContourViewer);
wxIMPLEMENT_APP(ContourViewer);

bool ContourViewer::OnInit()
{
    MainFrame* frame = new MainFrame(
        "Complex Contour Visualizer", wxDefaultPosition, wxSize(1300, 750));
    frame->Show(true);
    return true;
}