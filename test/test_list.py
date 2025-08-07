import subprocess
from pathlib import Path


def test_list_mpq_with_output_v1(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "cats.txt",
        "dogs.txt",
        "bytes",
    }

    result = subprocess.run(
        [str(binary_path), "list", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_list_mpq_with_detailed(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    # Update expected_out to match long listing format
    expected_output = {
        "         27  enUS                    (listfile)",
        "        148  enUS                    (attributes)",
        "         27  enUS  Jul 29 2025 14:31 dogs.txt",
        "          8  enUS  Jul 29 2025 14:31 bytes",
        "         27  enUS  Jul 29 2025 14:31 cats.txt",
    }

    # Adjust filesize for Windows
    if platform.system() == "Windows":
        expected_output.remove("         27  enUS  Jul 29 2025 14:31 dogs.txt")
        expected_output.add("         28  enUS  Jul 29 2025 14:31 dogs.txt")
        expected_output.remove("         27  enUS  Jul 29 2025 14:31 cats.txt")
        expected_output.add("         28  enUS  Jul 29 2025 14:31 cats.txt")

    result = subprocess.run(
        [str(binary_path), "list", "-a", "-d", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_list_mpq_with_weak_signature(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_weak_signature.mpq"

    expected_output = {
        "cats.txt",
        "dogs.txt",
        "bytes",
        "(listfile)",  # Only included as "-a" specified
        "(signature)",  # Only included as "-a" specified
        "(attributes)",  # Only included as "-a" specified
    }

    result = subprocess.run(
        [str(binary_path), "list", "-a", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
