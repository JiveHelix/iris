from cmake_includes.conan import LibraryConanFile


class IrisConan(LibraryConanFile):
    name = "iris"
    version = "0.3.0"

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/iris"
    description = "Imaging tools"

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.3]")
        self.requires("fields/[~1.4]")
        self.requires("tau/[~1.11]")
        self.requires("pex/[~1.0]")
        self.requires("wxpex/[~1.0]")
        self.requires("draw/[~0.3]")
        self.requires("fmt/[~10]")
        self.requires("nlohmann_json/[~3]")
