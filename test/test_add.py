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
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)

    test_file = script_dir / "does" / "not" / "exist"

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file)],
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
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)

    # Create a new test file on the fly
    test_file = script_dir / "data" / "test.txt"
    test_file.write_text("This is a test file for MPQ addition.")


    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[+] Adding file: test.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


def test_add_file_with_nameinarchive_and_dirinarchive_to_mpq_archive(binary_path, generate_test_files):
    """
    Test MPQ file addition with name-in-archive and dir-in-archive arguments.

    This test checks:
    - If the application correctly handles adding a file to an MPQ archive using the name-in-archive and dir-in-archive arguments.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)

    # Create new test files on the fly
    test_file0 = script_dir / "data" / "test0.txt"
    test_file0.write_text("This is a test file for MPQ addition.")
    test_file1 = script_dir / "data" / "test1.txt"
    test_file1.write_text("This is a another test file for MPQ addition.")


    result = subprocess.run(
        [str(binary_path), "add", str(test_file0), str(target_file), "--dir-in-archive", "directory"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    result = subprocess.run(
        [str(binary_path), "add", str(test_file1), str(target_file), "--name-in-archive", "important\\message.txt"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    result = subprocess.run(
        [str(binary_path), "list", str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    expected_output = {
        # Files from before:
        "cats.txt",
        "dogs.txt",
        "bytes",
        # Files added in this test:
        "directory\\test0.txt",
        "important\\message.txt",
    }
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_add_existing_file_without_overwrite_should_fail(binary_path, generate_test_files):
    """
    Test adding existing files to MPQ archive without the overwrite flag.

    This test checks:
    - If the application correctly prints an error when adding an existing file to an MPQ archive without the overwrite flag.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)


    # Verify that existing file has the expected content before attempting to add a new file with different content
    expected_content = {"This is a file about cats."}
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", expected_content)


    # Create new test files on the fly
    test_file = script_dir / "data" / "cats.txt"
    test_file.write_text("Attempting to make this file about dogs.")


    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[+] Adding file: cats.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = {
        "[!] File already exists in MPQ archive: cats.txt. Skipping...",
    }
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    # Verify that the file content is unchanged
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", expected_content)


def test_add_existing_file_with_overwrite_should_succeed(binary_path, generate_test_files):
    """
    Test adding existing files to MPQ archive with the overwrite flag.

    This test checks:
    - If the application correctly overwrites an existing file to an MPQ archive with the overwrite flag set.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)


    # Verify that file has the expected content before overwriting
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", { "This is a file about cats." })


    # Create new test files on the fly
    test_file = script_dir / "data" / "cats.txt"
    test_file.write_text("This file is suddenly about dogs.")

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file), "--overwrite"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[+] Adding file: cats.txt",
        "[*] File already exists in MPQ archive: cats.txt. Overwriting...",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


    # Verify that file has the expected content after overwriting
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", { "This file is suddenly about dogs." })


def test_add_nonexisting_file_with_overwrite_should_succeed(binary_path, generate_test_files):
    """
    Test adding a non-existing file to MPQ archive with the overwrite flag.

    This test checks:
    - If the application correctly adds a non-existing file to an MPQ archive with the overwrite flag set.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)


    # Create new test files on the fly
    test_file = script_dir / "data" / "test.txt"
    test_file.write_text("This file is newly added.")

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file), "--overwrite"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[+] Adding file: test.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


    # Verify that file has the expected content
    verify_file_in_mpq_has_content(binary_path, target_file, "test.txt", { "This file is newly added." })


def create_mpq_archive_for_test(binary_path, script_dir):
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


def verify_file_in_mpq_has_content(binary_path, mpq_archive, file_name, expected_content):
    result = subprocess.run(
        [str(binary_path), "read", file_name, str(mpq_archive)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # Remove empty lines from output, Windows adds an extra empty line
    output_lines = set(line for line in result.stdout.splitlines() if line.strip() != "")
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_content, f"Unexpected output: {output_lines}"
