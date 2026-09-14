#include "ui.h"
#include <gdiplus.h>
#include <vector>
#include <cstdio>

namespace {
using namespace Gdiplus;
const COLORREF ink = RGB(43, 43, 58), muted = RGB(137, 137, 153);
const COLORREF purple = RGB(112, 88, 203), border = RGB(235, 234, 240);
const COLORREF strokes[] = {RGB(46,49,65),RGB(112,88,203),RGB(79,126,209),RGB(62,151,132),RGB(215,144,68),RGB(204,102,124),RGB(156,162,177),RGB(255,255,255)};
const COLORREF fills[] = {RGB(229,221,250),RGB(218,231,253),RGB(214,238,229),RGB(250,233,206),RGB(250,220,226),RGB(235,237,243),RGB(112,88,203),RGB(255,255,255)};
const wchar_t* names[] = {L"直线",L"矩形",L"椭圆",L"圆形",L"三角形",L"自由画笔"};
const int widths[] = {1,2,4,8};
struct Box { int x,y,w,h; bool Has(int px,int py) const { return px>=x && py>=y && px<x+w && py<y+h; } };
struct Control { Box box; UIAction action; };
std::vector<Control> Controls() {
    std::vector<Control> c;
    for(int i=0;i<TOOL_COUNT;++i) c.push_back({{16,148+i*58,144,48},{UI_SELECT_TOOL,i}});
    c.push_back({{680,22,78,34},{UI_UNDO}});
    c.push_back({{768,22,78,34},{UI_REDO}});
    c.push_back({{864,22,88,34},{UI_CLEAR}});
    c.push_back({{1076,18,140,42},{UI_SAVE}});
    for(int i=0;i<8;++i) {
        c.push_back({{1008+i%4*52,194+i/4*42,32,32},{UI_SELECT_STROKE,0,strokes[i]}});
        c.push_back({{1008+i%4*52,342+i/4*42,32,32},{UI_SELECT_FILL,0,fills[i]}});
    }
    c.push_back({{1158,296,48,26},{UI_TOGGLE_FILL}});
    for(int i=0;i<4;++i) c.push_back({{1008+i*52,468,44,40},{UI_SET_WIDTH,widths[i]}});
    c.push_back({{1158,547,48,26},{UI_TOGGLE_GRID}});
    return c;
}
Color C(COLORREF c) { return Color(255,GetRValue(c),GetGValue(c),GetBValue(c)); }
void Round(Graphics& g, Box r, COLORREF fill, int radius=10, COLORREF edge=CLR_INVALID) {
    GraphicsPath p;
    const REAL d=(REAL)radius*2, x=(REAL)r.x,y=(REAL)r.y,w=(REAL)r.w,h=(REAL)r.h;
    p.AddArc(x,y,d,d,180,90); p.AddArc(x+w-d,y,d,d,270,90);
    p.AddArc(x+w-d,y+h-d,d,d,0,90); p.AddArc(x,y+h-d,d,d,90,90); p.CloseFigure();
    SolidBrush brush(C(fill)); g.FillPath(&brush,&p);
    if(edge!=CLR_INVALID) { Pen pen(C(edge),1); g.DrawPath(&pen,&p); }
}
void Text(Graphics& g,int x,int y,const wchar_t* text,int size=14,COLORREF color=RGB(43,43,58),bool bold=false) {
    Font font(L"Microsoft YaHei UI",(REAL)size,bold?FontStyleBold:FontStyleRegular,UnitPixel);
    SolidBrush brush(C(color));
    g.DrawString(text,-1,&font,PointF((REAL)x,(REAL)y),&brush);
}
void Rule(Graphics& g,int x,int y,int w) { Pen p(C(border)); g.DrawLine(&p,x,y,x+w,y); }
void Icon(Graphics& g,int type,int x,int y,COLORREF color,int size=22) {
    Pen pen(C(color),1.8f); pen.SetLineJoin(LineJoinRound); pen.SetStartCap(LineCapRound); pen.SetEndCap(LineCapRound);
    int e=size;
    switch(type) {
    case TOOL_LINE: g.DrawLine(&pen,x+2,y+e-2,x+e-2,y+2); break;
    case TOOL_RECT: g.DrawRectangle(&pen,x+2,y+3,e-4,e-6); break;
    case TOOL_ELLIPSE: g.DrawEllipse(&pen,x+1,y+5,e-2,e-10); break;
    case TOOL_CIRCLE: g.DrawEllipse(&pen,x+2,y+2,e-4,e-4); break;
    case TOOL_TRIANGLE: { Point p[]={{x+e/2,y+2},{x+e-1,y+e-2},{x+1,y+e-2}}; g.DrawPolygon(&pen,p,3); break; }
    default: { Point p[]={{x+1,y+17},{x+6,y+6},{x+10,y+17},{x+15,y+5},{x+21,y+10}}; g.DrawCurve(&pen,p,5); }
    }
}
void Toggle(Graphics& g,Box b,bool on,bool hover) {
    Round(g,b,on?purple:(hover?RGB(207,205,218):RGB(225,224,232)),13);
    SolidBrush white(C(WHITE)); g.FillEllipse(&white,b.x+(on?25:3),b.y+3,20,20);
}
}

