#include "MainWindowFrame.h"

class ContourViewer : public wxApp
{
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(ContourViewer);

bool ContourViewer::OnInit()
{
    MainWindowFrame* frame = new MainWindowFrame("Complex Contour Visualizer",
        wxDefaultPosition, wxSize(1100, 650));
    frame->Show(true);
    return true;
}

