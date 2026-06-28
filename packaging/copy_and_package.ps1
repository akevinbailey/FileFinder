# Copy an application and its runtime files into a staging directory, then
# create a ZIP archive containing the staged files.
# Usage:
#   .\copy_and_package.ps1 <source_dir> <dest_dir> <executable_name> [extra_paths...]
# Optional:
#   -ArchivePath <archive_path>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string] $SourceDir,

    [Parameter(Mandatory = $true, Position = 1)]
    [string] $DestDir,

    [Parameter(Mandatory = $true, Position = 2)]
    [string] $ExecutableName,

    [Parameter(Position = 3, ValueFromRemainingArguments = $true)]
    [string[]] $ExtraPaths = @(),

    [string] $ArchivePath
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Get-AbsolutePath {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Path
    )

    if ([string]::IsNullOrWhiteSpace($Path)) {
        throw "Path must not be empty."
    }

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    return [System.IO.Path]::GetFullPath(
        (Join-Path -Path (Get-Location).ProviderPath -ChildPath $Path)
    )
}

function Test-IsSameOrParentPath {
    param(
        [Parameter(Mandatory = $true)]
        [string] $ParentPath,

        [Parameter(Mandatory = $true)]
        [string] $ChildPath
    )

    $comparison = [System.StringComparison]::OrdinalIgnoreCase
    $parentWithSeparator = $ParentPath.TrimEnd("\", "/") + [System.IO.Path]::DirectorySeparatorChar

    return $ChildPath.Equals($ParentPath, $comparison) -or
        $ChildPath.StartsWith($parentWithSeparator, $comparison)
}

function Assert-RelativeSourcePath {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Path
    )

    if ([string]::IsNullOrWhiteSpace($Path) -or [System.IO.Path]::IsPathRooted($Path)) {
        throw "Source path must be relative to source_dir: '$Path'"
    }

    $segments = $Path -split "[\\/]"
    if ($segments -contains "..") {
        throw "Source path must not contain '..': '$Path'"
    }
}

$sourcePath = Get-AbsolutePath -Path $SourceDir
$destinationPath = Get-AbsolutePath -Path $DestDir

if (-not (Test-Path -LiteralPath $sourcePath -PathType Container)) {
    throw "source_dir not found: '$sourcePath'"
}

$destinationRoot = [System.IO.Path]::GetPathRoot($destinationPath)
if ($destinationPath.Equals($destinationRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to use a filesystem root as dest_dir: '$destinationPath'"
}

if (Test-IsSameOrParentPath -ParentPath $destinationPath -ChildPath $sourcePath) {
    throw "Refusing to remove dest_dir because it contains source_dir: '$destinationPath'"
}

if ([string]::IsNullOrWhiteSpace($ArchivePath)) {
    $archiveFileName = [System.IO.Path]::GetFileNameWithoutExtension($ExecutableName) + ".zip"
    $archivePath = Join-Path -Path $sourcePath -ChildPath $archiveFileName
}
else {
    $archivePath = Get-AbsolutePath -Path $ArchivePath
}

if (Test-IsSameOrParentPath -ParentPath $destinationPath -ChildPath $archivePath) {
    throw "Archive path must not be inside dest_dir: '$archivePath'"
}

$archiveParent = Split-Path -Parent $archivePath
if (-not (Test-Path -LiteralPath $archiveParent -PathType Container)) {
    throw "Archive parent directory not found: '$archiveParent'"
}

$pathsToCopy = @($ExecutableName) + $ExtraPaths
$destinationItems = @()
$copiedNames = @{}

foreach ($relativePath in $pathsToCopy) {
    Assert-RelativeSourcePath -Path $relativePath

    $sourceItem = Join-Path -Path $sourcePath -ChildPath $relativePath
    if (-not (Test-Path -LiteralPath $sourceItem)) {
        throw "Required path not found: '$sourceItem'"
    }

    $destinationName = Split-Path -Leaf $sourceItem
    if ($copiedNames.ContainsKey($destinationName)) {
        throw "Multiple source paths would be copied as '$destinationName'."
    }

    $copiedNames[$destinationName] = $true
    $destinationItems += Join-Path -Path $destinationPath -ChildPath $destinationName
}

if (Test-Path -LiteralPath $destinationPath) {
    Remove-Item -LiteralPath $destinationPath -Recurse -Force
}
New-Item -ItemType Directory -Path $destinationPath | Out-Null

foreach ($relativePath in $pathsToCopy) {
    $sourceItem = Join-Path -Path $sourcePath -ChildPath $relativePath
    Copy-Item -LiteralPath $sourceItem -Destination $destinationPath -Recurse -Force
}

Compress-Archive -LiteralPath $destinationItems -DestinationPath $archivePath -CompressionLevel Optimal -Force

Write-Host "OK: Packed files into $archivePath"
