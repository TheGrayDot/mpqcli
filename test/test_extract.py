import subprocess
from pathlib import Path


def test_extract_mpq_target_does_not_exist(binary_path, generate_test_files):
    """
    Test MPQ file extraction with a non-existent target.

    This test checks:
    - If the application exits correctly when the target does not exist.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_dir = script_dir / "does" / "not" / "exist"

    result = subprocess.run(
        [str(binary_path), "extract", str(target_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 105, f"mpqcli failed with error: {result.stderr}"


def test_extract_mpq_default_options(binary_path, generate_test_files):
    """
    Test MPQ archive extraction with default options.

    This test checks:
    - If the MPQ archive is extracted correctly.
    - If the output files match the expected files.
    - If the output directory is created.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "cats.txt",
        "dogs.txt",
        "bytes",
        "(listfile)",
        "(attributes)",
    }

    result = subprocess.run(
        [str(binary_path), "extract", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    # Create expected_lines set based on expected output with prefix
    expected_lines = {f"[*] Extracted: {line}" for line in expected_output}

    # Create output_file path without suffix (default extract behavior is MPQ without extension)
    output_file = test_file.with_suffix("")

    # Create output_files set based on directory contents (not full path)
    output_files = set(fi.name for fi in output_file.glob("*"))

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_lines, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_files == expected_output, f"Unexpected files: {output_files}"


def test_extract_mpq_output_directory_specified(binary_path, generate_test_files):
    """
    Test MPQ archive extraction with specified output directory.

    This test checks:
    - If the MPQ archive is extracted correctly.
    - If the output files match the expected files.
    - If the output directory is created.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"
    output_dir = script_dir / "data" / "extracted"

    expected_output = {
        "cats.txt",
        "dogs.txt",
        "bytes",
        "(listfile)",
        "(attributes)",
    }

    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    # Create expected_lines set based on expected output with prefix
    expected_lines = {f"[*] Extracted: {line}" for line in expected_output}

    # Create output_file path without suffix (default extract behavior is MPQ without extension)
    output_file = output_dir.with_suffix("")

    # Create output_files set based on directory contents (not full path)
    output_files = set(fi.name for fi in output_file.glob("*"))

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_lines, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_files == expected_output, f"Unexpected files: {output_files}"


def test_extract_file_from_mpq_output_directory_specified(binary_path, generate_test_files):
    """
    Test MPQ archive file extraction with specified output directory.

    This test checks:
    - If the file is extracted correctly.
    - If the output files match the expected files.
    - If the output directory is created.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"
    output_dir = script_dir / "data" / "extracted_file"
    for fi in output_dir.glob("*"):
        fi.unlink(missing_ok=True)


    expected_output = {
        "cats.txt",
    }

    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), "-f", "".join(expected_output), str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    # Create expected_lines set based on expected output with prefix
    expected_lines = {f"[*] Extracted: {line}" for line in expected_output}

    # Create output_file path without suffix (default extract behavior is MPQ without extension)
    output_file = output_dir.with_suffix("")

    # Create output_files set based on directory contents (not full path)
    output_files = set(fi.name for fi in output_file.glob("*"))

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_lines, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_files == expected_output, f"Unexpected files: {output_files}"


def test_extract_file_from_mpq_with_locale(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ archive file extraction with a specified locale.

    This test checks:
    - If the file with the given locale is extracted correctly.
    - If the output files match the expected files.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_many_locales.mpq"
    output_dir = script_dir / "data" / "extracted_file"
    file_to_extract = "cats.txt"
    locale = "esES"
    for fi in output_dir.glob("*"):
        fi.unlink(missing_ok=True)

    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), "-f", file_to_extract, str(test_file), "--locale", locale],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    expected_stdout = {
        "[*] Extracted: " + file_to_extract
    }
    output_file = output_dir / file_to_extract
    expected_content = "Este es un archivo sobre gatos."

    output_lines = set(result.stdout.splitlines())
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_stdout, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_file.read_text(encoding="utf-8") == expected_content, "Unexpected file content"


def test_extract_file_from_mpq_with_default_locale(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ archive file extraction with no specified locale but many matching file names.

    This test checks:
    - If the file with the default locale is extracted correctly.
    - If the output files match the expected files.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_many_locales.mpq"
    output_dir = script_dir / "data" / "extracted_file"
    file_to_extract = "cats.txt"
    for fi in output_dir.glob("*"):
        fi.unlink(missing_ok=True)


    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), "-f", file_to_extract, str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    expected_stdout = {
        "[*] Extracted: " + file_to_extract
    }
    output_file = output_dir / file_to_extract
    expected_content = "This is a file about cats."

    output_lines = set(result.stdout.splitlines())
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_stdout, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_file.read_text(encoding="utf-8") == expected_content, "Unexpected file content"


