#! /usr/bin/env python3

import io
import os
import platform
import re
import shutil
import subprocess
import sys
from distutils.version import LooseVersion
from pathlib import Path

from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)',
                                                   out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(
                cfg.upper(),
                extdir)]
            if sys.maxsize > 2 ** 32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j2']

        # universal support
        if platform.system() == "Darwin":
            print("running on osx")
            cmake_args += [
                '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64',
                '-DCMAKE_OSX_DEPLOYMENT_TARGET=12',
                '-G Xcode'
            ]
            build_args = ['--config', cfg]

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get('CXXFLAGS', ''),
            self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args,
                              cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args,
                              cwd=self.build_temp)

        # post-process release
        if platform.system() == "Darwin":

            # move files from Debug / Release
            release_dir = Path(self.build_lib, cfg)
            parent_dir = release_dir.parent
            for file in release_dir.iterdir():
                if file.is_file():
                    shutil.move(file, parent_dir)

            os.removedirs(release_dir)

        print()  # Add an empty line for cleaner output
        print(self.build_temp)


this_directory = os.path.abspath(os.path.dirname(__file__))
with io.open(os.path.join(this_directory, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='pynocchio',
    version='0.0.4',
    author='Alexander Larin',
    author_email='ekzebox@gmail.com',
    description='The pinocchio extension library',
    long_description=long_description,
    long_description_content_type='text/x-rst',
    url='https://github.com/alexanderlarin/pynocchio',
    packages=find_packages('src'),
    package_dir={'': 'src'},
    ext_modules=[CMakeExtension('pynocchio')],
    cmdclass=dict(build_ext=CMakeBuild),
    test_suite='tests',
    zip_safe=False,
)
