# Only works on windows PowerShell 2

#Install-Package 7Zip4Powershell
$output_dir = (Get-Item -Path ".\").FullName

$ffmpeg_version = "4.1.3"
$sdl_version = "2.0.9"

$ffmpeg_win32_static = "ffmpeg-$ffmpeg_version-win32-dev"
$ffmpeg_win64_static = "ffmpeg-$ffmpeg_version-win64-dev"
$ffmpeg_win32_shared = "ffmpeg-$ffmpeg_version-win32-shared"
$ffmpeg_win64_shared = "ffmpeg-$ffmpeg_version-win64-shared"

$ffmpeg_win32_static_url = "https://ffmpeg.zeranoe.com/builds/win32/dev/" + $ffmpeg_win32_static + ".zip"
$ffmpeg_win64_static_url = "https://ffmpeg.zeranoe.com/builds/win64/dev/" + $ffmpeg_win64_static + ".zip"
$ffmpeg_win32_shared_url = "https://ffmpeg.zeranoe.com/builds/win32/shared/" + $ffmpeg_win32_shared + ".zip"
$ffmpeg_win64_shared_url = "https://ffmpeg.zeranoe.com/builds/win64/shared/" + $ffmpeg_win64_shared + ".zip"

$sdl_dev_dir = "SDL2-devel-$sdl_version-VC"
$sdl_dev_url = "https://www.libsdl.org/release/" + $sdl_dev_dir + ".zip"

$libyuv_url = "https://chromium.googlesource.com/libyuv/libyuv/+archive/refs/heads/master.tar.gz"

$downloads = @{}
$downloads.add($ffmpeg_win32_static_url, "$output_dir/$ffmpeg_win32_static.zip")
$downloads.add($ffmpeg_win64_static_url, "$output_dir/$ffmpeg_win64_static.zip")
$downloads.add($ffmpeg_win32_shared_url, "$output_dir/$ffmpeg_win32_shared.zip")
$downloads.add($ffmpeg_win64_shared_url, "$output_dir/$ffmpeg_win64_shared.zip")
$downloads.add($sdl_dev_url, "$output_dir/$sdl_dev_dir.zip")
$downloads.add($libyuv_url, "$output_dir/libyuv.tar.gz")

$client = new-object System.Net.WebClient

Write-Output "Starting to download ffmpeg libraries ..."
$downloads.GetEnumerator() | ForEach-Object{
	# $message = '{0} {1}' -f $_.key, $_.value
	# Write-Output $message
	$client.DownloadFile($_.key, $_.value)
}

Write-Output "Starting to uncompress the files ..."
$downloads.GetEnumerator() | ForEach-Object{
	Expand-Archive -path $_.value -destinationpath $output_dir
}
###############################################################################

Function Create-LibraryDirs {
	Param([String]$dir)
	New-Item -ItemType "directory" -Path "$output_dir/include/$dir" -Force
	New-Item -ItemType "directory" -Path "$output_dir/lib/$dir/win32" -Force
	New-Item -ItemType "directory" -Path "$output_dir/lib/$dir/win64" -Force
}

New-Item -ItemType "directory" -Path "$output_dir/bin/win32" -Force
New-Item -ItemType "directory" -Path "$output_dir/bin/win64" -Force

Create-LibraryDirs "sdl2"
Create-LibraryDirs "ffmpeg"

###############################################################################
Write-Output "Starting to copy needed files to specified location ...."
$win32_dll_dir = "$output_dir/bin/win32"
$win64_dll_dir = "$output_dir/bin/win64"

$ffmpeg_include_dir = "$output_dir/include/ffmpeg"
$ffmpeg_win32_lib_dir = "$output_dir/lib/ffmpeg/win32"
$ffmpeg_win64_lib_dir = "$output_dir/lib/ffmpeg/win64"

Copy-Item "$output_dir/$ffmpeg_win64_static/include/*" -Destination "$ffmpeg_include_dir" -Recurse -Force
Copy-Item "$output_dir/$ffmpeg_win32_static/lib/*" -Destination "$ffmpeg_win32_lib_dir" -Force
Copy-Item "$output_dir/$ffmpeg_win64_static/lib/*" -Destination "$ffmpeg_win64_lib_dir" -Force
Copy-Item "$output_dir/$ffmpeg_win32_shared/bin/*" -Destination "$win32_dll_dir" -Force
Copy-Item "$output_dir/$ffmpeg_win64_shared/bin/*" -Destination "$win64_dll_dir" -Force

$sdl_include_dir = "$output_dir/include/sdl2"
$sdl_win32_lib_dir = "$output_dir/lib/sdl2/win32"
$sdl_win64_lib_dir = "$output_dir/lib/sdl2/win64"
$sdl_uncompressed_dir = "SDL2-$sdl_version"
Copy-Item "$output_dir/$sdl_uncompressed_dir/include/*" -Destination "$sdl_include_dir" -Force
Copy-Item "$output_dir/$sdl_uncompressed_dir/lib/x86/*" -Destination "$sdl_win32_lib_dir" -Force
Copy-Item "$output_dir/$sdl_uncompressed_dir/lib/x64/*" -Destination "$sdl_win64_lib_dir" -Force
Copy-Item "$output_dir/$sdl_uncompressed_dir/lib/x86/SDL2.dll" -Destination "$win32_dll_dir" -Force
Copy-Item "$output_dir/$sdl_uncompressed_dir/lib/x64/SDL2.dll" -Destination "$win64_dll_dir" -Force

###############################################################################
Write-Output "Deleting unnecessary files and directories ..."
$uncompressed_dirs = @(
	"$output_dir/$ffmpeg_win32_static",
	"$output_dir/$ffmpeg_win64_static",
	"$output_dir/$ffmpeg_win32_shared",
	"$output_dir/$ffmpeg_win64_shared",
	"$output_dir/$sdl_uncompressed_dir"
)

Remove-Item -Path $output_dir/* -Include *.zip
foreach($dir in $uncompressed_dirs) {
	Remove-Item -Path $dir -Recurse
}