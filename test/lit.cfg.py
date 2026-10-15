# -*- Python -*-

import os

import lit.formats
from lit.llvm import llvm_config

config.name = "MAGIC"
# Always use lit's internal shell: external execution is deprecated as of
# lit 23 and is rejected by newer lit releases.
config.test_format = lit.formats.ShTest(execute_external=False)
config.suffixes = [".mlir"]

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.magic_obj_root, "test")

config.substitutions.append(("%PATH%", config.environment["PATH"]))
config.substitutions.append(("%shlibext", config.llvm_shlib_ext))

llvm_config.with_system_environment(["HOME", "INCLUDE", "LIB", "TMP", "TEMP"])
llvm_config.use_default_substitutions()

config.excludes = ["CMakeLists.txt", "lit.cfg.py", "lit.site.cfg.py"]

config.magic_tools_dir = os.path.join(config.magic_obj_root, "bin")
llvm_config.with_environment("PATH", config.llvm_tools_dir, append_path=True)

tool_dirs = [config.magic_tools_dir, config.llvm_tools_dir]
tools = ["magic-opt"]
llvm_config.add_tool_substitutions(tools, tool_dirs)
