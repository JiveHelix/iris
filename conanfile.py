
from conans import ConanFile, CMake, tools


class WxpexConan(ConanFile):
    name = "iris"
    version = "0.1.0"

    scm = {
        "type": "git",
        "url": "https://github.com/JiveHelix/iris.git",
        "revision": "auto",
        "submodule": "recursive"}

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/iris"
    description = "Imaging tools"
    topics = ("Computer Vision", "Imaging", "Filters", "C++")

    settings = "os", "compiler", "build_type", "arch"

    generators = "cmake"

    no_copy_source = True

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includes = ["include"]
        self.cpp_info.libs = ["iris"]

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.0]")
        self.requires("fields/[~1]")
        self.requires("tau/[~1.8]")
        self.requires("pex/[~0.8]")
        self.requires("wxpex/[~0.7]")
        self.requires("libpng/1.6.39")
        self.requires("fmt/[~8]")
        self.requires("nlohmann_json/[~3]")
