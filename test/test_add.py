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
        "[+] Adding file: test.txt",
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


def test_add_file_with_nameinarchive_and_dirinarchive_to_mpq_archive(binary_path, generate_test_files):
    """
    Test MPQ file addition with name-in-archive and dir-in-archive arguments.
    This test checks:
    - If the application correctly gives an error when using both name-in-archive and dir-in-archive arguments.
    - If the application correctly handles adding a file to an MPQ archive with the name-in-archive argument.
    - If the application correctly handles adding a file to an MPQ archive with the dir-in-archive argument.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)

    # Create new test files on the fly
    test_file0 = script_dir / "data" / "test0.txt"
    test_file0.write_text("This is a test file for MPQ addition.")
    test_file1 = script_dir / "data" / "test1.txt"
    test_file1.write_text("This is a another test file for MPQ addition.")


    result = subprocess.run(
        [str(binary_path), "add", str(test_file0), str(target_file), "--dir-in-archive", "directory", "--name-in-archive", "important.txt"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 1, f"mpqcli failed with error: {result.stderr}"

    result = subprocess.run(
        [str(binary_path), "add", str(test_file0), str(target_file), "--dir-in-archive", "directory"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    result = subprocess.run(
        [str(binary_path), "add", str(test_file1), str(target_file), "--name-in-archive", "important\\message.txt"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    result = subprocess.run(
        [str(binary_path), "list", str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())

    expected_output = {
        # Files from before:
        "cats.txt",
        "dogs.txt",
        "bytes",
        # Files added in this test:
        "directory\\test0.txt",
        "important\\message.txt",
    }
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_output, f"Unexpected output: {output_lines}"


def test_add_existing_file_without_overwrite_should_fail(binary_path, generate_test_files):
    """
    Test adding existing files to MPQ archive without the overwrite flag.
    This test checks:
    - If the application correctly prints an error when adding an existing file to an MPQ archive without the overwrite flag.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)


    # Verify that existing file has the expected content before attempting to add a new file with different content
    expected_content = {"This is a file about cats."}
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", expected_content)


    # Create new test files on the fly
    test_file = script_dir / "data" / "cats.txt"
    test_file.write_text("Attempting to make this file about dogs.")


    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    assert output_lines == set(), f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = {
        "[!] File already exists in MPQ archive: cats.txt - Skipping...",
    }
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"

    # Verify that the file content is unchanged
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", expected_content)


def test_add_existing_file_with_overwrite_should_succeed(binary_path, generate_test_files):
    """
    Test adding existing files to MPQ archive with the overwrite flag.
    This test checks:
    - If the application correctly overwrites an existing file to an MPQ archive with the overwrite flag set.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)


    # Verify that file has the expected content before overwriting
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", { "This is a file about cats." })


    # Create new test files on the fly
    test_file = script_dir / "data" / "cats.txt"
    test_file.write_text("This file is suddenly about dogs.")

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file), "--overwrite"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[+] File already exists in MPQ archive: cats.txt - Overwriting...",
        "[+] Adding file: cats.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


    # Verify that file has the expected content after overwriting
    verify_file_in_mpq_has_content(binary_path, target_file, "cats.txt", { "This file is suddenly about dogs." })


def test_add_nonexisting_file_with_overwrite_should_succeed(binary_path, generate_test_files):
    """
    Test adding a non-existing file to MPQ archive with the overwrite flag.
    This test checks:
    - If the application correctly adds a non-existing file to an MPQ archive with the overwrite flag set.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Start by creating an MPQ archive for this test
    create_mpq_archive_for_test(binary_path, script_dir)


    # Create new test files on the fly
    test_file = script_dir / "data" / "test.txt"
    test_file.write_text("This file is newly added.")

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file), "--overwrite"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    output_lines = set(result.stdout.splitlines())
    expected_stdout_output = {
        "[+] Adding file: test.txt",
    }
    assert output_lines == expected_stdout_output, f"Unexpected output: {output_lines}"

    output_lines = set(result.stderr.splitlines())
    expected_stderr_output = set()
    assert output_lines == expected_stderr_output, f"Unexpected output: {output_lines}"

    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"


    # Verify that file has the expected content
    verify_file_in_mpq_has_content(binary_path, target_file, "test.txt", { "This file is newly added." })


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
        "[+] Adding file for locale esES: cats.txt",
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


