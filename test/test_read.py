import subprocess
from pathlib import Path


def test_read_mpq_v1_plain(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "This is a file about cats.",
    }

    result = subprocess.run(
        [str(binary_path), "read", "cats.txt", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # Remove empty lines from output, Windows adds an extra empty line
    output_lines = set(line for line in result.stdout.splitlines() if line.strip() != "")

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_read_mpq_v1_hex(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "\x00\x01\x02\x03\x04\x05\x06\x07",
    }

    result = subprocess.run(
        [str(binary_path), "read", "bytes", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
