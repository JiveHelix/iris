
from conan import ConanFile

class IrisConan(ConanFile):
    name = "iris"
    version = "0.4.0"

    python_requires = "boiler/0.2"
    python_requires_extend = "boiler.LibraryConanFile"

    license = "MIT"
    author = "Jive Helix (jivehelix@gmail.com)"
    url = "https://github.com/JiveHelix/iris"
    description = "Imaging tools"

    def build_requirements(self):
        self.test_requires("catch2/2.13.8")

    def requirements(self):
        self.requires("jive/[>=1.7 <2]")
        self.requires("fields/[>=1.8 <2]")
        self.requires("tau/[>=1.16 <2]")
        self.requires("pex/[>=1.4 <2]")
        self.requires("wxpex/[>=1.0 <2]")
        self.requires("ray/[~1.2]")
        self.requires("draw/[~0.4]")
        self.requires("fmt/[~10]")
