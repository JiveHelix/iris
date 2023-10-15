
from conans import ConanFile, CMake, tools


class IrisConan(ConanFile):
    name = "iris"
    version = "0.2.8"

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
        self.cpp_info.libs = ["iris"]

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.1]")
        self.requires("fields/[~1.3]")
        self.requires("tau/[~1.10]")
        self.requires("pex/[>=0.9.5 <0.10]")
        self.requires("wxpex/[>=0.9.7 <0.10]")
        self.requires("draw/[~0.2]")
        self.requires("libpng/1.6.39", override=True)
        self.requires("libiconv/1.17")
        self.requires("fmt/[~10]")
        self.requires("nlohmann_json/[~3]")
