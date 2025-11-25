import subprocess
from pathlib import Path


def test_remove_target_mpq_does_not_exist(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ file removal with a non-existent target.

    This test checks:
    - If the application exits correctly when the target does not exist.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = "cats.txt"
    target_file = script_dir / "does" / "not" / "exist.mpq"

    result = subprocess.run(
        [str(binary_path), "remove", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


def test_remove_target_file_does_not_exist(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ file removal with a non-existent file to remove.

    This test checks:
    - If the application exits correctly when the target file to remove does not exist.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = "does-not-exist.txt"
    target_file = script_dir / "data" / "mpq_with_many_locales.mpq"

    result = subprocess.run(
        [str(binary_path), "remove", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[-] Removing file for locale 0: does-not-exist.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = {
        "[!] Failed: File doesn't exist for locale 0: does-not-exist.txt",
    }
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


def test_remove_file_from_mpq_archive(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ file removal.

    This test checks:
    - If the application correctly handles removing a file from an MPQ archive.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = "cats.txt"
    target_file = script_dir / "data" / "mpq_with_many_locales.mpq"

    result = subprocess.run(
        [str(binary_path), "remove", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_output = {
        "[-] Removing file for locale 0: cats.txt",
    }
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


def test_remove_file_with_locale_from_mpq_archive(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ file removal with locale.

    This test checks:
    - If the application correctly handles removing a file with the given locale from an MPQ archive.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = "cats.txt"
    target_file = script_dir / "data" / "mpq_with_many_locales.mpq"

    expected_output = {
        "enUS  cats.txt",
        "deDE  cats.txt",
        "esES  cats.txt",
    }
    verify_archive_content(binary_path, target_file, expected_output)

    # Removing without specifying locale means removing using locale 0 = enUS
    result = subprocess.run(
        [str(binary_path), "remove", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    expected_output = {
        "deDE  cats.txt",
        "esES  cats.txt",
    }
    verify_archive_content(binary_path, target_file, expected_output)


    result = subprocess.run(
        [str(binary_path), "remove", str(test_file), str(target_file), "--locale", "esES"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    expected_output = {
        "deDE  cats.txt",
    }
    verify_archive_content(binary_path, target_file, expected_output)


def verify_archive_content(binary_path, target_file, expected_output):
    # Verify that the archive has the expected content
    result = subprocess.run(
        [str(binary_path), "list", "-d", str(target_file), "-p", "locale"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    output_lines = set(result.stdout.splitlines())
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