def test_add_file_with_game_profile(binary_path, generate_test_files):
    """
    Test adding a file to an MPQ archive with different game profiles.

    This test checks:
    - If files can be added with various game profiles.
    - If the game profile is accepted and applied.
    - If the correct compression flags are applied to added files.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Test profiles with their expected flags for .txt files
    test_cases = [
        ("generic", "ce"),      # Generic: compressed + encrypted
        ("diablo1", "ie"),      # Diablo1: imploded + encrypted
        ("starcraft1", "ce2"),  # StarCraft: compressed + encrypted + key v2
        ("wow1", "c"),          # WoW 1.x: compressed
        ("wow2", "cr"),         # WoW 2.x: compressed + sector CRC
        ("starcraft2", "c"),    # StarCraft2: compressed (small files use single unit)
        ("diablo3", "c"),       # Diablo3: compressed
    ]

    for profile, expected_flags in test_cases:
        # Create a fresh MPQ archive for each test
        create_mpq_archive_for_test(binary_path, script_dir)

        # Create a test file
        test_file = script_dir / "data" / f"test_{profile}.txt"
        test_file.write_text(f"Test file for {profile} profile.")

        result = subprocess.run(
            [str(binary_path), "add", str(test_file), str(target_file), "--game", profile],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode == 0, f"mpqcli failed for profile {profile}: {result.stderr}"
        assert f"Using game profile: {profile}" in result.stdout, f"Game profile message not found for {profile}"
        assert f"Adding file: test_{profile}.txt" in result.stdout

        # Verify compression flags on the added file
        list_result = subprocess.run(
            [str(binary_path), "list", str(target_file), "-d", "-p", "flags"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        assert list_result.returncode == 0, f"Failed to list files for {profile}"

        # Check that the added file has the expected flags
        found_test_file = False
        for line in list_result.stdout.splitlines():
            if f"test_{profile}.txt" in line:
                found_test_file = True
                flags = line.split()[0]  # Extract flags
                # Check that expected flags are present in the actual flags
                for flag in expected_flags:
                    assert flag in flags, f"Profile {profile}: expected flag '{flag}' in '{flags}' for added file"

        assert found_test_file, f"Profile {profile}: added file not found in archive"


def test_add_file_with_invalid_game_profile(binary_path, generate_test_files):
    """
    Test adding a file with an invalid game profile.

    This test checks:
    - If the application exits correctly when an invalid game profile is provided.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # Create a fresh MPQ archive
    create_mpq_archive_for_test(binary_path, script_dir)

    # Create a test file
    test_file = script_dir / "data" / "test_invalid.txt"
    test_file.write_text("Test file for invalid profile.")

    result = subprocess.run(
        [str(binary_path), "add", str(test_file), str(target_file), "-g", "invalid_profile"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    assert result.returncode != 0, "mpqcli should have failed with invalid game profile"


def test_add_file_with_all_game_profiles(binary_path, generate_test_files):
    """
    Test adding files with all available game profiles.

    This test checks:
    - If all game profiles work with the add command.
    - If files are actually added to the archive with compression applied.
    """
    _ = generate_test_files
    script_dir = Path(__file__).parent
    target_file = script_dir / "data" / "files.mpq"

    # All profiles should be accepted
    all_profiles = [
        "generic", "diablo1", "lordsofmagic", "starcraft1", "warcraft2", "diablo2",
        "warcraft3", "warcraft3-map", "wow1", "wow2", "wow3", "wow4", "wow5",
        "starcraft2", "diablo3"
    ]

    for profile in all_profiles:
        # Create a fresh MPQ archive for each test
        create_mpq_archive_for_test(binary_path, script_dir)

        # Create a test file
        test_file = script_dir / "data" / f"test_all_{profile}.txt"
        test_file.write_text(f"Test file for {profile} profile.")

        result = subprocess.run(
            [str(binary_path), "add", str(test_file), str(target_file), "-g", profile],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode == 0, f"mpqcli failed for profile {profile}: {result.stderr}"
        assert f"Using game profile: {profile}" in result.stdout, f"Game profile message not found for {profile}"

        # Verify the file was actually added
        list_result = subprocess.run(
            [str(binary_path), "list", str(target_file)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        assert list_result.returncode == 0, f"Failed to list files for {profile}"
        assert f"test_all_{profile}.txt" in list_result.stdout, f"Profile {profile}: added file not found in archive"

        # Verify that some compression flag is set (at least 'c' for compressed or 'i' for imploded)
        flags_result = subprocess.run(
            [str(binary_path), "list", str(target_file), "-d", "-p", "flags"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        assert flags_result.returncode == 0, f"Failed to get flags for {profile}"

        found_with_compression = False
        for line in flags_result.stdout.splitlines():
            if f"test_all_{profile}.txt" in line:
                flags = line.split()[0]
                # Check that either compressed or imploded flag is present
                if 'c' in flags or 'i' in flags:
                    found_with_compression = True
                break

        assert found_with_compression, f"Profile {profile}: no compression flag found on added file"


def create_mpq_archive_for_test(binary_path, script_dir):
    target_dir = script_dir / "data" / "files"
    target_file = target_dir.with_suffix(".mpq")
    # Remove the target file if it exists
    # Testing creation when file exists is handled:
    # test_create_mpq_already_exists
    target_file.unlink(missing_ok=True)
    result = subprocess.run(
        [str(binary_path), "create", "--version", "1", str(target_dir)],
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

def verify_file_in_mpq_has_content(binary_path, mpq_archive, file_name, expected_content):
    result = subprocess.run(
        [str(binary_path), "read", file_name, str(mpq_archive)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # Remove empty lines from output, Windows adds an extra empty line
    output_lines = set(line for line in result.stdout.splitlines() if line.strip() != "")
    assert result.returncode == 0, f"mpqcli failed with error: {result.stderr}"
    assert output_lines == expected_content, f"Unexpected output: {output_lines}"
