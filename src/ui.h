#pragma once
#include <graphics.h>
#include <string>
#include "shape.h"

constexpr int WIN_W = 1240, WIN_H = 800;
constexpr int CANVAS_X = 208, CANVAS_Y = 142;
constexpr int CANVAS_W = 744, CANVAS_H = 574;

enum UIActionType {
    UI_NONE, UI_SELECT_TOOL, UI_SELECT_STROKE, UI_SELECT_FILL,
    UI_TOGGLE_FILL, UI_SET_WIDTH, UI_UNDO, UI_REDO, UI_CLEAR,
    UI_SAVE, UI_TOGGLE_GRID
};
struct UIAction {
    UIActionType type = UI_NONE;
    int value = 0;
    COLORREF color = BLACK;
};
struct UIState {
    ShapeType tool = TOOL_RECT;
    COLORREF stroke = RGB(112, 88, 203);
    COLORREF fill = RGB(229, 221, 250);
    bool filled = true;
    int lineWidth = 2;
    int shapeCount = 0;
    bool canUndo = false, canRedo = false, grid = false;
    int mouseX = -1, mouseY = -1;
    std::wstring notice;
};
void DrawWorkspace(const UIState& state);
void DrawOverlay(const UIState& state);
UIAction HitTest(int x, int y);
const wchar_t* ToolName(ShapeType tool);
bool InCanvas(int x, int y);
