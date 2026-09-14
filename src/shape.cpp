#include "shape.h"
#include <gdiplus.h>
#include <algorithm>
#include <cmath>

namespace {
Gdiplus::Color ToColor(COLORREF c) {
    return Gdiplus::Color(255, GetRValue(c), GetGValue(c), GetBValue(c));
}
}

// EasyX 管理窗口和绘图目标，GDI+ 为几何边缘提供抗锯齿。
void DrawShape(const Shape& s) {
    using namespace Gdiplus;
    Graphics g(GetImageHDC(GetWorkingImage()));
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    Pen pen(ToColor(s.stroke), (REAL)s.lineWidth);
    pen.SetStartCap(LineCapRound);
    pen.SetEndCap(LineCapRound);
    pen.SetLineJoin(LineJoinRound);
    SolidBrush brush(ToColor(s.fill));
    REAL x = (REAL)(std::min)(s.x1, s.x2);
    REAL y = (REAL)(std::min)(s.y1, s.y2);
    REAL w = (REAL)std::abs(s.x2 - s.x1);
    REAL h = (REAL)std::abs(s.y2 - s.y1);
    switch (s.type) {
    case TOOL_LINE:
        g.DrawLine(&pen, s.x1, s.y1, s.x2, s.y2);
        break;
    case TOOL_RECT:
        if (s.filled) g.FillRectangle(&brush, x, y, w, h);
        g.DrawRectangle(&pen, x, y, w, h);
        break;
    case TOOL_CIRCLE: {
        REAL r = (REAL)std::hypot(s.x2 - s.x1, s.y2 - s.y1);
        x = s.x1 - r; y = s.y1 - r; w = h = r * 2;
    }
        // 圆与椭圆共享轮廓绘制逻辑。
        [[fallthrough]];
    case TOOL_ELLIPSE:
        if (s.filled) g.FillEllipse(&brush, x, y, w, h);
        g.DrawEllipse(&pen, x, y, w, h);
        break;
    case TOOL_TRIANGLE: {
        PointF points[] = {{x + w / 2, y}, {x + w, y + h}, {x, y + h}};
        if (s.filled) g.FillPolygon(&brush, points, 3);
        g.DrawPolygon(&pen, points, 3);
        break;
    }
    case TOOL_PENCIL:
        if (s.pts.size() > 1) {
            std::vector<Point> points;
            points.reserve(s.pts.size());
            for (const auto& p : s.pts) points.emplace_back(p.x, p.y);
            g.DrawLines(&pen, points.data(), (INT)points.size());
        }
        break;
    default: break;
    }
}

void DrawPreview(const Shape& s) {
    // 所见即所得：预览保留实际颜色、填充与线宽。
    DrawShape(s);
}
