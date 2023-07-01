workspace "KillEngine"
architecture "x64"
toolset "v143"
startproject "Sandbox"

filter {"action:vs*"}

configurations {"Debug", "Release", "Dist"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
