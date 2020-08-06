param(
    [string] $OutputDirectory = "bin", 
    [string] $IntermediateDirectory = "bin/int", 
    [switch] $Release, 
    [switch] $RelWithDebug,
    [switch] $BuildTests,
    [switch] $NoBuild
)

class CompileTarget {
    [string]$input_file
    [string]$output_file
    CompileTarget ($input_file, $output_file) { $this.input_file = $input_file; $this.output_file = $output_file }
}

$clang = "clang -Iinclude"
$lib   = "" 
$default_files = [CompileTarget]::new("src/allocator.c", "$IntermediateDirectory/allocator.o"),
                 [CompileTarget]::new("src/phashmap.c",  "$IntermediateDirectory/phashmap.o"),
                 [CompileTarget]::new("src/pio.c",       "$IntermediateDirectory/pio.o"),
                 [CompileTarget]::new("src/pstring.c",   "$IntermediateDirectory/pstring.o"),
                 [CompileTarget]::new("src/util.c",      "$IntermediateDirectory/util.o"),
                 [CompileTarget]::new("src/vector.c",    "$IntermediateDirectory/vector.o")
$files = {$default_files}.Invoke()

function CreateCompileCommands() {
    $loc = Get-Location 
    $loc = $loc -replace "\\", '/'
    $filedata = "["
    $num = 0;
    foreach ( $target in $files ) {
        $file = $target.input_file
        $filedata += "
    {
      `"directory`": `"$loc`",
      `"command`": `"clang -g -Isrc -Iinclude -Itests -Wall -Wextra -Wno-gnu-binary-literal -std=gnu2x $file`",
      `"file`": `"$loc/$file`"
    }"  
        if ($num -ne ($files.Count - 1)) {
            $filedata += ","
        }
        $num = $num + 1;
    }
    $filedata += "`n]"
    New-Item ./compile_commands.json -ItemType File -Value $filedata
}

if ((Test-Path ./compile_commands.json) -eq $false ) {
    CreateCompileCommands 
}

if ($NoBuild) { Exit }

#if output|intermediate directory does not exists we create it
if ((Test-Path $OutputDirectory) -eq $false) { mkdir $OutputDirectory }
if ((Test-Path $IntermediateDirectory) -eq $false) { mkdir $IntermediateDirectory }

switch ($PSVersionTable.Platform) {
    "Win32NT" { 
        $clang = "$clang -D_CRT_SECURE_NO_WARNINGS" 
        $lib   = "llvm-lib /OUT:$OutputDirectory/pstd.lib" 
        break
    }
    "UNIX" {
        $clang = "$clang" 
        break;
    }
}

$commands = $()

$linkfiles = $null
if ($Release) {
    $files.ForEach({ $file = $_
        $input_file = $file.input_file
        $output_file = $file.output_file
        $commands += ,"$clang -O2 -Ofast -std=gnu2x -c $input_file -o $output_file"
    })
} else { 
    if ($RelWithDebug) {
        $files.ForEach({ $file = $_
            $input_file = $file.input_file
            $output_file = $file.output_file
            $commands += ,"$clang -g -O2 -Ofast -std=gnu2x -c $input_file -o $output_file"
        })
    } else {
        $files.ForEach({ $file = $_
            $input_file = $file.input_file
            $output_file = $file.output_file
            $commands += ,"$clang -g -Wall -std=gnu2x -c $input_file -o $output_file"
        })
    }
}
$commands | ForEach-Object -Parallel { 
    Invoke-Expression $_ 
}
  
$linkfiles = ""
$files.ForEach({ $file = $_
    $output_file = $file.output_file
    $linkfiles = "$linkfiles $output_file"
})

Invoke-Expression "$lib $linkfiles"

if ($BuildTests) {
    $tests = Get-ChildItem -File tests 
    
    if ((Test-Path $OutputDirectory/tests) -eq $false) { mkdir $OutputDirectory/tests }
    $testcommands = $()

#########################################################################################################################
#########################################################################################################################
#                                            THIS IS IMPORTANT                                                          #
#########################################################################################################################
#########################################################################################################################
    $formatting_file = "$OutputDirectory/tests/pio.s"
    $formatting_data = ""
#########################################################################################################################
#########################################################################################################################
#########################################################################################################################
#########################################################################################################################

    $tests.ForEach({
        $file = $_
        $basename = $file.BaseName
        $name = $file.Name
        $testcommands +=, "$clang -L$OutputDirectory -g -std=gnu2x tests/$name -o $OutputDirectory/tests/$basename.exe -lpstd"
        $formatting_data = "$formatting_data$clang -L$OutputDirectory -g -std=gnu2x tests/$name -o $OutputDirectory/tests/$basename.exe -lpstd`n"
    })
    
    if ((Test-Path $formatting_file) -eq $true) {
        Remove-Item -Path $formatting_file
    }
    New-Item -Path $formatting_file -ItemType "file" -Value $formatting_data

    $testcommands | ForEach-Object -Parallel {
        Invoke-Expression $_
    }
}