def test_extract_file_from_mpq_with_illegal_locale(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ archive file extraction with an illegal locale.

    This test checks:
    - When a locale is given that does not exist in the file, the file with the default locale is extracted.
    - If the output files match the expected files.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_many_locales.mpq"
    output_dir = script_dir / "data" / "extracted_file"
    file_to_extract = "cats.txt"
    locale = "nosuchlocale"
    for fi in output_dir.glob("*"):
        fi.unlink(missing_ok=True)


    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), "-f", file_to_extract, str(test_file), "--locale", locale],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    expected_stdout = {
        "[!] Warning: The locale 'nosuchlocale' is unknown. Will use default locale instead.",
        "[*] Extracted: " + file_to_extract
    }
    output_file = output_dir / file_to_extract
    expected_content = "This is a file about cats."

    output_lines = set(result.stdout.splitlines())
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_stdout, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_file.read_text(encoding="utf-8") == expected_content, "Unexpected file content"


def test_extract_file_from_mpq_with_locale_not_in_file(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ archive file extraction with a specified locale that is not in the file.

    This test checks:
    - When a locale is given that does not exist in the file, the file with the default locale is extracted.
    - If the output files match the expected files.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_many_locales.mpq"
    output_dir = script_dir / "data" / "extracted_file"
    file_to_extract = "cats.txt"
    locale = "ptPT"  # There is no file for this locale
    for fi in output_dir.glob("*"):
        fi.unlink(missing_ok=True)

    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), "-f", file_to_extract, str(test_file), "--locale", locale],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    expected_stdout = {
        "[*] Extracted: " + file_to_extract
    }
    output_file = output_dir / file_to_extract
    expected_content = "This is a file about cats."

    output_lines = set(result.stdout.splitlines())
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_stdout, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_file.read_text(encoding="utf-8") == expected_content, "Unexpected file content"


def test_extract_file_from_mpq_with_no_locale_argument_and_no_default_locale(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ archive file extraction without a specified locale, and no file with the default locale.

    This test checks:
    - When no locale is given, and no file by that name exists for the default locale,
    but one does for a different one, no file is extracted.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_one_locale.mpq"
    output_dir = script_dir / "data" / "extracted_file"
    file_to_extract = "cats.txt"  # There is a file by this name, but for locale esES
    for fi in output_dir.glob("*"):
        fi.unlink(missing_ok=True)


    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), "-f", file_to_extract, str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    expected_stdout_output = set()
    expected_stderr_output = {
        "[!] Failed: File doesn't exist: " + file_to_extract,
    }

    stdout_output_lines = set(result.stdout.splitlines())
    stderr_output_lines = set(result.stderr.splitlines())

    output_file = output_dir / file_to_extract

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert stdout_output_lines == expected_stdout_output, f"Unexpected output: {stdout_output_lines}"
    assert stderr_output_lines == expected_stderr_output, f"Unexpected output: {stderr_output_lines}"
    assert not output_file.exists(), "Output directory was not created"


def test_extract_file_from_mpq_with_wrong_locale_argument_and_no_default_locale(binary_path, generate_locales_mpq_test_files):
    """
    Test MPQ archive file extraction with a specified locale, and no file with the default locale.

    This test checks:
    - When no locale is given, and no file by that name exists for the default locale,
    but one does for a different one, no file is extracted.
    """
    _ = generate_locales_mpq_test_files
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_one_locale.mpq"
    output_dir = script_dir / "data" / "extracted_file"
    file_to_extract = "cats.txt"  # There is a file by this name, but for locale esES
    locale = "deDE"
    for fi in output_dir.glob("*"):
        fi.unlink(missing_ok=True)


    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), "-f", file_to_extract, str(test_file), "--locale", locale],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    expected_stdout_output = set()
    expected_stderr_output = {
        "[!] Failed: File doesn't exist: " + file_to_extract,
    }

    stdout_output_lines = set(result.stdout.splitlines())
    stderr_output_lines = set(result.stderr.splitlines())

    output_file = output_dir / file_to_extract

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert stdout_output_lines == expected_stdout_output, f"Unexpected output: {stdout_output_lines}"
    assert stderr_output_lines == expected_stderr_output, f"Unexpected output: {stderr_output_lines}"
    assert not output_file.exists(), "Output directory was not created"
