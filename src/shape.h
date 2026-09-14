#pragma once

#include <graphics.h>
#include <vector>

// 支持的图形类型
enum ShapeType {
    TOOL_LINE = 0,   // 直线
    TOOL_RECT,       // 矩形
    TOOL_ELLIPSE,    // 椭圆
    TOOL_CIRCLE,     // 圆（以拖拽起点为圆心）
    TOOL_TRIANGLE,   // 等腰三角形（拖拽框内接）
    TOOL_PENCIL,     // 自由曲线
    TOOL_COUNT
};

// 一个图形对象。
// 除 TOOL_PENCIL 外，几何形状均由拖拽产生的对角点 (x1,y1)-(x2,y2) 决定；
// TOOL_PENCIL 忽略这两个点，改用 pts 里的折线顶点。
struct Shape {
    ShapeType type = TOOL_LINE;
    int x1 = 0, y1 = 0;
    int x2 = 0, y2 = 0;
    COLORREF stroke = BLACK;   // 描边色
    COLORREF fill = WHITE;     // 填充色
    bool filled = false;       // 是否填充
    int lineWidth = 2;         // 线宽（像素）
    std::vector<POINT> pts;    // 仅自由曲线使用
};

// 把 shape 画到当前绘图目标上。
void DrawShape(const Shape& s);

// 画拖拽过程中的真实样式预览。
void DrawPreview(const Shape& s);
