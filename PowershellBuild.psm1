class CompileTarget {
    [string]$input_file
    [string]$output_file
    CompileTarget ($input_file, $output_file) { $this.input_file = $input_file; $this.output_file = $output_file }
}

enum Build {
    STATIC_LIB
    DYNAMIC_LIB
    EXECUTABLE
}

enum BuildVersion {
    NONE                = 0
    DEBUG               = 1
    RELEASE             = 2
    RELEASE_WITH_DEBUG  = 3
}

New-Module 'PowershellBuild' {
    function IsNull($objectToCheck) {
        if ($null -eq $objectToCheck) {
            return $true
        }
    
        if ($objectToCheck -is [String] -and $objectToCheck -eq [String]::Empty) {
            return $true
        }
    
        if ($objectToCheck -is [DBNull] -or $objectToCheck -is [System.Management.Automation.Language.NullString]) {
            return $true
        }
    
        return $false
    }

    function GetCompileCommand([string] $location, [string]$file, [string] $Compiler) {
        $t = "    "
        # if powershell wasn't a stupid piece of shit that added windows line ending i could easily split this into mutliple lines 
        $command = "`n$t{`n$t$t`"directory`": `"$location`",`n$t$t`"command`": `"$Compiler $file`",`n$t$t`"file`": `"$location/$file`"`n$t}"
        return $command
    }

    function CreateCompileCommands( [string[]] $Files, [string] $Compiler ) {
        $loc = Get-Location
        $loc = $loc -replace "\\", '/'
        $filedata = "["
        $num = 0;
        foreach ( $file in $Files ) {
            $compilecommand = GetCompileCommand -Location $loc -File $file -Compiler $Compiler
            $filedata = "$filedata$compilecommand"
            $test = ($Files.Count - 1) - $num
            if ($test) {
                $filedata = "$filedata,"
            }
            $num = $num + 1;
        }

        $tmp = $filedata += "`n]"
        $tmp = $filedata.Replace("`n`r", "`n") 
        $tmp = $filedata.Replace("`r`n", "`n") 
        $tmp = New-Item ./compile_commands.json -ItemType File 
        $tmp = Set-Content ./compile_commands.json -NoNewline $filedata
    }

    function BuildFiles([string]$Compiler, [CompileTarget[]]  $Files, [BuildVersion] $version) {
        $commands = @()
        $ExecutableLinker = ""
        switch ( $version ) {
            DEBUG              {  
                $debugging_flag = "-g "
                if ($PSVersionTable.Platform -eq "Win32NT") {
                    $debugging_flag = "-g "
                }

                $files.ForEach({ $file = $_
                    $input_file = $file.input_file
                    $output_file = $file.output_file
                    $command = "$Compiler $debugging_flag -Wall -c $input_file -o $output_file"
                    $commands += $command
                })
                $ExecutableLinker = "$Compiler $debugging_flag -Wall"
                break
            }
            RELEASE            {  
                $files.ForEach({ $file = $_
                    $input_file = $file.input_file
                    $output_file = $file.output_file
                    $command   = "$Compiler -O2 -Ofast -c $input_file -o $output_file"
                    $commands += $command
                })
                $ExecutableLinker = "$Compiler -O2 -Ofast"

            }
            RELEASE_WITH_DEBUG {
                $files.ForEach({ $file = $_
                    $input_file = $file.input_file
                    $output_file = $file.output_file
                    $command = "$Compiler $debugging_flag -O2 -Ofast -c $input_file -o $output_file"
                    $commands += $command 
                })
                $ExecutableLinker = "$Compiler $debugging_flag -O2 -Ofast"
                break
            }
        }

        $commands | ForEach-Object -Parallel { 
            Invoke-Expression $_ 
        }

        return $ExecutableLinker
    }


    function Build-Powershell(
        [string] $BuildDirectory  = "",
        [string] $OutputDirectory = "bin", 
        [string] $IntermediateDirectory = "bin/int", 

        [Build] $Build = [Build]::STATIC_LIB,
        [string] $OutputName,

        [BuildVersion] $BuildVersion = [BuildVersion]::DEBUG,

        [switch]   $CreateCompileCommandsFile,

    # these are files that are going to be inserted 
    # into CompileCommands
        [string[]] $ExtraCommandFiles,


    # if these three are null
    # we will automatically assing variable
    # according to platform
    # compiler: clang
    # Linker  : clang
    # (WINDOWS) Archiver: llvm-lib
    # (LINUX)   Archiver: ar
    # linker is used for executables
    # archiver is for libaries
        [string] $Compiler = $null,
        [string] $Linker   = $null,
        [string] $Archiver = $null,


        [string[]] $ExtraArgs,
        [string[]] $IncludeDirectories,
        [string[]] $LibraryDirectories,
        [string[]] $Libraries,

    # Path To All project files
    # The Path Is a Local Path from $BuildDirectory
        [string[]] $Files,


    # this is the name of the final executable
    # when $Executable is set
        [switch]   $MsvcBasedCompiler
    ) {
        if ("" -eq $BuildDirectory) { 
            $BuildDirectory = Get-Location
        }

        $current_location = Get-Location
        Set-Location $BuildDirectory
        $ExecutableFileType = $null;

        switch ($PSVersionTable.Platform) {
            "Win32NT" { 
                $LibType = $null
                $IsDll = "" 
                if ($Build -eq [Build]::DYNAMIC_LIB)
                     { $LibType = "dll"; $IsDll = "/DLL"}
                else { $LibType = "lib" }

                if ("" -eq $Compiler ) { $Compiler = "clang -D_CRT_SECURE_NO_WARNINGS" }
                if ("" -eq $Linker)    { 
                    $Linker   = "clang" 
                    if (($BuildVersion -eq [BuildVersion]::DEBUG) -or ($BuildVersion -eq [BuildVersion]::RELEASE_WITH_DEBUG)) {
                        $Linker = "$Linker -g"
                    }
                }
                if ("" -eq $Archiver)  { $Archiver = "llvm-lib $IsDll /OUT:$OutputDirectory/$OutputName.$LibType" }
                $ExecutableFileType = ".exe"
                break
            }
            "UNIX" {
                if ($Build -eq [Build]::DYNAMIC_LIB) {
                    throw "i don't know how to create dynamic libraries on linux so this is not supported"
                }
                #don't know how 
                if ("" -eq $Compiler ) { $Compiler = "clang" }
                if ("" -eq $Linker)    { $Linker   = "clang" }
                if ("" -eq $Archiver)  { $Archiver = "ar cr $OutputDirectory/lib$OutputName.a" }
                break;
            }
        }

        foreach ( $include in $IncludeDirectories ) {
            $Compiler = "$Compiler -I$include"
        }
        $Compiler = "$Compiler $ExtraArgs"

        if ((Test-Path $OutputDirectory) -eq $false) { mkdir $OutputDirectory }
        if ((Test-Path $IntermediateDirectory) -eq $false) { mkdir $IntermediateDirectory }
        
        if ($CreateCompileCommandsFile) {
            $project_files = { $Files }.Invoke() 
            $HasExtra = -not ( IsNull $ExtraCommandFiles ) 
            if ($HasExtra) {
                $project_files += { $ExtraCommandFiles }.Invoke()
            }
            if (Test-Path ./compile_commands.json) {
                Remove-Item ./compile_commands.json
            }
            CreateCompileCommands -Files $project_files -Compiler $Compiler  
        }


        $tobuild = @()
        $linkfiles = ""

        foreach ( $file_str in $Files ) {
            $file = Get-ChildItem $file_str
            if ( ($file.Name).EndsWith(".c") ) {
                $basename = $file.BaseName
                $tobuild += [CompileTarget]::new($file_str, "$IntermediateDirectory/$basename.o")
                $linkfiles = "$linkfiles $IntermediateDirectory/$basename.o"
            }
        }

        if (-not $MsvcBasedCompiler) {
            $ExecutableLinker = BuildFiles -Compiler $Compiler -Files $tobuild -Version $BuildVersion 

            if (($Build -eq [Build]::STATIC_LIB) -or ($Build -eq [Build]::DYNAMIC_LIB)) {
                Invoke-Expression "$Archiver $linkfiles"
            } else {
                foreach ( $libdir in $LibraryDirectories ) {
                    $ExecutableLinker = "$ExecutableLinker -L$libdir"
                }
            
                $LibrariesString = ""
                foreach ( $lib in $Libraries ) {
                    $LibrariesString = "$LibrariesString -l$lib"
                }

                Invoke-Expression "$ExecutableLinker $linkfiles -o $OutputDirectory/$OutputName$ExecutableFileType $LibrariesString"  
            }
        } else {
        
        }

        Set-Location $current_location
    }

    Export-ModuleMember -Function Build-Powershell
    Export-ModuleMember -Function Get-EnumBuild
    Export-ModuleMember -Function Get-EnumBuildVersion
    Export-ModuleMember -Function Get-ClassCompileTarget
}
