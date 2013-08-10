## Project52a

#### Purpose
I'll tell you when I figure it out. When I originally created this repository, I intended to write a project to force-improve Command Prompt. Shortly thereafter, howewver, I found [clink](http://code.google.com/p/clink/) and lost any motivation to continue the project. It's more likely that this will just be a sandbox for me to test some ideas in.

#### Completed

* Write a DLL injector capable of: x86->x86, x64->x64, & x64->x86
* Write the DLL's logic to use an exported function as its entry point, rather than the `DllMain` proc. (See [complications](http://blogs.msdn.com/b/oldnewthing/archive/2004/01/27/63401.aspx).)
* Frankenstein as much code from different sources as possible, guaranteeing that the resulting code will be cumbersome to navigate and ugly as shit.

#### To-Do

* Find a purpose for this project. (Or don't)
* Implement memory cleanup for the injected code. Either pass the memory address to the Init function or set up some IPC mechanism to allow the DLL to signal our injector that cleanup can be done.
* Rewrite the x64 assembly to call `Dll.Init` following the `LoadLibraryW` call.
* [x86->x64 injection](http://code.google.com/p/rewolf-wow64ext) maybe?
* Clean up the code and use a single naming style for functions, variables, types, etc.
* Figure out how to comfortably use Visual Studio as the project's IDE without relying upon MsBuild for the building. (which has been problematic up until now)

#### Credits

* Used a lot of source from [ansicon](https://github.com/adoxa/ansicon). Currently, the `injdll64.c` and the `parent.c` source files are directly ripped from ansicon. Whether that changes is to be seen.