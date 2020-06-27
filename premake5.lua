-- premake5.lua
workspace "ServerTest"
   configurations { "Debug", "Release" }

project "ServerTest"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   includedirs { "include" }

   files { "**.cpp" }


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      postbuildcommands { "docker build -t taxes:servertest --build-arg buildcfg=%{cfg.buildcfg} ." }
