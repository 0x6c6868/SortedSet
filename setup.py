from distutils.core import setup, Extension

setup(
    name="sorted_set",
    version="dev",
    ext_modules=[Extension("sorted_set",
                           ["src/sortedset.c"])]
)
