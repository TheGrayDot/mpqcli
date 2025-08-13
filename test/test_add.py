import subprocess
from pathlib import Path


def test_add_target_mpq_does_not_exist(binary_path, generate_test_files):
    """
    Test MPQ file addition with a non-existent target.

    This test checks:
    - If the application exits correctly when the target does not exist.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "does" / "not" / "exist"
    target_file = script_dir / "data" / "files" / "cats.txt" 

    result = subprocess.run(
        [str(binary_path), "add", str(target_file), str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


def test_add_target_file_does_not_exist(binary_path, generate_test_files):
    """
    Test MPQ file addition with a non-existent file to add.

    This test checks:
    - If the application exits correctly when the target file to add does not exist.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "does" / "not" / "exist"
    target_file = script_dir / "data" / "files" / "cats.txt" 

    result = subprocess.run(
        [str(binary_path), "add", str(target_file), str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


def test_add_file_to_mpq_archive(binary_path, generate_test_files):
    """
    Test MPQ file addition.

    This test checks:
    - If the application correctly handles adding a file to an MPQ archive.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent

    # Start by creating an MPQ archive for this test
    target_dir = script_dir / "data" / "files"
    target_file = target_dir.with_suffix(".mpq")
    # Remove the target file if it exists
    # Testing creation when file exists is handled:
    # test_create_mpq_already_exists
    target_file.unlink(missing_ok=True)
    result = subprocess.run(
        [str(binary_path), "create", "-v", "1", str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert target_file.exists(), "MPQ file was not created"
    assert target_file.stat().st_size > 0, "MPQ file is empty"

    # Create a new test file on the fly
    test_file = script_dir / "data" / "test.txt"
    test_file.write_text("This is a test file for MPQ addition.")

    target_file = script_dir / "data" / "files.mpq"

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
