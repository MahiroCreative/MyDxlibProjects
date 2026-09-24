# UI driver for the click-through check (phase 9). Operates ONLY the test VSCode window
# identified by its user-data-dir, using real mouse/keyboard input.
# Coordinates are window-relative physical pixels, i.e. the same as in the screenshots.
# Comments are ASCII-only (PowerShell 5.1 misreads non-ASCII in BOM-less .ps1 files).
#
#   ui.ps1 -UserDataDir D -Action shot  -Out a.png
#   ui.ps1 -UserDataDir D -Action click -X 100 -Y 200 [-Out a.png]
#   ui.ps1 -UserDataDir D -Action dclick / rclick / hover -X .. -Y ..
#   ui.ps1 -UserDataDir D -Action keys  -Text "^%d"        (SendKeys syntax)
#   ui.ps1 -UserDataDir D -Action type  -Text "MyShader"   (literal text, via clipboard paste)
param(
	[Parameter(Mandatory = $true)][string]$UserDataDir,
	[Parameter(Mandatory = $true)][string]$Action,
	[int]$X = 0,
	[int]$Y = 0,
	[string]$Text = '',
	[string]$Out = '',
	[int]$WaitMs = 600
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class DxUi {
	[DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
	[DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
	[DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
	[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
	[DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
	[DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
	[DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, UIntPtr e);
	[DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint f, UIntPtr e);
	public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
}
"@
[DxUi]::SetProcessDPIAware() | Out-Null

function Find-Window {
	$cands = Get-CimInstance Win32_Process -Filter "Name='Code.exe'" | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($UserDataDir) }
	foreach ($c in $cands) {
		try {
			$p = Get-Process -Id $c.ProcessId -ErrorAction Stop
			if ($p.MainWindowHandle -ne [IntPtr]::Zero -and $p.MainWindowTitle -ne '') { return $p }
		} catch {}
	}
	throw "test window not found (UserDataDir=$UserDataDir)"
}

$proc = Find-Window
$h = $proc.MainWindowHandle
# Safety: refuse to act unless the test window really is in front (never click into the user's windows).
for ($i = 0; $i -lt 10; $i++) {
	if ([DxUi]::GetForegroundWindow() -eq $h) { break }
	[DxUi]::ShowWindow($h, 9) | Out-Null
	# ALT tap lets SetForegroundWindow succeed from a background process. Only when needed:
	# in VSCode an ALT tap moves keyboard focus to the menu bar, so later typing is lost.
	# ESC afterwards leaves the menu bar again.
	[DxUi]::keybd_event(0x12, 0, 0, [UIntPtr]::Zero); [DxUi]::keybd_event(0x12, 0, 2, [UIntPtr]::Zero)
	[DxUi]::SetForegroundWindow($h) | Out-Null
	Start-Sleep -Milliseconds 150
	[DxUi]::keybd_event(0x1B, 0, 0, [UIntPtr]::Zero); [DxUi]::keybd_event(0x1B, 0, 2, [UIntPtr]::Zero)
	Start-Sleep -Milliseconds 100
}
if ([DxUi]::GetForegroundWindow() -ne $h) { throw "could not bring the test window to front; aborting (no input sent)" }

$r = New-Object DxUi+RECT
[DxUi]::GetWindowRect($h, [ref]$r) | Out-Null
$sx = $r.Left + $X
$sy = $r.Top + $Y

switch ($Action) {
	'shot' { }
	'hover' { [DxUi]::SetCursorPos($sx, $sy) | Out-Null }
	'click' { [DxUi]::SetCursorPos($sx, $sy) | Out-Null; Start-Sleep -Milliseconds 80; [DxUi]::mouse_event(2, 0, 0, 0, [UIntPtr]::Zero); [DxUi]::mouse_event(4, 0, 0, 0, [UIntPtr]::Zero) }
	'dclick' { [DxUi]::SetCursorPos($sx, $sy) | Out-Null; Start-Sleep -Milliseconds 80; for ($k = 0; $k -lt 2; $k++) { [DxUi]::mouse_event(2, 0, 0, 0, [UIntPtr]::Zero); [DxUi]::mouse_event(4, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60 } }
	'rclick' { [DxUi]::SetCursorPos($sx, $sy) | Out-Null; Start-Sleep -Milliseconds 80; [DxUi]::mouse_event(8, 0, 0, 0, [UIntPtr]::Zero); [DxUi]::mouse_event(16, 0, 0, 0, [UIntPtr]::Zero) }
	'keys' { [System.Windows.Forms.SendKeys]::SendWait($Text) }
	'type' { [System.Windows.Forms.Clipboard]::SetText($Text); [System.Windows.Forms.SendKeys]::SendWait('^v') }
	default { throw "unknown action $Action" }
}

if ($Out -ne '') {
	Start-Sleep -Milliseconds $WaitMs
	[DxUi]::GetWindowRect($h, [ref]$r) | Out-Null
	$w = $r.Right - $r.Left; $hh = $r.Bottom - $r.Top
	$bmp = New-Object System.Drawing.Bitmap $w, $hh
	$g = [System.Drawing.Graphics]::FromImage($bmp)
	$g.CopyFromScreen($r.Left, $r.Top, 0, 0, $bmp.Size)
	$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
	$g.Dispose(); $bmp.Dispose()
	Write-Output "shot $Out ($w x $hh)"
}
Write-Output "ok $Action"
