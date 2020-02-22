#pragma once
#include "ComplexPlane.h"
#include "Event_IDs.h"

#include <wx/clrpicker.h>

class ContourPolygon;
class OutputPlane;
class Grid;

// Left Panel in UI. User draws on the panel, then the points are stored as
// complex numbers and mapped to the ouput panel under some complex function.

class InputPlane : public ComplexPlane {
   friend class OutputPlane;

 public:
   InputPlane(wxWindow* parent, std::string name = "input");
   ~InputPlane();

   void OnMouseLeftUpContourTools(wxMouseEvent& mouse);
   void OnMouseLeftUpPaintbrush(wxMouseEvent& mouse);
   void OnMouseLeftUpSelectionTool(wxMouseEvent& mouse);
   void OnMouseRightUp(wxMouseEvent& mouse);
   //void OnMouseRightDown(wxMouseEvent& mouse);
   // void OnMouseMiddleDown(wxMouseEvent& mouse);
   // void OnMouseMiddleUp(wxMouseEvent& mouse);
   void OnMouseWheel(wxMouseEvent& mouse);
   void OnMouseMoving(wxMouseEvent& mouse);
   void OnKeyUp(wxKeyEvent& Key);
   void OnPaint(wxPaintEvent& paint);
   void OnColorPicked(wxColourPickerEvent& colorPicked);
   void OnColorRandomizer(wxCommandEvent& event);
   void OnContourResCtrl(wxSpinEvent& event);
   void OnContourResCtrl(wxCommandEvent& event);

   int GetState() { return state; }
   void RecalcAll();

   // "Type" meaning Circle, Polygon, Rectangle, etc.
   void SetContourType(int id);
   void RemoveContour(int index);
   Contour* CreateContour(wxPoint mousePos);

   void SetColorPicker(wxColourPickerCtrl* ptr) {
      colorPicker = ptr;
   };
   wxColor RandomColor();

   // If true, when axes step values change, grid step values
   // change accordingly
   bool linkGridToAxes                  = true;
   bool randomizeColor                  = true;
   wxColor color                        = wxColor(0, 0, 200);
   const wxColor BGcolor                = *wxWHITE;
   const int COLOR_SIMILARITY_THRESHOLD = 96;

 private:
   const int CIRCLED_POINT_RADIUS = 7;
   int res                        = 100;
   // drawnContours stores contours in original form for editing.
   // subDivContours stores them as approximating polygons for mapping
   std::vector<ContourPolygon*> subDivContours;
   // Pointers to outputs for or sending refresh signals.
   // App only uses one output for now, but more might be nice later.
   std::vector<OutputPlane*> outputs;
   wxColourPickerCtrl* colorPicker = nullptr;

   Grid* grid;
   int contourType = ID_Circle;
   wxDECLARE_EVENT_TABLE();
};