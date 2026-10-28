from skbuild import setup

setup(
    name="zmod4510",
    version="0.2.0",
    description="ZMOD4510 firmware with Python bindings",
    packages=["zmod4510"],
    package_dir={"zmod4510": "python"},
    cmake_install_dir=".",
    # The module the test script imports must use src/reimpl, not the archives.
    cmake_args=["-DUSE_REIMPL=ON"],
    python_requires=">=3.10",
)
