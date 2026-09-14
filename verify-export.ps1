Add-Type @'
using System;
using System.Runtime.InteropServices;
public class ExportCheck {
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern IntPtr FindWindow(string cls,string title);
 public delegate bool EnumProc(IntPtr h,IntPtr l);
 [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr h,EnumProc p,IntPtr l);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h,System.Text.StringBuilder s,int n);
 public static IntPtr Edit(IntPtr h) { IntPtr found=IntPtr.Zero; EnumChildWindows(h,(c,l)=>{var s=new System.Text.StringBuilder(256); GetClassName(c,s,256); if(s.ToString()=="Edit") found=c; return true;},IntPtr.Zero); return found; }
 [DllImport("user32.dll")] public static extern IntPtr GetDlgItem(IntPtr h,int id);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern IntPtr SendMessage(IntPtr h,uint m,IntPtr w,string l);
 [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h,uint m,IntPtr w,IntPtr l);
}
'@
$h=[ExportCheck]::FindWindow('#32770',"导出画布 · 几何工坊")
if($h -eq 0){throw 'Save dialog not found'}
$edit=[ExportCheck]::Edit($h)
if($edit -eq 0){throw 'Filename edit not found'}
$path=Join-Path $PSScriptRoot ('build\verified-export-'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'.png')
[ExportCheck]::SendMessage($edit,12,[IntPtr]::Zero,$path) | Out-Null
[ExportCheck]::PostMessage($h,273,[IntPtr]1,[IntPtr]::Zero) | Out-Null
Start-Sleep -Seconds 2
if(!(Test-Path $path)){throw 'PNG not created'}
Add-Type -AssemblyName System.Drawing
$im=[System.Drawing.Bitmap]::FromFile($path)
Write-Output "Verified PNG $($im.Width) x $($im.Height): $path"
$im.Dispose()
