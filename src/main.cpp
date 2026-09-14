#include <graphics.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <commdlg.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>
#include "shape.h"
#include "ui.h"

namespace {
std::vector<Shape> shapes;
UIState ui;
IMAGE canvas(CANVAS_W, CANVAS_H);
IMAGE committed(CANVAS_W, CANVAS_H);
bool cacheDirty = true;
WNDPROC previousWindowProc = nullptr;
std::atomic<bool> closeRequested{false};

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_CLOSE) { closeRequested = true; return 0; }
    if (message == WM_LBUTTONDOWN && InCanvas(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam))) SetCapture(window);
    if (message == WM_LBUTTONUP || message == WM_RBUTTONDOWN ||
        (message == WM_KEYDOWN && wparam == VK_ESCAPE)) ReleaseCapture();
    return CallWindowProc(previousWindowProc, window, message, wparam, lparam);
}

ULONGLONG noticeUntil = 0;

// 每次操作只记录变化；清空也可撤销，不复制每一帧的整个文档。
struct Change { bool clear = false; std::vector<Shape> removed; };
std::vector<Change> undoStack, redoStack;
constexpr size_t HISTORY_LIMIT = 100;

void Notify(const std::wstring& text) {
    ui.notice = text;
    noticeUntil = GetTickCount64() + 4000;
}

void Record(Change change) {
    undoStack.push_back(std::move(change));
    if (undoStack.size() > HISTORY_LIMIT) undoStack.erase(undoStack.begin());
    redoStack.clear();
    cacheDirty = true;
}

void Undo() {
    if (undoStack.empty()) return;
    Change change = std::move(undoStack.back());
    undoStack.pop_back();
    if (change.clear) shapes = change.removed;
    else if (!shapes.empty()) shapes.pop_back();
    redoStack.push_back(std::move(change));
    cacheDirty = true;
    Notify(L"已撤销 · Ctrl+Y 可重做");
}

void Redo() {
    if (redoStack.empty()) return;
    Change change = std::move(redoStack.back());
    redoStack.pop_back();
    if (change.clear) shapes.clear();
    else shapes.push_back(change.removed.front());
    undoStack.push_back(std::move(change));
    cacheDirty = true;
    Notify(L"已重做");
}

void RenderCanvas(bool grid, const Shape* preview) {
    if (cacheDirty) {
        SetWorkingImage(&committed);
        setbkcolor(WHITE);
        cleardevice();
        if (grid) {
            for (int y = 14; y < CANVAS_H; y += 24)
                for (int x = 14; x < CANVAS_W; x += 24)
                    putpixel(x, y, RGB(222, 223, 232));
        }
        for (const Shape& shape : shapes) DrawShape(shape);
        SetWorkingImage();
        cacheDirty = false;
    }
    SetWorkingImage(&canvas);
    putimage(0, 0, &committed);
    if (preview) DrawPreview(*preview);
    SetWorkingImage();
}

void Redraw(const Shape* preview = nullptr) {
    RenderCanvas(ui.grid, preview);
    ui.shapeCount = (int)shapes.size();
    ui.canUndo = !undoStack.empty();
    ui.canRedo = !redoStack.empty();
    DrawWorkspace(ui);
    putimage(CANVAS_X, CANVAS_Y, &canvas);
    DrawOverlay(ui);
    FlushBatchDraw();
}

int PngEncoder(CLSID& id) {
    UINT count = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&count, &size);
    if (!size) return -1;
    std::vector<BYTE> storage(size);
    auto codecs = reinterpret_cast<Gdiplus::ImageCodecInfo*>(storage.data());
    if (Gdiplus::GetImageEncoders(count, size, codecs) != Gdiplus::Ok) return -1;
    for (UINT i = 0; i < count; ++i) {
        if (wcscmp(codecs[i].MimeType, L"image/png") == 0) { id = codecs[i].Clsid; return 0; }
    }
    return -1;
}

