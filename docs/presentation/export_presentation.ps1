# Smart Banking Management System - Presentation PDF Exporter
$dir = $PSScriptRoot
$htmlPath = "$dir\presentation.html"
$pdfPath  = "$dir\smart_banking_system_presentation.pdf"

if (!(Test-Path $htmlPath)) {
    Write-Error "Could not find presentation.html at $htmlPath"
    exit 1
}

$browser = if (Test-Path "C:\Program Files\Google\Chrome\Application\chrome.exe") {
    "C:\Program Files\Google\Chrome\Application\chrome.exe"
} elseif (Test-Path "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe") {
    "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
} elseif (Test-Path "C:\Program Files\Microsoft\Edge\Application\msedge.exe") {
    "C:\Program Files\Microsoft\Edge\Application\msedge.exe"
} else {
    Write-Error "Neither Google Chrome nor Microsoft Edge was found."
    exit 1
}

Write-Output "Found browser: $browser"
Write-Output "Converting $htmlPath to 16:9 PDF..."

$fileUri = "file:///" + ($htmlPath -replace '\\', '/')

$args = @(
    "--headless",
    "--disable-gpu",
    "--no-pdf-header-footer",
    "--print-to-pdf=$pdfPath",
    $fileUri
)

$proc = Start-Process -FilePath $browser -ArgumentList $args -PassThru -Wait

if (Test-Path $pdfPath) {
    $bytes = (Get-Item $pdfPath).Length
    $kb = [math]::Round($bytes / 1KB, 2)
    Write-Output "[SUCCESS] Exported presentation PDF ($kb KB): $pdfPath"
} else {
    Write-Error "[FAILURE] Failed to generate PDF export."
    exit 1
}