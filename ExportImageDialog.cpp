#include "ExportImageDialog.h"
#include "MainWindowFrame.h"
#include "InputPlane.h"

ExportImageDialog::ExportImageDialog(wxWindow* parent,
    const std::string& saveFileName, const std::string& saveFilePath,
    InputPlane* input, bool animation) :
    wxDialog(parent, wxID_ANY, "", wxDefaultPosition,
        wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    sizer(7, 3, 0, 0)
{
    wxSizerFlags flagsLeft(1);
    flagsLeft.Border(wxALL, 3).Proportion(0);
    wxSizerFlags flagsRight(1);
    flagsRight.Border(wxALL, 3).Proportion(1).Expand();

    auto clientSize = input->GetClientSize();
    double defaultDur = input->GetLongestAnimDur();

    std::string inputFileName, outputFileName;

    std::string AnimFilter = "Animated GIF(*.gif) | *.gif";
    std::string ImFilter = "PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPEG files (*.jpg)|*.jpg";
    std::string animLabel = "Animation";
    std::string imLabel = "Image";
    std::string* filter, *type;

    if (animation)
    {
        filter = &AnimFilter;
        type = &animLabel;
        if (saveFileName.length())
        {
            inputFileName = removeExt(saveFilePath) + "_input.gif";
            outputFileName = removeExt(saveFilePath) + "_output.gif";
        }
    }
    else
    {
        filter = &ImFilter;
        type = &imLabel;
        if (saveFileName.length())
        {
            inputFileName = removeExt(saveFilePath) + "_input.png";
            outputFileName = removeExt(saveFilePath) + "_output.png";
        }
    }
    SetTitle("Export " +  *type + "...");
    sizer.AddGrowableCol(2);

    inCheck.Create(this, wxID_ANY, "");
    sizer.Add(&inCheck, flagsLeft);
    inputFileLabel.Create(this, wxID_ANY, "Export Input " + *type + " : ");
    sizer.Add(&inputFileLabel, flagsLeft);
    inputAnimFile.Create(this, wxID_ANY, inputFileName,
        "Export " + *type + "...", *filter,
        wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT
        | wxFLP_USE_TEXTCTRL);
    sizer.Add(&inputAnimFile, flagsRight);

    outCheck.Create(this, wxID_ANY, "");
    sizer.Add(&outCheck, flagsLeft);
    outputFileLabel.Create(this, wxID_ANY, "Export Output " + *type + " : ");
    sizer.Add(&outputFileLabel, flagsLeft);
    outputAnimFile.Create(this, wxID_ANY, outputFileName,
        "Export " + *type + "...", *filter,
        wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT
        | wxFLP_USE_TEXTCTRL);
    sizer.Add(&outputAnimFile, flagsRight);

    sizer.AddSpacer(0);
    xResLabel.Create(this, wxID_ANY, "x Resolution: ");
    sizer.Add(&xResLabel, flagsLeft);
    xResCtrl.Create(this, wxID_ANY, std::to_string(clientSize.x),
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, clientSize.x);
    sizer.Add(&xResCtrl, flagsRight);
    sizer.AddSpacer(0);
    yResLabel.Create(this, wxID_ANY, "y Resolution: ");
    sizer.Add(&yResLabel, flagsLeft);
    yResCtrl.Create(this, wxID_ANY, std::to_string(clientSize.y),
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000, clientSize.y);
    sizer.Add(&yResCtrl, flagsRight);
    if (animation)
    {
        sizer.AddSpacer(0);
        durLabel.Create(this, wxID_ANY, "Duration: ");
        sizer.Add(&durLabel, flagsLeft);
        durCtrl.Create(this, wxID_ANY, std::to_string(defaultDur),
            wxDefaultPosition, xResCtrl.GetSize(), wxSP_ARROW_KEYS, 0, 10000, defaultDur);
        sizer.Add(&durCtrl, flagsRight);
        sizer.AddSpacer(0);
        fpsLabel.Create(this, wxID_ANY, "Frame Rate: ");
        sizer.Add(&fpsLabel, flagsLeft);
        fpsCtrl.Create(this, wxID_ANY, "30",
            wxDefaultPosition, xResCtrl.GetSize(), wxSP_ARROW_KEYS, 1, 1000, 30);
        sizer.Add(&fpsCtrl, flagsRight);
    }
    sizer.AddSpacer(0);
    sizer.Add(CreateButtonSizer(wxOK | wxCANCEL),
        wxSizerFlags(1).Border(wxALL, 3).Align(wxALIGN_RIGHT));
    SetSizerAndFit(&sizer);
}
