#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <string>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>

class InputPlane;

class ExportImageDialog : public wxDialog
{
public:
	ExportImageDialog(wxWindow* parent, const std::string& saveFileName,
		const std::string& saveFilePath, InputPlane* input, bool animation = false);
	~ExportImageDialog() {}

	wxCheckBox inCheck, outCheck;
	wxFilePickerCtrl inputAnimFile, outputAnimFile;
	wxSpinCtrl xResCtrl, yResCtrl;
	wxSpinCtrlDouble durCtrl, fpsCtrl;
	wxFlexGridSizer sizer;
	wxStaticText inputFileLabel, outputFileLabel, xResLabel,
		yResLabel, durLabel, fpsLabel;
};

