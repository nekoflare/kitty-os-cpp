import os

def get_all_file_paths(root_folder):
    file_paths = []
    for dirpath, dirnames, filenames in os.walk(root_folder):
        for filename in filenames:
            full_path = os.path.join(dirpath, filename)
            relative_path = os.path.relpath(full_path, root_folder)
            file_paths.append(relative_path)
    return file_paths

timeout_time = 3
os_name = "KittyOS"
protocol = "limine"
kernel_path="boot:///kernel"

config = open("limine.cfg", "w")

if config.closed:
    print("Failure to open limine.cfg file!")
    exit(1)

files = get_all_file_paths("files")

config.write(f"TIMEOUT={timeout_time}\n\n")

# Debug (No KASLR)
config.write(f":{os_name}\n")
config.write(f"\tPROTOCOL={protocol}\n")
config.write(f"\tKASLR=no\n")
for file in files:
    config.write(f"\tMODULE_PATH=boot:///{file}\n")
    config.write(f"\tMODULE_CMDLINE={file}\n")
config.write(f"\tKERNEL_PATH={kernel_path}\n\n")

config.write(f":{os_name} (Normal Mode)\n")
config.write(f"\tPROTOCOL={protocol}\n")
config.write(f"\tKASLR=yes\n")
for file in files:
    config.write(f"\tMODULE_PATH=boot:///{file}\n")
    config.write(f"\tMODULE_CMDLINE={file}\n")
config.write(f"\tKERNEL_PATH={kernel_path}\n\n")


config.close()