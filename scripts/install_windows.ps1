param(
    [string]$BuildType = "Release",
    [string]$InstallDir = ""
)

$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptRoot "..")
if (-not $InstallDir) {
    $InstallDir = Join-Path $repoRoot "dist/windows"
}
$buildDir = Join-Path $repoRoot ("build-" + $BuildType.ToLower())

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

cmake -S $repoRoot -B $buildDir -DCMAKE_BUILD_TYPE=$BuildType | Out-Null
cmake --build $buildDir --config $BuildType | Out-Null

$exeName = "achim_alarm.exe"
$binaryCandidates = @(
    Join-Path $buildDir $exeName,
    Join-Path $buildDir (Join-Path $BuildType $exeName)
)
$binaryPath = $null
foreach ($candidate in $binaryCandidates) {
    if (Test-Path $candidate) {
        $binaryPath = $candidate
        break
    }
}

if (-not $binaryPath) {
    throw "Could not locate built executable. Expected one of: $($binaryCandidates -join ', ')"
}

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
Copy-Item $binaryPath (Join-Path $InstallDir $exeName) -Force

$trayIconSource = Join-Path $repoRoot "tray/logo.png"
if (Test-Path $trayIconSource) {
    Copy-Item $trayIconSource (Join-Path $InstallDir "tray_icon.png") -Force
}

$deployTool = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($deployTool) {
    & $deployTool.Source --multimedia --compiler-runtime --dir $InstallDir $binaryPath
} else {
    Write-Warning "windeployqt not found in PATH. Install the Qt deployment tools to bundle Qt runtime libraries."
}

Write-Host "Achim Alarm binaries staged under $InstallDir"
