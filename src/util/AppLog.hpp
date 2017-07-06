#pragma once

#include "imgui.h"
#include <stdio.h>
#include <cstdarg>
#include <vector>
#include <string>
#include <numeric>


struct LOG_LEVEL {
	bool showPrefixInStdOut;
	string prefix;
	ImColor color;

	// Equality operator
	bool operator == (const LOG_LEVEL &Other) const
	{
		return(this->showPrefixInStdOut == Other.showPrefixInStdOut
			&& this->prefix == Other.prefix
			&& this->color == Other.color);
	}
};

const LOG_LEVEL LVL_INFO {
	false,
	string("INFO"),
	ImColor(255, 255, 255)
};

const LOG_LEVEL LVL_WARNING {
	false,
	string("WARNING"),
	ImColor(255, 255, 0)
};

const LOG_LEVEL LVL_ERROR {
	false,
	string("ERROR"),
	ImColor(255, 0, 0)
};

const LOG_LEVEL LVL_DEBUG {
	false,
	string("DEBUG"),
	ImColor(0, 255, 0)
};

struct LogLine {
	std::string text;
	LOG_LEVEL level;

	LogLine(std::string text, LOG_LEVEL level = LVL_INFO) {
		this->text = text;
		this->level = level;
	}
};

struct AppLog {
public:
	std::vector<LogLine> lines;

	void Clear() {
		lines.clear();
		indents.clear();
		filteredLines.clear();
	}

	// Add a log with the current log level (INFO by default).
	void AddLog(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(standardLogLevel, fmt, args);

		va_end(args);
	}

	// Add a log at the INFO level.
	void LogInfo(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_INFO, fmt, args);

		va_end(args);
	}

	// Add a log at the WARNING level.
	void LogWarning(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_WARNING, fmt, args);

		va_end(args);
	}

	// Add a log at the ERROR level.
	void LogError(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_ERROR, fmt, args);

		va_end(args);
	}

	// Add a log at the DEBUG level.
	void LogDebug(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_DEBUG, fmt, args);

		va_end(args);
	}

	// Imgui hook for drawing the window representing this log
	void Draw(const char* title, bool* p_opened = NULL) {
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);

		ImGui::Begin(title, p_opened);
		{
			if (ImGui::Button("Clear")) Clear();
			ImGui::SameLine();
			ImGui::Text("    ");
			ImGui::SameLine();

			ImGui::Text("Filter: ");
			ImGui::SameLine();
			if (ToggleButton("Info", filterKeepInfo, ImColor(255, 255, 255), ImColor(128, 128, 128), ImColor(0, 0, 0))) ToggleInfo();
			ImGui::SameLine();
			if (ToggleButton("Warning", filterKeepWarnings, LVL_WARNING.color, GetScaledColor(LVL_WARNING.color, 0.75), ImColor(0, 0, 0))) ToggleWarnings();
			ImGui::SameLine();
			if (ToggleButton("Error", filterKeepErrors, LVL_ERROR.color, GetScaledColor(LVL_ERROR.color, 0.75), ImColor(0, 0, 0))) ToggleErrors();
			ImGui::SameLine();
			if (ToggleButton("Debug", filterKeepDebug, LVL_DEBUG.color, GetScaledColor(LVL_DEBUG.color, 0.75), ImColor(0, 0, 0))) ToggleDebug();

			ImGui::Separator();

			//RefreshFilters();

			ImGui::BeginChild("scrolling");
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

			// If marked to scroll to the bottom, scroll all the way down before clipping
			if (scrollToBottom)
				ImGui::SetScrollHere(1.0f);
			scrollToBottom = false;

			// Show just the elements that would appear within the bounds of the scroll area
			ImGuiListClipper clipper(filteredLines.size(), ImGui::GetTextLineHeightWithSpacing());
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				// Draw each line
				LogLine currentLine = filteredLines[i];

				if (!FilterLine(currentLine)) {
					continue;
				}
				ImGui::TextColored(currentLine.level.color, "%s", currentLine.text.c_str());
			}
			clipper.End();

			ImGui::PopStyleVar();
			ImGui::EndChild();
		}
		ImGui::End();
	}

	// Adds `indent` as an indentation level.
	void PushIndent(string indent) {
		this->indents.push_back(indent);
	}

	// Adds the given number of spaces as a new indentation level.
	void PushIndent(int spaces = 3) {
		this->indents.push_back(string(spaces, ' '));
	}

	// Removes `levels` indentation levels.
	void PopIndent(int levels = 1) {
		for (int i = 0; i < levels; i++) {
			if(this->indents.size() > 0) this->indents.pop_back();
		}
	}

	// Removes all indentation levels.
	void ClearIndent() {
		this->indents.clear();
	}

	// Sets the log level used by AddLog.
	void SetDefaultLogLevel(LOG_LEVEL level) {
		this->standardLogLevel = level;
	}

