#
# generate, compile and run exe files
#
function getExePathFromCMakeLists() {
    $content = Get-Content -Raw -Path "./CMakeLists.txt"
    $exePath = ""
    foreach ($line in $content -split "`n") {
        if ($line -match 'set\(MY_EXECUTABLE_NAME[^\"]*\"([^\"]+)\"') {
            $exeName = $matches[1]
            $exePath = "./build/bin/Debug/$exeName" + ".exe"
            break
        }
    }
    return $exePath
}

$currentDirectory = Get-Location
$cmakeListsPath = Join-Path -Path $currentDirectory -ChildPath "CMakeLists.txt"

if (-not (Test-Path $cmakeListsPath)) {
    Write-Host("No CMakeLists.txt in current directory, please check.")
    return
}

Write-Host "Start generating and compiling..."

$buildFolderPath = ".\build"

if (-not (Test-Path $buildFolderPath)) {
    New-Item -ItemType Directory -Path $buildFolderPath | Out-Null
    Write-Host "build folder created."
}

# cmake -G -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Users/SonnyCalcr/scoop/apps/vcpkg/current/scripts/buildsystems/vcpkg.cmake -S . -B ./build/
cmake --preset=default

if ($LASTEXITCODE -eq 0) {
    cmake --build build
    if ($LASTEXITCODE -eq 0) {
        $exePath = getExePathFromCMakeLists
        Write-Host "start running as follows..."
        Write-Host "=================================================="
        Invoke-Expression $exePath
    }
}
