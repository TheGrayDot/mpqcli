import subprocess
from pathlib import Path


"""

https://archive.org/download/World_of_Warcraft_Client_and_Installation_Archive/Patches/1.x/WoW-1.9.3.5059-to-1.9.4.5086-enUS.patch.exe

https://archive.org/download/World_of_Warcraft_Client_and_Installation_Archive/Patches/2.x/WoW-2.2.2.7318-to-0.2.3.7344-enUS-patch.exe
"""

def test_info_v1(binary_path):
    script_dir = Path(__file__).parent
    test_file = script_dir / "data" / "mpq_with_output_v1.mpq"

    expected_output = {
        "Format version: 1",
        "Archive size: 370",
        "Header offset: 0",
        "Header size: 32",
        "File count: 4",
        "Signature type: None",
    }

    result = subprocess.run(
        [str(binary_path), "info", str(test_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    output_lines = set(result.stdout.splitlines())

    assert output_lines == expected_output, f"Unexpected output: {output_lines}"
