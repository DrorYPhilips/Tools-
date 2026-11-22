README for THIRDPARTY/LIVEMEDIA

The latest live555 source package can be obtained from http://www.live555.com/liveMedia/

Copy the contents of the downloaded live directory into the live directory in the 'live' directory.

Build the live libraries according to recommended instructions.


To build the live libraries:


Navigate to the live directory and open the file win32config in a text editor.
Change the TOOLS32 line to reference your Visual Studio installation; for example,
Change
TOOLS32	=		c:\Program Files\DevStudio\Vc
To
TOOLS32	=		c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC


Remove reference to msvcirt.lib from the LINK_OPTS_0 line, for example,
Change
LINK_OPTS_0 =		$(linkdebug) msvcirt.lib
To
LINK_OPTS_0 =		$(linkdebug)


To build the libraries with debug information, comment out the NODEBUG line, for example,
Change
NODEBUG=1
To
#NODEBUG=1


To generate make files for each of the four necessary libraries, run the genWindowsMakefiles script.
To build the libraries, use nmake with the make file located in each project directory.

See build_live.bat for further information on building the libraries using nmake.
build_live.bat can be viewed and edited in a text editor.
To run build_live.bat (ensure that you have viewed the script and edited it as necessary first), run build_live.bat from a command prompt.
