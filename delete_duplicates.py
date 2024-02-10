import os
import hashlib

def find_duplicate_files(folder):
    # Dictionary to store files based on their hash
    files_by_hash = {}

    for root, _, files in os.walk(folder):
        for filename in files:
            filepath = os.path.join(root, filename)
            # Calculate the hash of the file content
            with open(filepath, 'rb') as f:
                file_hash = hashlib.sha256(f.read()).hexdigest()
            # Add file path to dictionary based on hash
            if file_hash in files_by_hash:
                files_by_hash[file_hash].append(filepath)
            else:
                files_by_hash[file_hash] = [filepath]

    # Filter out unique files and keep duplicates
    duplicate_files = [files for files in files_by_hash.values() if len(files) > 1]
    return duplicate_files

def delete_duplicate_files(duplicate_files):
    for files in duplicate_files:
        # Keep the first file, delete the rest
        for filepath in files[1:]:
            try:
                os.remove(filepath)
                print(f"Deleted: {filepath}")
            except Exception as e:
                print(f"Error deleting {filepath}: {e}")

if __name__ == "__main__":
    folder_path = input("Enter folder path: ")
    if not os.path.isdir(folder_path):
        print("Invalid folder path.")
    else:
        duplicate_files = find_duplicate_files(folder_path)
        if duplicate_files:
            print("Duplicate files found:")
            for files in duplicate_files:
                for filepath in files:
                    print(filepath)
            delete_duplicates = input("Do you want to delete these files? (y/n): ").strip().lower()
            if delete_duplicates == 'y':
                delete_duplicate_files(duplicate_files)
        else:
            print("No duplicate files found.")
