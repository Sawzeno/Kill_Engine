workspace "KillEngine"
architecture "x64"
toolset "v143"
startproject "Sandbox"

filter {"action:vs*"}

configurations {"Debug", "Release", "Dist"}

----------------------------------------------------------------------Defines
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
IncludeDir = {}
IncludeDir["GLFW"] = "KillEngine/vendor/GLFW/include"

----------------------------------------------------------------------Includes
include "KillEngine/vendor/GLFW"

----------------------------------------------------------------------KillEngine project
project "KillEngine"
location "KillEngine"

kind "SharedLib"
language "C++"

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("bin-int/" .. outputdir .. "/%{prj.name}")

pchheader "Keipch.h"
pchsource "KillEngine/src/Keipch.cpp"

----------------------------------------------------------------------
files {"%{prj.name}/src/**.h", "%{prj.name}/src/**.cpp"}

includedirs {"%{prj.name}/vendor/spdlog/include", "%{prj.name}/src", "%{IncludeDir.GLFW}"}

links {"GLFW", "opengl32.lib"}
----------------------------------------------------------------------
filter "system:windows"
cppdialect "C++17"
staticruntime "On"
systemversion "latest"

defines {"KEI_PLATFORM_WINDOWS", "KEI_BUILD_DLL"}

postbuildcommands {("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")}

filter "configurations:Debug"
defines "KEI_DEBUG"
symbols "On"

filter "configurations:Release"
defines "KEI_RELEASE"
optimize "On"

filter "configurations:Dist"
defines "KEI_DIST"
optimize "On"

-----------------------------------------------------------------------Sandbox project
project "Sandbox"
location "Sandbox"
kind "ConsoleApp"
language "C++"

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("bin-int/" .. outputdir .. "/%{prj.name}")

files {"%{prj.name}/src/**.h", "%{prj.name}/src/**.cpp"}

includedirs {"KillEngine/vendor/spdlog/include", "KillEngine/src"}

links {"KillEngine"}

filter "system:windows"
cppdialect "C++17"
staticruntime "On"
systemversion "latest"

defines {"KEI_PLATFORM_WINDOWS"}

filter "configurations:Debug"
defines "KEI_DEBUG"
symbols "On"

filter "configurations:Release"
defines "KEI_RELEASE"
optimize "On"

filter "configurations:Dist"
defines "KEI_DIST"
optimize "On"
