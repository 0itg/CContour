#pragma once

#pragma warning(push, 0)
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#pragma warning(pop)

#include <complex>

struct Axes;
class ToolPanel;
class InputPlane;
class ContourPolygon;
class ComplexPlane;
class ToolPanel;

// All contours include a set of points, may be open or closed,
// and need to define a function for interpolating the points
// (e.g. straight lines for polygons, or some spline function)

class Contour {
   friend class ToolPanel;
 public:
   virtual ~Contour() {}
   virtual Contour* Clone() = 0;

   virtual void Draw(wxDC* dc, ComplexPlane* canvas) = 0;
   virtual void AddPoint(std::complex<double> mousePos);
   virtual void RemovePoint(int index);
   virtual void moveCtrlPoint(std::complex<double> mousePos, int ptIndex = -1);

   // Action to be taken when selecting the contour between control
   // points. Typically it will just call the Translate function.
   virtual void ActionNoCtrlPoint(std::complex<double> mousePos,
                                  std::complex<double> lastPointClicked) = 0;

   virtual void Translate(std::complex<double> z1, std::complex<double> z2);

   // Returns true if editing is finished on this contour.
   virtual bool IsDone() = 0;

   virtual bool IsPointOnContour(std::complex<double> pt, ComplexPlane* canvas,
                            int pixPrecision = 3) = 0;

   // Returns the index of the control point under the mouse, -1 if none.
   virtual int OnCtrlPoint(std::complex<double> pt, ComplexPlane* canvas,
                           int pixPrecision = 3);
   virtual std::complex<double> GetCtrlPoint(int index);
   virtual void SetCtrlPoint(int index, std::complex<double> c);
   virtual std::string& GetName() { return name; }

   // Any actions to be taken when editing is finished.
   virtual void Finalize(){};

   // Base version draws the Contour's name, followed by a list of control
   // points and LinkedTextCtrl's linked to them.
   virtual void PopulateMenu(ToolPanel* TP);
   // Allows derived classes to add extra menu items before the control points,
   // E.g. a LinkedTextCtrl for the radius of a circle. Takes upper left
   // coordinate, returns size of menu, so both sides can manage their layouts.
   virtual wxSize PopulateSupplementalMenu(ToolPanel* TP, wxPoint UL) {
      return wxSize(0,0);
   }

   // Parameterizing the contour as g(t) with 0 < t < 1, returns g(t).
   virtual std::complex<double> Interpolate(double t) = 0;

   // Creates a polygonal approximation of the contour with number of segments
   // dependent on res. No actual necessity for the number to equal res
   // precisely, but it would be expected from the user.
   virtual ContourPolygon* Subdivide(int res) = 0;
   int GetPointCount() {
      return (int)points.size();
   }

   void DrawCtrlPoint(wxDC* dc, wxPoint p);
   wxColor color = *wxRED;

 protected:
   std::string name;
   std::vector<std::complex<double>> points;
};

double DistancePointToLine(std::complex<double> pt, std::complex<double> z1,
                           std::complex<double> z2);
bool IsInsideBox(std::complex<double> pt, std::complex<double> z1,
                 std::complex<double> z2);

void DrawClippedLine(wxPoint p1, wxPoint p2, wxDC* dc, ComplexPlane* canvas);