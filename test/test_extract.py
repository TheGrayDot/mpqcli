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
    }

    result = subprocess.run(
        [str(binary_path), "extract", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    # Create expected_lines set based on expected output with prefix
    expected_lines = {f"[+] Extracted: {line}" for line in expected_output}

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
    }

    result = subprocess.run(
        [str(binary_path), "extract", "-o", str(output_dir), str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    # Create expected_lines set based on expected output with prefix
    expected_lines = {f"[+] Extracted: {line}" for line in expected_output}

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
    expected_lines = {f"[+] Extracted: {line}" for line in expected_output}

    # Create output_file path without suffix (default extract behavior is MPQ without extension)
    output_file = output_dir.with_suffix("")

    # Create output_files set based on directory contents (not full path)
    output_files = set(fi.name for fi in output_file.glob("*"))

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_lines, f"Unexpected output: {output_lines}"
    assert output_file.exists(), "Output directory was not created"
    assert output_files == expected_output, f"Unexpected files: {output_files}"
