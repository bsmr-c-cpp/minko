newoption {
	trigger			= "with-offscreen",
	description		= "Enable the Minko Offscreen plugin."
}

PROJECT_NAME = path.getname(os.getcwd())

minko.project.library("minko-plugin-" .. PROJECT_NAME)
	kind "StaticLib"
	language "C++"
	files { "**.hpp", "**.h", "**.cpp", "**.c", "include/**.hpp" }
	includedirs { "include", "src", "lib/osmesa/include" }
