**rptoon**\
Restored and modified rptoon plugin for working without recompile rpskin plugin with toon fx feature for rw 3.7 (windows d3d9 only) that was get from [repo](https://github.com/sigmaco/rwsrc-v37-pc/tree/master)\
All modified rw code or added modules were marked with `NEW` keyword as comments (at start of module or any changed code lines)

**Build**\
Currently only MS Visual Studio build support. So required any version of this product and installed MS DirectX 9.0c SDK with `DXSDK_DIR` environment variable is set. Generate `.sln` file by [premake5](https://premake.github.io/) and build debug or release lib

**Credits**\
All sources were taken from this [repo](https://github.com/sigmaco/rwsrc-v37-pc/tree/master) that unfortunately contains already modified too rptoon plugin that not drawing anything directly