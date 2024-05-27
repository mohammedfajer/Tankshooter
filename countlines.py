import os

def count_lines_in_file(file_path):
    try:
        with open(file_path, 'r', errors='ignore') as file:
            return sum(1 for _ in file)
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
        return 0

def count_lines_in_directory(directory):
    total_lines = 0
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.h') or file.endswith('.c'):
                file_path = os.path.join(root, file)
                lines = count_lines_in_file(file_path)
                total_lines += lines
                print(f"{file_path}: {lines} lines")
    return total_lines

if __name__ == "__main__":
    directory = input("Enter the directory path: ")
    total_lines = count_lines_in_directory(directory)
    print(f"\nTotal lines in .h and .c files: {total_lines}")