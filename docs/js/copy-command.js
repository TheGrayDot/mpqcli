(function () {
  "use strict";

  // Run after the page is fully loaded
  window.addEventListener("load", function () {
    attachCopyHandlers();
  });

  // Re-attach on mdBook page navigation (it uses a SPA-style renderer)
  document.addEventListener("mdbook-loaded", function () {
    attachCopyHandlers();
  });

  function attachCopyHandlers() {
    // mdBook renders copy buttons with the class .clip-button
    // Find all code blocks that have a copy button
    var codeBlocks = document.querySelectorAll("pre code");

    codeBlocks.forEach(function (codeEl) {
      var pre = codeEl.parentElement;
      var copyBtn = pre.querySelector(".clip-button");
      if (!copyBtn) return;

      // Detect language from class e.g. "language-bash" or "language-powershell"
      var lang = detectLanguage(codeEl);
      if (lang !== "bash" && lang !== "powershell") return;

      // Replace the default click handler with our filtered one
      var newBtn = copyBtn.cloneNode(true);
      copyBtn.parentNode.replaceChild(newBtn, copyBtn);

      newBtn.addEventListener("click", function (e) {
        e.stopPropagation();
        var text = filterCommands(codeEl.innerText, lang);
        navigator.clipboard.writeText(text).then(function () {
          // Brief visual feedback - reuse mdBook's own checkmark if present,
          // otherwise flash the button title
          newBtn.classList.add("clip-button--success");
          setTimeout(function () {
            newBtn.classList.remove("clip-button--success");
          }, 1500);
        });
      });
    });
  }

  function detectLanguage(codeEl) {
    var classes = codeEl.className.split(" ");
    for (var i = 0; i < classes.length; i++) {
      if (classes[i].startsWith("language-")) {
        return classes[i].replace("language-", "").toLowerCase();
      }
    }
    return null;
  }

  function filterCommands(rawText, lang) {
    var lines = rawText.split("\n");
    var result = [];
    var continuation = false;

    for (var i = 0; i < lines.length; i++) {
      var line = lines[i];

      if (lang === "bash") {
        if (continuation) {
          // This line is a continuation of the previous command - include it
          result.push(line);
          // Check if this line itself continues further
          continuation = line.trimEnd().endsWith("\\");

        } else if (line.startsWith("$ ")) {
          // Bash prompt line - strip the prompt and include
          var stripped = line.slice(2);
          result.push(stripped);
          continuation = stripped.trimEnd().endsWith("\\");

        } else if (line.startsWith("> ")) {
          // Bash secondary prompt (e.g. after a heredoc or multiline)
          result.push(line.slice(2));
          continuation = false;

        } else {
          // Output line - skip
          continuation = false;
        }

      } else if (lang === "powershell") {
        if (continuation) {
          // Continuation line after backtick - include it
          result.push(line);
          continuation = line.trimEnd().endsWith("`");

        } else if (line.startsWith("PS> ")) {
          // PowerShell prompt line - strip the prompt and include
          var stripped = line.slice(4);
          result.push(stripped);
          continuation = stripped.trimEnd().endsWith("`");

        } else {
          // Output line - skip
          continuation = false;
        }
      }
    }

    // Join and trim trailing newline
    return result.join("\n").trimEnd();
  }
})();
