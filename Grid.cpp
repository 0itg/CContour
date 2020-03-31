#include "Grid.h"

//#include <thread>
//#include <execution>

BOOST_CLASS_EXPORT_IMPLEMENT(TransformedGrid)

void Grid::Draw(wxDC* dc, ComplexPlane* canvas)
{
    for (auto& v : lines)
        v->Draw(dc, canvas);
}

void Grid::CalcVisibleGrid()
{
    // Draws gridlines every hStep x VStep, offset to line up with the origin
    // If the viewport has been panned around.
    lines.clear();
    const auto hMin = parent->axes.realMin;
    const auto hMax = parent->axes.realMax;
    const auto vMin = parent->axes.imagMin;
    const auto vMax = parent->axes.imagMax;
    lines.reserve((hMax - hMin) / hStep + (vMax - vMin) / vStep);

    auto ULcorner = parent->ScreenToComplex(wxPoint(0, 0));
    auto BRcorner = parent->ScreenToComplex(
        wxPoint(parent->GetClientSize().x, parent->GetClientSize().y));

    double hOffset = fmod(ULcorner.real(), hStep);
    double vOffset = fmod(BRcorner.imag(), vStep);

    for (double y = vMin - vOffset; y <= vMax; y += vStep)
    {
        lines.push_back(std::make_unique<ContourLine>(
            cplx(ULcorner.real(), y), cplx(BRcorner.real(), y)));
    }
    for (double x = hMin - hOffset; x <= hMax; x += hStep)
    {
        lines.push_back(std::make_unique<ContourLine>(
            cplx(x, ULcorner.imag()), cplx(x, BRcorner.imag())));
    }
}

void TransformedGrid::Draw(wxDC* dc, ComplexPlane* canvas)
{
    for (auto& v : lines)
        v->Draw(dc, canvas);
}

void TransformedGrid::MapGrid(const Grid& grid, ParsedFunc<cplx>& f)
{
    lines.clear();
    lines.reserve(grid.lines.size());

    for (auto& v : grid.lines)
    {
        auto p1 = v->GetCtrlPoint(0);
        auto p2 = v->GetCtrlPoint(1);
        lines.push_back(std::make_unique<ContourPolygon>());
        double t;
        for (double i = 0; i <= res; i++)
        {
            t        = i / res;
            cplx p_i = f(p1 * t + p2 * (1 - t));

            // In the case of division by zero, move along the gridline
            // a bit further until we find a defined point.
            // Rarely should this take more than one step.
            while (isnan(p_i.real()) || isnan(p_i.imag()))
            {
                double t_avoid_pole = 1.0 / res / 100;
                p_i = f(p1 * (t + t_avoid_pole) + p2 * (1 - t - t_avoid_pole));
            }

            lines.back()->AddPoint(p_i);
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