private:
	bool scrollToBottom = false; // When set to true, the GUI will scroll to the bottom and unset this variable.
	LOG_LEVEL standardLogLevel = LVL_INFO; // The log level used by AddLog.

	// Filtering bools for all log levels.
	bool filterKeepInfo = true;
	bool filterKeepWarnings = true;
	bool filterKeepErrors = true;
	bool filterKeepDebug = true;

	// Holds the log lines that will be displayed by the Imgui window. Gets rebuilt when any of the filters change.
	std::vector<LogLine> filteredLines;

	// Concatenated and placed at the beginning of each log line when it's added.
	std::vector<string> indents;

	// C-style add log
	void Log(LOG_LEVEL level, const char* fmt, va_list args) {

		string indentsAccumulated = accumulate(indents.begin(), indents.end(), string(""));
		string line = indentsAccumulated + vformat(fmt, args);

		// Print line to stdout
		string stdoutLine = line;
		if (level.showPrefixInStdOut) {
			stdoutLine = string("[" + level.prefix + "]") + line;
		}
		std::vprintf(stdoutLine.c_str(), args);

		// Push line to vector
		LogLine logLine = LogLine(line, level);
		lines.push_back(logLine);
		// If it passes through the filter, append it immediately
		if (FilterLine(logLine)) {
			filteredLines.push_back(logLine);
		}
		scrollToBottom = true;
	}

	// Returns true if the line shouldn't be filtered out for display in the Imgui window.
	bool FilterLine(LogLine line) {
		if (line.level == LVL_INFO && !filterKeepInfo) {
			return false;
		}
		else if (line.level == LVL_WARNING && !filterKeepWarnings) {
			return false;
		}
		else if (line.level == LVL_ERROR && !filterKeepErrors) {
			return false;
		}
		else if (line.level == LVL_DEBUG && !filterKeepDebug) {
			return false;
		}

		return true;

	}

	// Rebuilds filteredLines.
	void RefreshFilters() {
		if (filterKeepInfo && filterKeepWarnings && filterKeepErrors && filterKeepDebug) {
			filteredLines = lines;
			return;
		}

		// Rebuild the filtered lines list
		filteredLines.clear();
		for (LogLine line : lines) {
			if (FilterLine(line)) {
				filteredLines.push_back(line);
			}
		}

	}

	// Handler for info filter button press
	void ToggleInfo() {
		filterKeepInfo = !filterKeepInfo;
		RefreshFilters();
	}

	// Handler for warning filter button press
	void ToggleWarnings() {
		filterKeepWarnings = !filterKeepWarnings;
		RefreshFilters();
	}

	// Handler for error filter button press
	void ToggleErrors() {
		filterKeepErrors = !filterKeepErrors;
		RefreshFilters();
	}

	// Handler for debug filter button press
	void ToggleDebug() {
		filterKeepDebug = !filterKeepDebug;
		RefreshFilters();
	}

	// Convenience method for making the stylized filter buttons in the Imgui window.
	bool ToggleButton(const char* text, bool enabled, ImColor backgroundColor, ImColor hoverColor, ImColor textColor) {

		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		if(!enabled) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		}

		ImGui::PushStyleColor(ImGuiCol_Button, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, backgroundColor);

		bool result = ImGui::Button(text);

		ImGui::PopStyleColor(4);

		if (!enabled) {
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}

		return result;

	}

	// Scales an ImGui color by the given scale.
	ImColor GetScaledColor(ImColor color, float scale) {
		return ImColor(color.Value.x * scale, color.Value.y * scale, color.Value.z * scale);
	}

	// Turns printf style commands into a std::string.
	// https://stackoverflow.com/a/69911
	std::string vformat(const char *fmt, va_list ap)
	{
		const size_t baseSize = 1024;

		// Allocate a buffer on the stack that's big enough for us almost
		// all the time.
		size_t size = baseSize;
		char buf[baseSize];

		// Try to vsnprintf into our buffer.
		va_list apcopy;
		va_copy(apcopy, ap);
		int needed = vsnprintf(&buf[0], size, fmt, ap);
		// NB. On Windows, vsnprintf returns -1 if the string didn't fit the
		// buffer.  On Linux & OSX, it returns the length it would have needed.

		if (needed <= size && needed >= 0) {
			// It fit fine the first time, we're done.
			return std::string(&buf[0]);
		}
		else {
			// vsnprintf reported that it wanted to write more characters
			// than we allotted.  So do a malloc of the right size and try again.
			// This doesn't happen very often if we chose our initial size
			// well.
			std::vector <char> buf;
			size = needed;
			buf.resize(size);
			needed = vsnprintf(&buf[0], size, fmt, apcopy);
			return std::string(&buf[0]);
		}
	}

};
