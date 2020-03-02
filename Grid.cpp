#include "Grid.h"
#include "ComplexPlane.h"
#include "ContourLine.h"
#include "ContourPolygon.h"
#include "Parser.h"

#include <algorithm>
//#include <thread>
//#include <execution>

Grid::~Grid() {
   for (auto v : lines)
      delete v;
}

void Grid::Draw(wxDC* dc, ComplexPlane* canvas) {
   CalcVisibleGrid();
   for (auto v : lines)
      v->Draw(dc, canvas);
}

void Grid::CalcVisibleGrid() {
   // Draws gridlines every hStep x VStep, offset to line up with the origin
   // If the viewport has been panned around.
   for (auto v : lines)
      delete v;
   lines.clear();

   wxPoint corner(parent->GetClientSize().x, parent->GetClientSize().y);
   double hOffset = fmod(parent->ScreenToComplex(wxPoint(0, 0)).real(), hStep);
   double vOffset = fmod(parent->ScreenToComplex(corner).imag(), vStep);

   for (double y = parent->axes.imagMin - vOffset; y <= parent->axes.imagMax;
        y += vStep) {
      lines.push_back(new ContourLine(
          cplx(parent->ScreenToComplex(wxPoint(0, 0)).real(), y),
          cplx(parent->ScreenToComplex(corner).real(), y)));
   }
   for (double x = parent->axes.realMin - hOffset; x <= parent->axes.realMax;
        x += hStep) {
      lines.push_back(new ContourLine(
          cplx(x, parent->ScreenToComplex(wxPoint(x, 0)).imag()),
          cplx(x, parent->ScreenToComplex(corner).imag())));
   }
}

TransformedGrid::~TransformedGrid() {
   for (auto v : lines)
      delete v;
   Grid::~Grid();
}

void TransformedGrid::Draw(wxDC* dc, ComplexPlane* canvas) {
   for (auto v : lines)
      v->Draw(dc, canvas);
}

void TransformedGrid::MapGrid(Grid* grid, ParsedFunc<cplx>& f) {
   for (auto v : lines)
      delete v;
   lines.clear();

   for (auto v : grid->lines) {
      lines.push_back(new ContourPolygon());
      double t;
      for (double i = 0; i <= res; i++) {
         t = i / res;
         lines.back()->AddPoint(
             f(v->GetCtrlPoint(0) * t + v->GetCtrlPoint(1) * (1 - t)));

         // In the case of division by zero, move along the gridline
         // a bit further until we find a defined point.
         // Rarely should this take more than one step.
         while (isnan(lines.back()->GetCtrlPoint(i).real())) {
            lines.back()->RemovePoint(i);
            double t_avoid_pole = 1.0 / res / 100;
            lines.back()->AddPoint(
                f(v->GetCtrlPoint(0) * (t + t_avoid_pole) +
                  v->GetCtrlPoint(1) * (1 - t - t_avoid_pole)));
         }
         while (isnan(lines.back()->GetCtrlPoint(i).imag())) {
            lines.back()->RemovePoint(i);
            double t_avoid_pole = 1.0 / res / 100;
            lines.back()->AddPoint(
                f(v->GetCtrlPoint(0) * (t + t_avoid_pole) +
                  v->GetCtrlPoint(1) * (1 - t - t_avoid_pole)));
         }
      }
   }
   // Multithreaded version of the same code. Functions, but isn't noticeably
   //  faster. Drawing is probably the bottleneck.

   // auto size = grid->lines.size();
   // for (int i= 0; i < size; i++)
   //   lines.push_back(new ContourPolygon());

   // std::vector<ParsedFunc<cplx>> funcs;
   // std::vector<std::thread> threads;
   // auto threadCount = std::thread::hardware_concurrency();
   //
   // copy function once per thread because it isn't thread safe
   // (for testing purposes. copies would be precalculated in real code)
   // for (size_t thrd = 0; thrd < threadCount; thrd++)
   //   funcs.push_back(f);
   // for (size_t thrd = 0; thrd < threadCount; thrd++) {
   //   threads.emplace_back([this, &grid, &funcs, size, thrd, threadCount]() {
   //      double t;
   //      for (size_t k = thrd; k < size; k += threadCount) {
   //         for (double i = 0; i <= res; i++) {
   //            t      = i / res;
   //            auto v = grid->lines[k];
   //            lines[k]->AddPoint(funcs[thrd](v->GetCtrlPoint(0) * t +
   //                                 v->GetCtrlPoint(1) * (1 - t)));

   //            // In the case of division by zero, move along the gridline
   //            // a bit further until we find a defined point.
   //            // Rarely should this take more than one step.
   //            while (isnan(lines[k]->GetCtrlPoint(i).real())) {
   //               lines[k]->RemovePoint(i);
   //               double t_avoid_pole = i / res / 100;
   //               lines[k]->AddPoint(
   //                   funcs[thrd](v->GetCtrlPoint(0) * (t + t_avoid_pole) +
   //                     v->GetCtrlPoint(1) * (1 - t - t_avoid_pole)));
   //            }
   //            while (isnan(lines[k]->GetCtrlPoint(i).imag())) {
   //               lines[k]->RemovePoint(i);
   //               double t_avoid_pole = i / res / 100;
   //               lines[k]->AddPoint(
   //                   funcs[thrd](v->GetCtrlPoint(0) * (t + t_avoid_pole) +
   //                     v->GetCtrlPoint(1) * (1 - t - t_avoid_pole)));
   //            }
   //         }
   //      }
   //   });
   //}
   // for (auto& th : threads)
   //   th.join();
}
