# The command window

One dialog runs every sequence of commands the tool issues, streaming a merged stdout and stderr into a read-only log. The output is split into lines as bytes and each line is decoded on its own, as UTF-8 unless it is not valid UTF-8, in which case it is read in the system's code page: on Windows, MSBuild and CMake write UTF-8 while some older tools still write the code page, and splitting first keeps a character that arrives across two reads in one piece.

Its colors are decided by what each line looks like, not by ANSI codes. The tools turn their own color off when they write to a pipe rather than to a terminal, so forcing it on would mean a switch per tool and a parser for the escape sequences; recognizing `error:`, `CMake Warning`, make's percentages and haxe's diagnostics covers all three tools with one set of rules. The colors come from the current palette, since a red dark enough to read on white is too dark to read on near-black.

The log is written with a text cursor rather than as HTML, because HTML would have to be escaped and compiler output is full of `<` and `&`.

A read of the pipe ends wherever it ran dry rather than at a line break, so the tail of a chunk is held back until the rest of its line arrives. Without that, coloring per line would split one line into two of different colors.

The activity the dialog reports is set by whoever opens it, so the build window says `Compiling` while the one running `git` does not claim to be compiling. Beside it runs the time since the window opened, which becomes `Completed in` or `Failed after` that time once the commands end. It sits in a label of its own, wide enough for minutes and seconds, so neither the dots nor the clock nudge the other.
