dir_bin = "%{wks.location}/../../bin/lib/d3d9/%{cfg.buildcfg}"
dir_obj = "%{wks.location}/../../obj/%{cfg.buildcfg}/%{prj.name}"
dir_src = "%{wks.location}/../../src"
dir_lib = "%{wks.location}/../../vendor"
dir_pch = "src"


workspace "rptoon"      
   location "build/%{_ACTION}"
   filename "rptoon"
   configurations { "debug", "release" }
   system "Windows"
   startproject "rptoon"
   architecture "x86"
   targetdir "%{dir_bin}"
   objdir "%{dir_obj}"
   language "C"
   rtti "Off"
   characterset "MBCS"
   defines { "WIN32", "_WINDOWS" }
   includedirs  { "%{dir_src}" }  
   files  {  "%{dir_src}/**.h", "%{dir_src}/**.c", }
   filter { "kind:WindowedApp" }
      linkoptions { "/SAFESEH:NO", "/DYNAMICBASE:NO" }
   filter { "kind:WindowedApp or StaticLib" }
      linkoptions { "/MACHINE:X86" }   
   filter "configurations:Debug"
      defines { "_DEBUG", "RWDEBUG" }
      symbols "On"
      optimize "Off"
      intrinsics "Off"
      inlining "Disabled"
      staticruntime "On"
      runtime "Debug"
      editandcontinue "On"
   filter "configurations:Release"
      defines { "NDEBUG" }
      symbols "Off"
      optimize "Full"
      intrinsics "On"
      inlining "Auto"
      staticruntime "On"
      runtime "Release"
      editandcontinue "Off"
      flags { "NoIncrementalLink", "NoBufferSecurityCheck", "NoRuntimeChecks", "MultiProcessorCompile" }


project "rptoon"
   targetname "%{wks.name}"
   kind "StaticLib"
   includedirs  { "$(DXSDK_DIR)Include", "%{dir_src}/d3d9", "%{dir_lib}/rwsdk37/include/d3d9" }
   filter { "configurations:Debug" }
      libdirs  { "%{dir_lib}/rwsdk37/lib/d3d9/debug" }
   filter { "configurations:Release" }
      libdirs  { "%{dir_lib}/rwsdk37/lib/d3d9/release" }