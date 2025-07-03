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
include "Home3.lua"
include "Home4.lua"
include "Home5.lua"
include "Home6.lua"
include "Home7.lua"