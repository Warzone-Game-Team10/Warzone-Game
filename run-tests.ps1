$ErrorActionPreference = "Stop"

Write-Host "`n=== COMP 345 WARZONE TEST SUITE ===" -ForegroundColor Cyan

# Compile main project
g++ -std=c++20 -Iinclude src/*.cpp -o warzone-tests.exe
if ($LASTEXITCODE -ne 0) { throw "Main compilation failed" }

# Run regression tests
$tests = @("issue28", "orders", "player", "cards", "startup", "gameplay")
$failed = @()

foreach ($test in $tests) {
    Write-Host "`n=== Running $test ===" -ForegroundColor Cyan
    & .\warzone-tests.exe $test

    if ($LASTEXITCODE -ne 0) {
        $failed += $test
    }
}

# Compile standalone integration test
$sourceFiles = Get-ChildItem src -Filter "*.cpp" |
    Where-Object { $_.Name -ne "MainDriver.cpp" } |
    ForEach-Object { $_.FullName }

g++ -std=c++20 -Iinclude tests/CardIntegrationSmokeTest.cpp $sourceFiles -o card-integration-test.exe
if ($LASTEXITCODE -ne 0) { throw "Integration compilation failed" }

# Run integration test
Write-Host "`n=== Running Card Integration ===" -ForegroundColor Cyan
& .\card-integration-test.exe

if ($LASTEXITCODE -ne 0) {
    $failed += "card-integration"
}

if ($failed.Count -gt 0) {
    Write-Host "FAILED: $($failed -join ', ')" -ForegroundColor Red
    exit 1
}

Write-Host "`nALL SELECTED WARZONE TESTS PASSED!" -ForegroundColor Green


