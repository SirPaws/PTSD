param(
    [string] $OutputDirectory = "bin", 
    [string] $IntermediateDirectory = "bin/int", 
    [switch] $Release, 
    [switch] $RelWithDebug,
    [switch] $BuildTests,
    [switch] $NoBuild,
    [string] $LibraryName = "pstd"
)
Import-Module ".\PowershellBuild.psm1"

$incdirs = "include", "src", "tests", "bin"

$files = "include/allocator.h",
         "include/dynarray.h",
         "include/general.h",
         "include/phashmap.h",
         "include/pio.h",
         "include/pstring.h",
         "include/util.h",
         "include/vector.h",
         "include/pplatform.h",
         "src/allocator.c",
         "src/phashmap.c",
         "src/pio.c",
         "src/pstring.c",
         "src/util.c",
         "src/vector.c",
         "src/dynarray.c",
         "src/pplatform.c"

$cargs = "-Wall -Wextra -Wno-gnu-binary-literal -std=gnu2x -fms-compatibility-version=19" 

Build-Powershell -CreateCompileCommandsFile       `
    -OutputDirectory       $OutputDirectory       `
    -IntermediateDirectory $IntermediateDirectory `
    -Build STATIC_LIB                             `
    -IncludeDirectories $incdirs                  `
    -ExtraArgs $cargs                             `
    -Files $files                                 `
    -OutputName $LibraryName


if ($BuildTests) {
    $tests = Get-ChildItem ./tests -Exclude "POOF*"
    
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
        
        $libdirs =, $OutputDirectory
        $libs    =, "pstd"
        $test_files =, "tests/$name"

        Build-Powershell -OutputDirectory "$OutputDirectory/tests" `
            -IntermediateDirectory $IntermediateDirectory          `
            -Build EXECUTABLE                                      `
            -IncludeDirectories $incdirs                           `
            -LibaryDirectories  $libdirs                           `
            -Libraries          $libs                              `
            -ExtraArgs $cargs                                      `
            -Files $test_files                                     `
            -OutputName $basename

        $testcommands +=, "$clang -L$OutputDirectory -g -std=gnu2x tests/$name -o $OutputDirectory/tests/$basename.$platformout -lpstd"
        $formatting_data = "$formatting_data$clang -L$OutputDirectory -g -std=gnu2x tests/$name -o $OutputDirectory/tests/$basename.$platformout -lpstd`n"
    })



    if ((Test-Path $formatting_file) -eq $true) {
        Remove-Item -Path $formatting_file
    }
    New-Item -Path $formatting_file -ItemType "file" -Value $formatting_data
}

Remove-Module PowershellBuild

