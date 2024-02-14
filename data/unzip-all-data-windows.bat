@echo off
setlocal EnableDelayedExpansion

set SEARCHDIR=%~dp0
set SUBDIR=data
set DATADIR=
pushd "%SEARCHDIR%"

:try_next_dir
set CURRENT=%CD%
if exist "%SUBDIR%" (set DATADIR=%CURRENT%\%SUBDIR%) && (goto exit_data_dir_search)
cd ..
if "%CD%" == "%CURRENT%" goto exit_data_dir_search
goto try_next_dir

:exit_data_dir_search
popd
if not "%DATADIR%" == "" goto unzip

echo No data subdirectory %SUBDIR% found along path from %SEARCHDIR% to %CURRENT%. >&2
goto :EOF

:unzip
set RANDOMKEY=%RANDOM%-%RANDOM%-%RANDOM%-%RANDOM%
set SERIAL=0

:try_next_temp_name
set TEMPNAME=%TEMP%\julia-unzip-%RANDOMKEY%-%SERIAL%.tmp
if exist "%TEMPNAME%" (set /A SERIAL=%SERIAL% + 1) && (goto try_next_temp_name)

set GATE=closed
for /F "tokens=1,*" %%x in (%0) do if "%%x" == "endlocal" (set GATE=open) else (if "!GATE!" == "open" (echo %%x %%y)>>%TEMPNAME%)
goto run_vbs

set LINENO=0
for /F "tokens=*" %%x in (%TEMPNAME%) do (set /A LINENO=!LINENO! + 1) && (echo !LINENO! %%x)

:run_vbs
pushd "%DATADIR%"
cscript //Nologo /e:vbscript %TEMPNAME%
popd

erase %TEMPNAME%

endlocal && goto :EOF

Set fs = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("Shell.Application")

Set stderr = fs.GetStandardStream(2)
datadir = fs.GetAbsolutePathName(".")

Sub UnzipFile(zipfile, directory)
	Dim sources
	Dim source
	Dim destination
	Dim disqualified
	
	disqualified = False
	Set sources = shell.NameSpace(zipfile.Path).Items()
	For Each source In sources
		If fs.FileExists(fs.BuildPath(directory, source.Name)) Then
			disqualified = True
		End If
	Next
	
	If disqualified Then
		stderr.WriteLine("[Warning] Skipping " & zipfile.Name & " because contents already exist in data directory...")
	Else
		stderr.WriteLine("[Info]    Unzipping " & zipfile.Name & " to data directory...")
		Set destination = shell.NameSpace(directory)
		Call destination.CopyHere(sources, 1024 + 4)
	End If
End Sub

stderr.WriteLine("[Info]    Found data directory at " & datadir)

For Each zipfile In fs.GetFolder(datadir).Files
	If LCase(fs.GetExtensionName(zipfile.Name)) = "zip" Then
		Call UnzipFile(zipfile, datadir)
	End If
Next
