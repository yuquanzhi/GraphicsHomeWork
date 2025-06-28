workspace "GraphicsHomeWork"
architecture "x64"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
outputdir="%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Home1.lua"
include "Home2.lua"
