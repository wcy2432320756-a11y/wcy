param([switch]$LaunchOnly)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class DesktopTest {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h,IntPtr after,int x,int y,int w,int z,uint flags);
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X,Y; }
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr h,int x,int y,int w,int z,bool repaint);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h,out RECT r);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h,ref POINT p);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x,int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f,uint x,uint y,uint d,UIntPtr extra);
    [DllImport("user32.dll")] public static extern void keybd_event(byte v,byte scan,uint f,UIntPtr extra);
}
'@
[DesktopTest]::SetProcessDPIAware() | Out-Null
$p = Start-Process -FilePath "$PSScriptRoot\build\geodraw.exe" -WorkingDirectory $PSScriptRoot -PassThru
Start-Sleep -Seconds 2
$p.Refresh()
$h=$p.MainWindowHandle
if ($h -eq 0) { throw 'No application window' }
[DesktopTest]::SetWindowPos($h,[IntPtr](-1),80,50,0,0,1) | Out-Null
[DesktopTest]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 500
$origin=New-Object DesktopTest+POINT
[DesktopTest]::ClientToScreen($h,[ref]$origin) | Out-Null
function MoveTo($x,$y) { [DesktopTest]::SetCursorPos($origin.X+$x,$origin.Y+$y) | Out-Null; Start-Sleep -Milliseconds 90 }
function Click($x,$y) { MoveTo $x $y; [DesktopTest]::mouse_event(2,0,0,0,[UIntPtr]::Zero); [DesktopTest]::mouse_event(4,0,0,0,[UIntPtr]::Zero); Start-Sleep -Milliseconds 140 }
function Key($key,$ctrl=$false) {
    if($ctrl) { [DesktopTest]::keybd_event(17,0,0,[UIntPtr]::Zero) }
    [DesktopTest]::keybd_event($key,0,0,[UIntPtr]::Zero)
    [DesktopTest]::keybd_event($key,0,2,[UIntPtr]::Zero)
    if($ctrl) { [DesktopTest]::keybd_event(17,0,2,[UIntPtr]::Zero) }
    Start-Sleep -Milliseconds 200
}
function Drag($x,$y,$tx,$ty) {
    MoveTo $x $y
    [DesktopTest]::mouse_event(2,0,0,0,[UIntPtr]::Zero)
    for($i=1;$i -le 12;$i++) { MoveTo ([int]($x+($tx-$x)*$i/12)) ([int]($y+($ty-$y)*$i/12)) }
    [DesktopTest]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
    Start-Sleep -Milliseconds 250
}
function Screenshot($name) {
    $r=New-Object DesktopTest+RECT
    [DesktopTest]::GetClientRect($h,[ref]$r) | Out-Null
    $bitmap=New-Object System.Drawing.Bitmap ($r.R),($r.B)
    $g=[System.Drawing.Graphics]::FromImage($bitmap)
    $g.CopyFromScreen($origin.X,$origin.Y,0,0,$bitmap.Size)
    $bitmap.Save("$PSScriptRoot\build\$name.png")
    $g.Dispose(); $bitmap.Dispose()
}
Screenshot 'ui-empty'
if(-not $LaunchOnly) {
    Click 85 229
    Drag 288 232 498 392
    Click 85 345
    Drag 691 325 780 325
    Click 85 403
    Drag 480 440 675 615
    Click 85 171
    Drag 300 580 435 465
    Click 719 39
    Click 807 39
    Click 1180 559
    MoveTo 183 744
    Start-Sleep -Seconds 5
    Screenshot 'ui-smoke'
    Click 85 287
    Drag 840 670 730 590
    Click 85 461
    Drag 300 660 420 690
    Click 906 39
    Screenshot 'ui-cleared'
    Click 719 39
    Start-Sleep -Seconds 5
    Screenshot 'ui-restored'
    Click 1138 39
    Start-Sleep -Seconds 1
    Add-Type -AssemblyName System.Windows.Forms
    $export=Join-Path $PSScriptRoot ('build\export-test-'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'.png')
    [System.Windows.Forms.SendKeys]::SendWait('%n')
    [System.Windows.Forms.SendKeys]::SendWait($export)
    [System.Windows.Forms.SendKeys]::SendWait('{ENTER}')
    Start-Sleep -Seconds 2
    if(Test-Path $export) {
        $im=[System.Drawing.Bitmap]::FromFile($export)
        Write-Output "PNG export verified: $($im.Width)x$($im.Height); $export"
        $im.Dispose()
    } else { Write-Output 'PNG export automation did not complete; manual verification needed' }
}
[DesktopTest]::SetWindowPos($h,[IntPtr](-2),0,0,0,0,3) | Out-Null
Write-Output "Window=$h Process=$($p.Id); screenshots saved in build"
