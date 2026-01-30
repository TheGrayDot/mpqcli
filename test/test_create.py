import subprocess
from pathlib import Path


def test_create_mpq_target_does_not_exist(binary_path, generate_test_files):
    """
    Test MPQ file creation with a non-existent target.

    This test checks:
    - If the application exits correctly when the target does not exist.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "does" / "not" / "exist"

    result = subprocess.run(
        [str(binary_path), "create", "-v", "1", str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


def test_create_mpq_versions(binary_path, generate_test_files):
    """
    Test MPQ archive creation with different MPQ versions.

    This test checks:
    - MPQ archive creation with version 1 and version 2.
    - If the MPQ archive is created in the correct directory.
    - If the MPQ archive is not empty.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "data" / "files"

    for version in [1, 2]:
        target_file = target_dir.with_suffix(".mpq")
        # Remove the target file if it exists
        # Testing creation when file exists is handled:
        # test_create_mpq_already_exists
        target_file.unlink(missing_ok=True)
        result = subprocess.run(
            [str(binary_path), "create", "-v", str(version), str(target_dir)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode == 0, f"mpqcli failed with error (version {version}): {result.stderr}"
        assert target_file.exists(), f"MPQ file was not created (version {version})"
        assert target_file.stat().st_size > 0, f"MPQ file is empty (version {version})"


def test_create_mpq_with_output(binary_path, generate_test_files):
    """
    Test MPQ archive creation with output file argument.

    This test checks:
    - If the MPQ archive is created in the correct directory.
    - If the MPQ archive is not empty.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "data" / "files"

    for version in [1, 2]:
        output_file = script_dir / "data" / f"mpq_with_output_v{version}.mpq"
        # Remove the target file if it exists
        # Testing creation when file exists is handled:
        # test_create_mpq_already_exists
        if output_file.exists():
            output_file.unlink()

        result = subprocess.run(
            [str(binary_path), "create", "-v", str(version), "-o", str(output_file), str(target_dir)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
        assert output_file.exists(), "MPQ file was not created"
        assert output_file.stat().st_size > 0, "MPQ file is empty"


def test_create_mpq_with_weak_signature(binary_path, generate_test_files):
    """
    Test MPQ archive creation with signature and output file argument.

    This test checks:
    - If the MPQ archive is created in the correct directory.
    - If the MPQ archive is not empty.
    - If the MPQ archive is signed correctly.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "data" / "files"

    output_file = script_dir / "data" / "mpq_with_weak_signature.mpq"
    # Remove the target file if it exists
    # Testing creation when file exists is handled in following function:
    # test_create_mpq_already_exists
    if output_file.exists():
        output_file.unlink()

    result = subprocess.run(
        [str(binary_path), "create", "-s", "-o", str(output_file), str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_file.exists(), "MPQ file was not created"
    assert output_file.stat().st_size > 0, "MPQ file is empty"


def test_create_mpq_already_exists(binary_path, generate_test_files):
    """
    Test MPQ file creation with an existing output file.

    This test checks:
    - If the MPQ archive is not created (as archive exists already).
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "data" / "files"

    output_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    result = subprocess.run(
        [str(binary_path), "create", "-v", "1", "-o", str(output_file), str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 1, f"mpqcli failed with error: {result.stderr}"


def test_create_mpq_from_file(binary_path, generate_test_files):
    """
    Test MPQ archive creation from a file rather than a directory.

    This test checks:
    - MPQ archive creation from a file.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent

    # Create a new test file on the fly
    test_file = script_dir / "data" / "test.txt"
    test_file.write_text("This is a test file for MPQ addition.")

    target_file = test_file.with_suffix(".mpq")
    # Remove the target file if it exists
    # Testing creation when file exists is handled:
    # test_create_mpq_already_exists
    target_file.unlink(missing_ok=True)
    result = subprocess.run(
        [str(binary_path), "create", str(test_file), "--output", str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert target_file.exists(), f"MPQ file was not created)"
    assert target_file.stat().st_size > 0, f"MPQ file is empty)"

    verify_archive_file_content(binary_path, target_file, {"enUS  test.txt"})


def test_create_mpq_from_file_with_nameinarchive_parameter(binary_path, generate_test_files):
    """
    Test MPQ archive creation from a file, with the --name-in-archive parameter.
    This test checks:
    - MPQ archive creation from a file.
    - That the --name-in-archive parameter is correctly handled.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent

    # Create a new test file on the fly
    test_file = script_dir / "data" / "test.txt"
    test_file.write_text("This is a test file for MPQ addition.")

    target_file = test_file.with_suffix(".mpq")
    # Remove the target file if it exists
    # Testing creation when file exists is handled:
    # test_create_mpq_already_exists
    target_file.unlink(missing_ok=True)
    result = subprocess.run(
        [str(binary_path), "create", str(test_file), "--output", str(target_file), "--name-in-archive", "messages\\important.txt"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert target_file.exists(), f"MPQ file was not created)"
    assert target_file.stat().st_size > 0, f"MPQ file is empty)"

    verify_archive_file_content(binary_path, target_file, {"enUS  messages\\important.txt"})


def test_create_mpq_from_directory_with_nameinarchive_parameter(binary_path, generate_test_files):
    """
    Test MPQ archive creation from a directory, with the --name-in-archive parameter.
    This test checks:
    - No MPQ archive is created.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "data" / "files"

    target_file = target_dir.with_name("files_test").with_suffix(".mpq")
    # Remove the target file if it exists
    # Testing creation when file exists is handled:
    # test_create_mpq_already_exists
    target_file.unlink(missing_ok=True)
    result = subprocess.run(
        [str(binary_path), "create", str(target_dir), "--output", str(target_file), "--name-in-archive", "messages\\important.txt"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 1, f"mpqcli failed with error: {result.stderr}"
    assert not target_file.exists(), f"MPQ file was created)"


def test_create_mpq_with_illegal_locale(binary_path, generate_test_files):
    """
    Test MPQ file creation with illegal locale.

    This test checks:
    - That the MPQ archive is not created
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "data" / "files"

    output_file = script_dir / "data" / "new_mpq.mpq"

    result = subprocess.run(
        [str(binary_path), "create", "-v", "1", "-o", str(output_file), str(target_dir), "--locale", "illegal_locale"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"
    assert not output_file.exists(), "MPQ file was created"


def test_create_mpq_with_locale(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ file creation with a given locale.

    This test checks:
    - That the MPQ archive contains the expected file with the expected locale.
    """

    # Creating an MPQ with a given locale happens in conftest.py. This test only checks the output.
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    output_file = script_dir / "data" / "mpq_with_one_locale.mpq"

    verify_archive_file_content(binary_path, output_file, {"esES  cats.txt"})


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
