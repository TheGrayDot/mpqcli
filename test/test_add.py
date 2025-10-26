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
    target_mpq = script_dir / "does" / "not" / "exist"
    target_file = script_dir / "data" / "files" / "cats.txt"

    result = subprocess.run(
        [str(binary_path), "add", str(target_file), str(target_mpq)],
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
    target_mpq = script_dir / "data" / "files.mpq"
    target_file = script_dir / "does" / "not" / "exist"

    result = subprocess.run(
        [str(binary_path), "add", str(target_file), str(target_mpq)],
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
        "[+] Adding file for locale 0: test.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    expected_content = {
        "enUS  bytes",
        "enUS  dogs.txt",
        "enUS  cats.txt",
        "enUS  test.txt",
    }
    verify_archive_file_content(binary_path, target_file, expected_content)


def test_create_mpq_with_illegal_locale(binary_path, generate_test_files):
    """
    Test MPQ file addition with illegal locale.

    This test checks:
    - That when an illegal locale is provided, no file is added.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)

    # Verify that the archive has the expected content
    expected_content = {
        "enUS  bytes",
        "enUS  dogs.txt",
        "enUS  cats.txt",
    }
    verify_archive_file_content(binary_path, target_file, expected_content)


    # Create a new test file on the fly
    test_file = script_dir / "data" / "horses.txt"
    test_file.write_text("This is a file about horses.")

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file), "--locale", "illegal_locale"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


    # Verify that the archive still has the same content
    expected_content = {
        "enUS  bytes",
        "enUS  dogs.txt",
        "enUS  cats.txt",
    }
    verify_archive_file_content(binary_path, target_file, expected_content)


def test_create_mpq_with_locale(binary_path, generate_test_files):
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
    test_file = script_dir / "data" / "cats.txt"
    test_file.write_text("Este es un archivo sobre gatos.")

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file), "--locale", "esES"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[+] Adding file for locale 1034: cats.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


    # Verify that the archive has the expected content
    expected_content = {
        "enUS  bytes",
        "enUS  dogs.txt",
        "enUS  cats.txt",
        "esES  cats.txt",
    }
    verify_archive_file_content(binary_path, target_file, expected_content)


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


def verify_archive_file_content(binary_path, test_file, expected_output):
    result = subprocess.run(
        [str(binary_path), "list", str(test_file), "-d", "-p", "locale"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
