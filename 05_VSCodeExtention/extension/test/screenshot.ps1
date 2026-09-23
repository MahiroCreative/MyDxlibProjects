# Screenshot helper for the test VSCode window, identified by its UserDataDir.
# Uses only built-in .NET (System.Drawing) — no extra tools required.
# Comments are kept ASCII-only because PowerShell 5.1 misparses non-ASCII
# comments in a BOM-less UTF-8 .ps1 file (it assumes the system codepage).
param(
	[Parameter(Mandatory = $true)][string]$UserDataDir,
	[Parameter(Mandatory = $true)][string]$OutPath
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class DxLibWin32 {
	[DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
	[DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
	[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
	[DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
	public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
}
"@

# Without DPI awareness, GetWindowRect returns virtualized (scaled) coordinates on
# high-DPI displays (e.g. 150%), so the capture is offset and includes other windows.
[DxLibWin32]::SetProcessDPIAware() | Out-Null

$candidates = Get-CimInstance Win32_Process -Filter "Name='Code.exe'" | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($UserDataDir) }
if (-not $candidates) { throw "No matching Code.exe process found (UserDataDir=$UserDataDir)" }

$target = $null
foreach ($c in $candidates) {
	try {
		$p = Get-Process -Id $c.ProcessId -ErrorAction Stop
		if ($p.MainWindowHandle -ne [IntPtr]::Zero -and $p.MainWindowTitle -ne '') {
			$target = $p
			break
		}
	} catch {}
}
if (-not $target) { throw "No process with a visible window was found" }

$hwnd = $target.MainWindowHandle
[DxLibWin32]::ShowWindow($hwnd, 9) | Out-Null   # SW_RESTORE
[DxLibWin32]::SetForegroundWindow($hwnd) | Out-Null
Start-Sleep -Milliseconds 400

$rect = New-Object DxLibWin32+RECT
[DxLibWin32]::GetWindowRect($hwnd, [ref]$rect) | Out-Null
$w = $rect.Right - $rect.Left
$h = $rect.Bottom - $rect.Top
if ($w -le 0 -or $h -le 0) { throw "Could not read window size" }

$bmp = New-Object System.Drawing.Bitmap $w, $h
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bmp.Size)
New-Item -ItemType Directory -Force -Path (Split-Path $OutPath) | Out-Null
$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose()
$bmp.Dispose()
Write-Output "saved: $OutPath ($w x $h) title=$($target.MainWindowTitle)"
