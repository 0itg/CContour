#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>

#include "MainWindowFrame.h"

// App which graphs a complex function as a mapping between two complex planes.
// Allows for drawing and editing of contours which are mapped in real time.
// The function is currently hardcoded as f: z->z^2, but the plan is to support
// arbitrary function input.

class ContourViewer : public wxApp {
 public:
   virtual bool OnInit();
};

#pragma warning(push, 0)
wxIMPLEMENT_APP(ContourViewer);
#pragma warning(pop)

bool ContourViewer::OnInit()
{
   MainWindowFrame* frame = new MainWindowFrame(
       "Complex Contour Visualizer", wxDefaultPosition, wxSize(1250, 650));
   frame->Show(true);
   return true;
}
