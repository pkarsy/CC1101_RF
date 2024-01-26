Import("env")
import os

env["PROJECT_SRC_DIR"] = os.path.join(env["PROJECT_DIR"],"examples", env["PIOENV"].replace('-',os.path.sep))

print("From extra_script src_dir = ", env["PROJECT_SRC_DIR"])
