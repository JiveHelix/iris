
from conan import ConanFile

class IrisConan(ConanFile):
    name = "iris"
    version = "0.3.0"

    python_requires = "boiler/0.1"
    python_requires_extend = "boiler.LibraryConanFile"

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/iris"
    description = "Imaging tools"

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[~1.4]")
        self.requires("fields/[~1.5]")
        self.requires("tau/[~1.12]")
        self.requires("pex/[~1.1]")
        self.requires("wxpex/[~1.0]")
        self.requires("draw/[~0.3]")
        self.requires("fmt/[~10]")
        self.requires("nlohmann_json/[~3]")
