import os
import glob
import itertools
import shutil
import json
import zipfile

print("Creating packages for Unreal Engine")
supported_ue_versions = ["5.0.0", "5.1.0", "5.2.0", "5.3.0", "5.4.0", "5.5.0"]

print("Preparing files")

current_dir = os.path.dirname(os.path.realpath(__file__))
temporary_dir = current_dir + "/../Pubnub/"

shutil.copytree(current_dir, temporary_dir)

os.remove(temporary_dir + "/LICENSE")
os.remove(temporary_dir + "/make_packages.py")
os.remove(temporary_dir + "/.gitignore")
os.remove(temporary_dir + "/.git")
os.remove(temporary_dir + "/.pubnub.yml")
os.remove(temporary_dir + "/README.md")
os.remove(temporary_dir + "/Config/FilterPlugin.ini")
    
shutil.rmtree(temporary_dir + "/.github", ignore_errors=True)
shutil.rmtree(temporary_dir + "/readme_content", ignore_errors=True)
shutil.rmtree(temporary_dir + "/Binaries", ignore_errors=True)
shutil.rmtree(temporary_dir + "/Intermediate", ignore_errors=True)

cpp_files = itertools.chain(
    glob.glob(temporary_dir + "/**/*.cpp", recursive=True),
    glob.glob(temporary_dir + "/**/*.hpp", recursive=True),
    glob.glob(temporary_dir + "/**/*.c", recursive=True),
    glob.glob(temporary_dir + "/**/*.h", recursive=True),
)

for version in supported_ue_versions:
    print("Creating package for Unreal Engine " + version)
    with open(temporary_dir + "/PubnubLibrary.uplugin", "r") as uplugin_file:
        uplugin = json.load(uplugin_file)
        uplugin["EngineVersion"] = version
        plugin_version = uplugin["VersionName"]
    with open(
        temporary_dir + "/PubnubLibrary.uplugin", "w"
    ) as uplugin_file_write:
        json.dump(uplugin, uplugin_file_write, indent=4)

    zipf = zipfile.ZipFile(
        temporary_dir
        + "../Pubnub-"
        + plugin_version
        + "-ue"
        + version
        + ".zip",
        "w",
        zipfile.ZIP_DEFLATED,
    )

    for root, dirs, inner_files in os.walk(temporary_dir):
        for file in inner_files:
            zipf.write(
                os.path.join(root, file),
                os.path.relpath(
                    os.path.join(root, file), os.path.join(temporary_dir, "..")
                ),
            )

    zipf.close()

shutil.rmtree(temporary_dir, ignore_errors=True)