const wchar_t* ToolName(ShapeType tool) { return tool>=0 && tool<TOOL_COUNT?names[tool]:L""; }
bool InCanvas(int x,int y) { return Box{CANVAS_X,CANVAS_Y,CANVAS_W,CANVAS_H}.Has(x,y); }
UIAction HitTest(int x,int y) {
    for(const auto& c:Controls()) if(c.box.Has(x,y)) return c.action;
    return {};
}

void DrawWorkspace(const UIState& st) {
    Graphics g(GetImageHDC());
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g.Clear(C(RGB(246,245,249)));
    SolidBrush white(C(WHITE));
    g.FillRectangle(&white,0,0,WIN_W,76);
    g.FillRectangle(&white,0,77,176,687);
    g.FillRectangle(&white,984,77,256,687);
    g.FillRectangle(&white,0,765,WIN_W,35);
    Rule(g,0,76,WIN_W); Rule(g,0,764,WIN_W);
    Round(g,{24,22,34,34},purple,10);
    Icon(g,TOOL_TRIANGLE,31,28,WHITE,20);
    Text(g,70,16,L"几何工坊",21,ink,true);
    Text(g,72,44,L"G E O M E T R Y   S T U D I O",9,muted);
    Text(g,292,28,L"未命名作品",14,ink);
    Text(g,399,30,L"/  自由创作",12,muted);
    Text(g,24,108,L"绘图工具",12,muted,true);
    Text(g,1008,104,L"样式面板",18,ink,true);
    Text(g,1008,132,L"为下一个图形搭配风格",12,muted);
    Text(g,1008,167,L"描边颜色",13,ink,true);
    Text(g,1008,300,L"图形填充",13,ink,true);
    Text(g,1008,441,L"线条粗细",13,ink,true);
    Text(g,1008,548,L"辅助点阵",13,ink,true);
    Text(g,1008,583,L"仅辅助绘制，不会出现在导出图片中",11,muted);
    Rule(g,1008,280,208); Rule(g,1008,424,208); Rule(g,1008,529,208);
    Text(g,208,104,L"画布",16,ink,true);
    Text(g,255,108,L"744 × 574 px",12,muted);
    Text(g,824,108,L"100%  ·  自由绘制",12,muted);
    Round(g,{205,143,750,580},RGB(230,228,237),5);
    Round(g,{207,141,746,576},WHITE,3,border);
    for(const auto& control:Controls()) {
        const Box& b=control.box;
        const UIAction& a=control.action;
        bool hover=b.Has(st.mouseX,st.mouseY);
        if(a.type==UI_SELECT_TOOL) {
            bool selected=st.tool==a.value;
            if(selected||hover) Round(g,b,selected?RGB(239,234,252):RGB(247,246,250),10);
            Icon(g,a.value,b.x+14,b.y+13,selected?purple:RGB(103,104,124));
            Text(g,b.x+46,b.y+13,names[a.value],14,selected?purple:ink,selected);
            wchar_t key[3]; swprintf_s(key,L"%d",a.value+1);
            Text(g,b.x+124,b.y+16,key,11,selected?purple:muted);
        } else if(a.type==UI_SELECT_STROKE || a.type==UI_SELECT_FILL) {
            bool selected=a.type==UI_SELECT_STROKE?st.stroke==a.color:st.fill==a.color&&st.filled;
            if(selected||hover) Round(g,{b.x-3,b.y-3,b.w+6,b.h+6},WHITE,11,selected?purple:RGB(204,199,222));
            Round(g,b,a.color,8,a.color==WHITE?border:CLR_INVALID);
            if(selected) {
                COLORREF check=(GetRValue(a.color)+GetGValue(a.color)+GetBValue(a.color)>550)?purple:WHITE;
                Pen p(C(check),2); g.DrawLine(&p,b.x+10,b.y+16,b.x+14,b.y+20); g.DrawLine(&p,b.x+14,b.y+20,b.x+23,b.y+11);
            }
        } else if(a.type==UI_TOGGLE_FILL || a.type==UI_TOGGLE_GRID) {
            Toggle(g,b,a.type==UI_TOGGLE_FILL?st.filled:st.grid,hover);
        } else if(a.type==UI_SET_WIDTH) {
            bool selected=st.lineWidth==a.value;
            Round(g,b,selected?RGB(239,234,252):(hover?RGB(245,244,248):WHITE),8,selected?RGB(207,195,241):border);
            Pen p(C(selected?purple:muted),(REAL)a.value); p.SetStartCap(LineCapRound); p.SetEndCap(LineCapRound);
            g.DrawLine(&p,b.x+13,b.y+14,b.x+31,b.y+14);
            wchar_t label[8]; swprintf_s(label,L"%d",a.value); Text(g,b.x+18,b.y+25,label,9,selected?purple:muted);
        } else {
            bool primary=a.type==UI_SAVE;
            bool enabled= a.type==UI_UNDO?st.canUndo:a.type==UI_REDO?st.canRedo:a.type==UI_CLEAR?st.shapeCount>0:true;
            Round(g,b,primary?(hover?RGB(96,72,184):purple):(hover&&enabled?RGB(241,238,248):WHITE),9,primary?CLR_INVALID:border);
            const wchar_t* label=primary?L"导出 PNG  ↗":a.type==UI_UNDO?L"↶  撤销":a.type==UI_REDO?L"↷  重做":L"清空画布";
            Text(g,b.x+(primary?22:12),b.y+(primary?11:8),label,13,primary?WHITE:enabled?ink:RGB(190,189,200),primary);
        }
    }
    Rule(g,24,524,128);
    Text(g,25,548,L"简单形状",15,ink,true);
    Text(g,25,573,L"也有无限可能。",13,muted);
    Icon(g,TOOL_CIRCLE,30,638,RGB(206,194,237),38);
    Icon(g,TOOL_TRIANGLE,67,625,RGB(166,146,217),44);
    Icon(g,TOOL_RECT,99,649,RGB(209,199,233),32);
    Text(g,25,727,L"EASYX  /  C++",10,muted);
    Round(g,{1008,634,208,104},RGB(247,245,252),12);
    Text(g,1022,646,L"创作小提示",12,purple,true);
    Text(g,1022,671,L"Shift   约束比例与方向",11,muted);
    Text(g,1022,692,L"Esc     取消当前绘制",11,muted);
    Text(g,1022,713,L"Ctrl+S  导出你的作品",11,muted);
    Text(g,208,733,L"拖动鼠标开始创作  ·  按 1–6 快速切换工具",11,muted);
    SolidBrush dot(C(RGB(99,168,142))); g.FillEllipse(&dot,22,779,6,6);
    Text(g,36,773,L"就绪",11,muted);
    Text(g,100,773,ToolName(st.tool),11,ink);
    wchar_t status[120]; swprintf_s(status,L"%d 个图形    ·    %d px 描边",st.shapeCount,st.lineWidth);
    Text(g,208,773,status,11,muted);
    if(InCanvas(st.mouseX,st.mouseY)) { swprintf_s(status,L"X %03d   Y %03d",st.mouseX-CANVAS_X,st.mouseY-CANVAS_Y); Text(g,784,773,status,11,muted); }
    Text(g,1020,773,L"Ctrl+Z 撤销  /  Ctrl+Y 重做",11,muted);
}

void DrawOverlay(const UIState& st) {
    if(st.notice.empty()) return;
    Graphics g(GetImageHDC()); g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    Round(g,{370,661,432,40},RGB(49,45,66),12);
    Text(g,390,672,st.notice.c_str(),12,WHITE);
}
