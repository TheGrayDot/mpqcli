import subprocess
from pathlib import Path


def test_create_mpq_versions(binary_path, test_files):
    """
    Test MPQ archive creation with different MPQ versions.

    This test checks:
    - MPQ archive creation with version 1 and version 2.
    - If the MPQ archive is created in the correct directory.
    - If the MPQ archive is not empty.
    """
    _ = test_files
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


def test_create_mpq_with_output(binary_path, test_files):
    """
    Test MPQ archive creation with output file argument.

    This test checks:
    - If the MPQ archive is created in the correct directory.
    - If the MPQ archive is not empty.
    """
    _ = test_files
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

        assert output_file.exists(), f"MPQ file was not created"

        assert output_file.stat().st_size > 0, f"MPQ file is empty"


def test_create_mpq_already_exists(binary_path, test_files):
    """
    Test MPQ file creation with an existing output file.

    This test checks:
    - If the MPQ archive is not created (as archive exists already).
    """
    _ = test_files
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
