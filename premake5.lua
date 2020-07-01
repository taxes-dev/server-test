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
      if os.getenv("PROJECT_ID") then
         postbuildcommands { "docker build -t gcr.io/" .. os.getenv("PROJECT_ID") .. "/servertest:v1 --build-arg buildcfg=%{cfg.buildcfg} ." }
      end
