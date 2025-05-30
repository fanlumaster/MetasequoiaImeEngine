import os


def normpath(path):
    return path.replace("\\", "/")


local_app_data_dir = os.environ.get("LOCALAPPDATA")
home_dir = os.path.expanduser("~")

cur_file_path = os.path.dirname(os.path.abspath(__file__))
project_root_path = os.path.dirname(os.path.dirname(cur_file_path))

user_home = os.path.expanduser("~")

FanImeEngine_root = normpath(project_root_path)
vcpkg_include = normpath(
    os.path.join(
        FanImeEngine_root, "tests", "build", "vcpkg_installed", "x64-windows", "include"
    )
)
utfcpp_path = normpath(os.path.join(FanImeEngine_root, "utfcpp", "source"))
boost_path = normpath(os.path.join(user_home, "scoop", "apps", "boost", "1.88.0"))

# project_root/.clangd
dot_clangd_new_lines = [
    f'      "-I{FanImeEngine_root}",\n',
    f'      "-I{vcpkg_include}",\n',
    f'      "-I{utfcpp_path}",\n',
    f'      "-I{boost_path}",\n',
]
dot_clangd_file = os.path.join(
    FanImeEngine_root, "tests", "scripts", "config_files", ".clangd"
)
dot_clangd_output_file = os.path.join(FanImeEngine_root, ".clangd")
with open(dot_clangd_file, "r", encoding="utf-8") as f:
    lines = f.readlines()
lines[6:10] = dot_clangd_new_lines
with open(dot_clangd_output_file, "w", encoding="utf-8") as f:
    f.writelines(lines)

# project_root/tests/CMakeLists.txt
CMakeLists_new_line = (
    f'set(Boost_ROOT "{normpath(user_home)}/scoop/apps/boost/current")\n'
)
CMakeLists_file = os.path.join(
    FanImeEngine_root, "tests", "scripts", "config_files", "CMakeLists.txt"
)
CMakeLists_output_file = os.path.join(FanImeEngine_root, "tests", "CMakeLists.txt")
with open(CMakeLists_file, "r", encoding="utf-8") as f:
    lines = f.readlines()
lines[18] = CMakeLists_new_line
with open(CMakeLists_output_file, "w", encoding="utf-8") as f:
    f.writelines(lines)

# CMakePresets.json
CMakePresets_new_line_8 = (
    f'        "VCPKG_ROOT": "{normpath(user_home)}/scoop/apps/vcpkg/current/"\n'
)
CMakePresets_new_line_11 = f'        "CMAKE_TOOLCHAIN_FILE": "{normpath(user_home)}/scoop/apps/vcpkg/current/scripts/buildsystems/vcpkg.cmake",\n'
CMakePresets_file = os.path.join(
    FanImeEngine_root, "tests", "scripts", "config_files", "CMakePresets.json"
)
CMakePresets_file_output_file = os.path.join(
    FanImeEngine_root, "tests", "CMakePresets.json"
)
with open(CMakePresets_file, "r", encoding="utf-8") as f:
    lines = f.readlines()
lines[8] = CMakePresets_new_line_8
lines[11] = CMakePresets_new_line_11
with open(CMakePresets_file_output_file, "w", encoding="utf-8") as f:
    f.writelines(lines)
