workspace "KillEngine"

configurations {"Debug", "Release", "Dist"}

architecture "x64"

toolset "v143"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

startproject "Sandbox"

project "KillEngine"
location "KillEngine"
kind "SharedLib"
language "C++"
targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("bin-int/" .. outputdir .. "/%{prj.name}")
files {"%{prj.name}/src/**.h", "%{prj.name}/src/**.cpp"}

includedirs {"%{prj.name}/vendor/spdlog/include", "%{prj.name}/src"}

filter "system:windows"

cppdialect "C++17"
staticruntime "On"

systemversion "latest"

defines {"KILL_PLATFORM_WINDOWS", "KILL_BUILD_DLL"}

postbuildcommands {
    ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
}

filter "configurations:Debug"
defines {"KILL_DEBUG"}
symbols "On"

filter "configurations:Release"
defines {"KILL_RELEASE"}
optimize "On"

filter "configurations:Dist"
defines {"KILL_DIST"}
optimize "On"

---------------------------------------------

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

defines {"KILL_PLATFORM_WINDOWS"}

filter "configurations:Debug"
defines {"KILL_DEBUG"}
symbols "On"

filter "configurations:Release"
defines {"KILL_RELEASE"}
optimize "On"

filter "configurations:Dist"
defines {"KILL_DIST"}
optimize "On"