void SaveCanvas() {
    wchar_t path[MAX_PATH] = L"几何作品.png";
    OPENFILENAMEW dialog = {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetHWnd();
    dialog.lpstrFilter = L"PNG 图片 (*.png)\0*.png\0\0";
    dialog.lpstrFile = path;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = L"png";
    dialog.lpstrTitle = L"导出画布 · 几何工坊";
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetSaveFileNameW(&dialog)) {
        if (CommDlgExtendedError()) Notify(L"无法打开保存窗口，请重试");
        return;
    }
    // 单独绘制纯白导出目标，不包含辅助网格、界面和预览。
    IMAGE image(CANVAS_W, CANVAS_H);
    SetWorkingImage(&image);
    setbkcolor(WHITE);
    cleardevice();
    for (const Shape& shape : shapes) DrawShape(shape);
    SetWorkingImage();
    CLSID encoder;
    bool saved = false;
    if (PngEncoder(encoder) == 0) {
        Gdiplus::Bitmap bitmap(CANVAS_W, CANVAS_H, PixelFormat24bppRGB);
        {
            Gdiplus::Graphics graphics(&bitmap);
            HDC target = graphics.GetHDC();
            BitBlt(target, 0, 0, CANVAS_W, CANVAS_H, GetImageHDC(&image), 0, 0, SRCCOPY);
            graphics.ReleaseHDC(target);
        }
        saved = bitmap.Save(path, &encoder, nullptr) == Gdiplus::Ok;
    }
    Notify(saved ? L"已导出 PNG · 纯净画布，不含网格" : L"导出失败，请检查路径或文件权限");
}

void ApplyAction(const UIAction& action) {
    switch (action.type) {
    case UI_SELECT_TOOL: ui.tool = (ShapeType)action.value; break;
    case UI_SELECT_STROKE: ui.stroke = action.color; break;
    case UI_SELECT_FILL: ui.fill = action.color; ui.filled = true; break;
    case UI_TOGGLE_FILL: ui.filled = !ui.filled; break;
    case UI_SET_WIDTH: ui.lineWidth = action.value; break;
    case UI_TOGGLE_GRID: ui.grid = !ui.grid; cacheDirty = true; break;
    case UI_UNDO: Undo(); break;
    case UI_REDO: Redo(); break;
    case UI_CLEAR:
        if (!shapes.empty()) {
            Change change; change.clear = true; change.removed = shapes;
            shapes.clear(); Record(std::move(change));
            Notify(L"画布已清空 · Ctrl+Z 可以恢复");
        }
        break;
    case UI_SAVE: SaveCanvas(); break;
    default: break;
    }
}

void UpdateShape(Shape& shape, int x, int y, bool shift) {
    x = std::clamp(x - CANVAS_X, 0, CANVAS_W - 1);
    y = std::clamp(y - CANVAS_Y, 0, CANVAS_H - 1);
    int dx = x - shape.x1, dy = y - shape.y1;
    if (shift && (shape.type == TOOL_RECT || shape.type == TOOL_ELLIPSE || shape.type == TOOL_TRIANGLE)) {
        int length = (std::max)(std::abs(dx), std::abs(dy));
        length = (std::min)(length, dx < 0 ? shape.x1 : CANVAS_W - 1 - shape.x1);
        length = (std::min)(length, dy < 0 ? shape.y1 : CANVAS_H - 1 - shape.y1);
        x = shape.x1 + (dx < 0 ? -length : length);
        y = shape.y1 + (dy < 0 ? -length : length);
    } else if (shift && shape.type == TOOL_LINE) {
        if (std::abs(dx) > std::abs(dy) * 2) y = shape.y1;
        else if (std::abs(dy) > std::abs(dx) * 2) x = shape.x1;
        else {
            int length = (std::min)(std::abs(dx), std::abs(dy));
            x = shape.x1 + (dx < 0 ? -length : length);
            y = shape.y1 + (dy < 0 ? -length : length);
        }
    }
    shape.x2 = x; shape.y2 = y;
    if (shape.type == TOOL_PENCIL &&
        (shape.pts.empty() || shape.pts.back().x != x || shape.pts.back().y != y))
        shape.pts.push_back({x, y});
}
}

