import subprocess
from pathlib import Path

def test_list(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "data\\files\\cats.txt",
        "data\\files\\dogs.txt",
        "(listfile)",
    }

    result = subprocess.run(
        [str(binary_path), "list", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    output_lines = set(result.stdout.splitlines())
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
