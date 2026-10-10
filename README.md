# Industrial Lab — Easy GitHub Upload

This is the **basic, untested kick synth starter**, not the complete industrial plugin.

## Upload (no folders needed)
1. Create or open your GitHub repository, Code tab.
2. Upload **CMakeLists.txt** and **Plugin.cpp** to the top level of the repository. You can also upload README.md.
3. IMPORTANT: **Do not upload build-windows.yml to the top level.** Instead, click Add file > Create new file, type `.github/workflows/build-windows.yml` as the filename, then copy and paste the contents of the `build-windows.yml` included here. Click Commit changes.
4. Open Actions > Build Industrial Lab (Windows VST3) > Run workflow, or open the automatic push run.
5. On a successful run, download the artifact under the run's Artifacts section.

The included build workflow downloads JUCE while running, so you don't need JUCE installed locally. Build success has **not** yet been verified.