int main() {
    SetProcessDPIAware();
    Gdiplus::GdiplusStartupInput startup;
    ULONG_PTR token = 0;
    if (Gdiplus::GdiplusStartup(&token, &startup, nullptr) != Gdiplus::Ok) return 1;
    initgraph(WIN_W, WIN_H);
    SetWindowTextW(GetHWnd(), L"几何工坊  /  Geometry Studio");
    previousWindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(GetHWnd(), GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(WindowProc)));
    BeginBatchDraw();
    bool dragging = false;
    bool running = true;
    Shape current;
    Redraw();
    while (running) {
        if (closeRequested.exchange(false)) {
            dragging = false;
            ReleaseCapture();
            if (shapes.empty() || MessageBoxW(GetHWnd(),
                L"作品不会自动保存。请确认已导出需要保留的画布。\n\n确定退出几何工坊吗？",
                L"退出前确认", MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON2) == IDOK) {
                running = false;
                continue;
            }
            Redraw();
        }
        if (dragging && GetForegroundWindow() != GetHWnd()) {
            dragging = false;
            ReleaseCapture();
            Notify(L"已取消本次绘制");
            Redraw();
        }
        if (!ui.notice.empty() && GetTickCount64() >= noticeUntil) {
            ui.notice.clear(); Redraw(dragging ? &current : nullptr);
        }
        ExMessage m;
        if (!peekmessage(&m, EX_MOUSE | EX_KEY, true)) { Sleep(8); continue; }
        if (m.message == WM_MOUSEMOVE) {
            ui.mouseX = m.x; ui.mouseY = m.y;
            SetCursor(LoadCursor(nullptr, dragging || InCanvas(m.x, m.y) ? IDC_CROSS :
                HitTest(m.x, m.y).type != UI_NONE ? IDC_HAND : IDC_ARROW));
            if (dragging) UpdateShape(current, m.x, m.y, (GetKeyState(VK_SHIFT) & 0x8000) != 0);
            Redraw(dragging ? &current : nullptr);
        } else if (m.message == WM_LBUTTONDOWN && !dragging) {
            if (InCanvas(m.x, m.y)) {
                current = Shape{};
                current.type = ui.tool;
                current.stroke = ui.stroke; current.fill = ui.fill;
                current.filled = ui.filled; current.lineWidth = ui.lineWidth;
                current.x1 = current.x2 = m.x - CANVAS_X;
                current.y1 = current.y2 = m.y - CANVAS_Y;
                if (current.type == TOOL_PENCIL) current.pts.push_back({current.x1, current.y1});
                dragging = true;
                SetCapture(GetHWnd());
            } else { ApplyAction(HitTest(m.x, m.y)); Redraw(); }
        } else if (m.message == WM_LBUTTONUP && dragging) {
            UpdateShape(current, m.x, m.y, (GetKeyState(VK_SHIFT) & 0x8000) != 0);
            dragging = false; ReleaseCapture();
            const int width = std::abs(current.x2 - current.x1);
            const int height = std::abs(current.y2 - current.y1);
            bool valid = current.type == TOOL_PENCIL ? current.pts.size() > 1 :
                (current.type == TOOL_LINE || current.type == TOOL_CIRCLE) ? width + height >= 3 :
                width >= 3 && height >= 3;
            if (valid) {
                shapes.push_back(current);
                Change change; change.removed.push_back(current); Record(std::move(change));
            }
            Redraw();
        } else if (m.message == WM_RBUTTONDOWN || (m.message == WM_KEYDOWN && m.vkcode == VK_ESCAPE)) {
            if (dragging) { dragging = false; ReleaseCapture(); Notify(L"已取消本次绘制"); Redraw(); }
        } else if (m.message == WM_KEYDOWN && !dragging) {
            UIAction action;
            if (m.ctrl && m.vkcode == 'Z') action.type = m.shift ? UI_REDO : UI_UNDO;
            else if (m.ctrl && m.vkcode == 'Y') action.type = UI_REDO;
            else if (m.vkcode == 'S') action.type = UI_SAVE;
            else if (m.vkcode == VK_DELETE) action.type = UI_CLEAR;
            else if (m.vkcode == 'G') action.type = UI_TOGGLE_GRID;
            else if (m.vkcode == 'F') action.type = UI_TOGGLE_FILL;
            else if (!m.ctrl && m.vkcode >= '1' && m.vkcode <= '6') {
                action.type = UI_SELECT_TOOL; action.value = m.vkcode - '1';
            }
            ApplyAction(action); Redraw();
        }
    }
    EndBatchDraw();
    closegraph();
    Gdiplus::GdiplusShutdown(token);
    return 0;
}
