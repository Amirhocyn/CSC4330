Developer Intallation Instructions
---------------------------------------------

Make sure you have installed Git and CMake

Git download link: https://git-scm.com/install/

CMake download link (Install latest release, currently 4.1.2, not release candidate, 4.2.0): https://cmake.org/download/

Update your system environment variable PATH to include the locations of the executable binaries.
Verify that they've been added to the path by opening a terminal and inputting the following commands:

Windows (CMD):

~~~~~~~~~~~~~~~~~~~
where git
~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~
where cmake
~~~~~~~~~~~~~~~~~~~

Windows (Powershell):

~~~~~~~~~~~~~~~~~~~
where.exe git
~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~
where.exe cmake
~~~~~~~~~~~~~~~~~~~

Linux/MacOS (Bash or Zsh):

~~~~~~~~~~~~~~~~~~~
which git
~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~
which cmake
~~~~~~~~~~~~~~~~~~~

If both return with at least one path, then you've properly added them.

------------------------------------------------------------------------
------------------------------------------------------------------------

Then, install vcpkg and set environment variable by follwing these steps:

Open a terminal, then run the following commands from your initial directory:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
git clone https://github.com/microsoft/vcpkg.git
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~
cd vcpkg
~~~~~~~~~~~~~


Run the bootstrap script, the command differs by shell

Windows (CMD):

~~~~~~~~~~~~~~~~~~~~~~~
bootstrap-vcpkg.bat
~~~~~~~~~~~~~~~~~~~~~~~

Windows (Powershell):

~~~~~~~~~~~~~~~~~~~~~~~~~
.\bootstrap-vcpkg.bat
~~~~~~~~~~~~~~~~~~~~~~~~~

Linux/MacOS (Bash or Zsh):

~~~~~~~~~~~~~~~~~~~~~~~~~
./bootstrap-vcpkg.sh
~~~~~~~~~~~~~~~~~~~~~~~~~

Now, this is where the Environment Variable VCPKG_ROOT should point to, so we'll take a moment to set that environment variable from the command line, as well as add this to the PATH so you can call vcpkg from CLI at any time. The commands are slightly different depending on OS and shell.


Windows (CMD):

~~~~~~~~~~~~~~~~~~~~~~~~~~~
setx VCPKG_ROOT "%cd%"
~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~
setx PATH "%cd%;%PATH%"
~~~~~~~~~~~~~~~~~~~~~~~~~~~


Windows (Powershell):

~~~~~~~~~~~~~~~~~~~~~~~~~~~
setx VCPKG_ROOT "$pwd"
~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
setx PATH "$pwd;$env:PATH"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Linux/MacOS (Bash)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo "export VCPKG_ROOT=$PWD" >> ~/.bash_profile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo "export PATH=\"$PWD:$PATH\"" >> ~/.bash_profile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Linux/MacOS (Zsh)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo "export VCPKG_ROOT=$PWD" >> ~/.zshenv
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo "export PATH=\"$PWD:$PATH\"" >> ~/.zshenv
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


You will have to close and reopen the terminal for it to recognize the variables, but you can then check that they properly exist by running


Windows (CMD):

~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo %VCPKG_ROOT%
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Windows (Powershell)

~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo $env:VCPKG_ROOT
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Linux/MacOS (Bash or Powershell)

~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo $VCPKG_ROOT
~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you see the directory, then you've done it successfully.

------------------------------------------------------------------------
------------------------------------------------------------------------

OPTIONAL: To integrate vcpkg with VS Code, after setting the environment variables and reopening a terminal, run the command:

~~~~~~~~~~~~~~~~~~~~~~~~~~~
vcpkg integrate install
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will integrate vcpkg with MSBuild environment (on Windows) and VS Code (on MacOS), allowing it to link to installed libraries and use intellisense


------------------------------------------------------------------------
------------------------------------------------------------------------

Next, install the following VS Code Extensions:


> C/C++ Extension Pack (Contains 3 extensions)

> Cmake Tools

> Cmake Test Explorer 

> Vcpkg Cmake Tools 

> Python (Contains 3 extensions) 

> Python PyPi Assistant



This should be all the required/very useful extensions necessary for the project


------------------------------------------------------------------------
------------------------------------------------------------------------


Configuring and Building the executable

Cmake Tools will add a sidebar icon (a triangle with a wrench) for easy Cmake configuration. With the folder with a CMakeLists.txt in it open, you can select the folder, then select the Configure preset (I suggest msvc), then on the same line as the "Configure" at the right will be an icon of a small paper with an arrow with tooltip "Configure." Click that to start configuration. 
During configuration, all dependencies will be checked and installed, so on your first run this could take upwards of 40 minutes. However, it caches binaries after finishing, so later configurations (even on a different triplet) will take almost no time at all unless new dependencies are added.

After configuration completes and the Output window states "Build files have been written to: $SOME_PATH", you can build.

Make sure the preset under "Build" in the sidebar is set to the same as Configure (in this case, msvc), then to the right of "Build" will be an icon like a small bin with a few paper balls in it with tooltip "Build." Click this to build the executable.
Once the output states "Build completed" and "Build finished with exit code 0," the executable will be found in ./build/msvc/ as "FileDropOCRTest.exe"

------------------------------------------------------------------------
------------------------------------------------------------------------

Possible Issues:

If Configure fails due to pip error and you get a warning that longPathAware might not be set, you should be able to fix it by opening powershell as an administrator and entering the command

~~~~~~~~~~~~~~~~~~~~~~~~~~~
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
~~~~~~~~~~~~~~~~~~~~~~~~~~~


 


