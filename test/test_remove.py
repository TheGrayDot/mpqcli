import subprocess
from pathlib import Path


def test_remove_target_mpq_does_not_exist(binary_path, generate_test_files):
    """
    Test MPQ file removal with a non-existent target.

    This test checks:
    - If the application exits correctly when the target does not exist.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "does" / "not" / "exist"
    target_file = script_dir / "data" / "files" / "cats.txt" 

    result = subprocess.run(
        [str(binary_path), "remove", str(target_file), str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


def test_remove_target_file_does_not_exist(binary_path, generate_test_files):
    """
    Test MPQ file removal with a non-existent file to remove.

    This test checks:
    - If the application exits correctly when the target file to remove does not exist.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "does" / "not" / "exist"
    target_file = script_dir / "data" / "files" / "cats.txt" 

    result = subprocess.run(
        [str(binary_path), "remove", str(target_file), str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


def test_remove_file_from_mpq_archive(binary_path, generate_test_files):
    """
    Test MPQ file removal.

    This test checks:
    - If the application correctly handles removing a file from an MPQ archive.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"
    test_file = "cats.txt"

    result = subprocess.run(
        [str(binary_path), "remove", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
