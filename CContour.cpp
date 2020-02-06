#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/dcbuffer.h"
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

