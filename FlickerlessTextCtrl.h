#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/display.h>

class FlickerlessTextCtrl : public wxStaticText {
 public:
   FlickerlessTextCtrl(wxWindow* parent, wxWindowID id,
                                      const wxString& label, const wxPoint& pos,
                                      const wxSize& size   = wxDefaultSize,
                                      long style           = 0,
                                      const wxString& name = "staticText")
       : wxStaticText(parent, id, label, pos, size, style, name){};
   void OnEraseBackGround(wxEraseEvent& event){};
   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(FlickerlessTextCtrl, wxStaticText)
EVT_ERASE_BACKGROUND(FlickerlessTextCtrl::OnEraseBackGround)
END_EVENT_TABLE()