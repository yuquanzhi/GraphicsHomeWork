project "Home1"
	
	location "Home1"
	kind "ConsoleApp"
		language "C++"
		staticruntime "On"
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")	
		files
		{
			"src/Home1/**.hpp",
			"src/Home1/**.cpp",
		}
		includedirs
		{
			"vendor/opencv/build/include",
			"vendor/eigen-3.4.0/Eigen"
		}
		libdirs
		{
			"vendor/opencv/build/x64/vc16/lib"
		}
		links
		{
			
			"opencv_world4110d"
		}
		postbuildcommands {
		 '{COPYDIR} "%{wks.location}/vendor/opencv/build/x64/vc16/bin/opencv_world4110.dll" "%{cfg.targetdir}"'
			}
		filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"


		filter "configurations:Debug"
		defines "LE_DEBUG"
		runtime "Debug"
		symbols "On"
		filter "configurations:Release"
		defines "LE_RELEASE"
		runtime "Release"
		optimize "On"
		filter "configurations:Dist"
		defines "LE_DIST"
		runtime "Release"
		optimize "On"