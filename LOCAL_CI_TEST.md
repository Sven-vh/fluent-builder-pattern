# Local CI Test Script

This PowerShell script simulates the GitHub Actions CI workflow for local testing.

**Note**: This requires Windows with Visual Studio Build Tools or Visual Studio installed.

```powershell
# Test script to simulate CI workflow locally
Write-Host "=== Local CI Test ==="

# Check prerequisites
$nuget = Get-Command nuget -ErrorAction SilentlyContinue
$msbuild = Get-Command msbuild -ErrorAction SilentlyContinue

if (-not $nuget) {
    Write-Error "NuGet CLI not found. Please install NuGet CLI."
    exit 1
}

if (-not $msbuild) {
    Write-Error "MSBuild not found. Please install Visual Studio or Build Tools."
    exit 1
}

try {
    # Step 1: Restore packages
    Write-Host "1. Restoring NuGet packages..."
    nuget restore FluentBuilderPattern.sln
    if ($LASTEXITCODE -ne 0) { throw "NuGet restore failed" }

    # Step 2: Build solution
    Write-Host "2. Building solution..."
    msbuild FluentBuilderPattern.sln /p:Configuration=Release /p:Platform=x64 /p:WarningLevel=4 /verbosity:minimal
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }

    # Step 3: Check if test executable exists
    $testExe = ".\UnitTests\out\Release-x64\UnitTests.exe"
    if (-not (Test-Path $testExe)) {
        throw "Test executable not found at $testExe"
    }
    Write-Host "3. Test executable found: $testExe"

    # Step 4: Run tests
    Write-Host "4. Running tests..."
    & $testExe --gtest_output=xml:test_results_local.xml
    if ($LASTEXITCODE -ne 0) { throw "Tests failed" }

    Write-Host "✅ All steps passed! Your changes should pass CI."
}
catch {
    Write-Host "❌ Local CI test failed: $_"
    exit 1
}
```

## Usage

1. Save this as `test-ci-locally.ps1`
2. Run in PowerShell: `.\test-ci-locally.ps1`
3. If successful, your changes should pass the GitHub Actions CI

## Differences from GitHub Actions

- Uses your local Visual Studio/Build Tools version instead of GitHub's
- May have different dependency versions
- Local environment variables and paths may differ

For the most accurate test, push to a branch and create a PR to see the actual CI results.