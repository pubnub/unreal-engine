import os
import glob
import itertools
import shutil
import json
import zipfile

print("Creating packages for Unreal Engine")
supported_ue_versions = ["5.0.0", "5.1.0", "5.2.0", "5.3.0", "5.4.0", "5.5.0", "5.6.0", "5.7.0"]

print("Preparing files")

# Script lives in Plugins/Pubnub/Scripts/; plugin root is one level up.
script_dir = os.path.dirname(os.path.realpath(__file__))
plugin_root = os.path.normpath(os.path.join(script_dir, ".."))
temporary_dir = os.path.join(plugin_root, "PubnubGamingSDK")

shutil.copytree(plugin_root, temporary_dir)
shutil.rmtree(os.path.join(temporary_dir, "Scripts"), ignore_errors=True)

# os.remove(temporary_dir + "/LICENSE")
# os.remove(temporary_dir + "/make_packages.py")
# os.remove(temporary_dir + "/.gitignore")
# os.remove(temporary_dir + "/.git")
# os.remove(temporary_dir + "/.pubnub.yml")
# os.remove(temporary_dir + "/README.md")
# os.remove(temporary_dir + "/Config/FilterPlugin.ini")

# shutil.rmtree(temporary_dir + "/.github", ignore_errors=True)
# shutil.rmtree(temporary_dir + "/readme_content", ignore_errors=True)
# shutil.rmtree(temporary_dir + "/Binaries", ignore_errors=True)
# shutil.rmtree(temporary_dir + "/Intermediate", ignore_errors=True)

cpp_files = itertools.chain(
    glob.glob(os.path.join(temporary_dir, "**", "*.cpp"), recursive=True),
    glob.glob(os.path.join(temporary_dir, "**", "*.hpp"), recursive=True),
    glob.glob(os.path.join(temporary_dir, "**", "*.c"), recursive=True),
    glob.glob(os.path.join(temporary_dir, "**", "*.h"), recursive=True),
)

for version in supported_ue_versions:
    print("Creating package for Unreal Engine " + version)
    uplugin_path = os.path.join(temporary_dir, "PubnubLibrary.uplugin")
    with open(uplugin_path, "r") as uplugin_file:
        uplugin = json.load(uplugin_file)
        uplugin["EngineVersion"] = version
        plugin_version = uplugin["VersionName"]
    with open(uplugin_path, "w") as uplugin_file_write:
        json.dump(uplugin, uplugin_file_write, indent=4)

    zip_path = os.path.join(
        plugin_root,
        "PubnubGamingSDK-" + plugin_version + "-ue" + version + ".zip",
    )
    zipf = zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED)

    for root, dirs, inner_files in os.walk(temporary_dir):
        for file in inner_files:
            zipf.write(
                os.path.join(root, file),
                os.path.relpath(os.path.join(root, file), plugin_root),
            )

    zipf.close()

shutil.rmtree(temporary_dir, ignore_errors=True)
