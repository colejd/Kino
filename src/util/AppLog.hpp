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

	void AddLog(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(standardLogLevel, fmt, args);

		va_end(args);
	}

	void LogInfo(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_INFO, fmt, args);

		va_end(args);
	}

	void LogWarning(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_WARNING, fmt, args);

		va_end(args);
	}

	void LogError(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_ERROR, fmt, args);

		va_end(args);
	}

	void LogDebug(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		Log(LVL_DEBUG, fmt, args);

		va_end(args);
	}

	void Draw(const char* title, bool* p_opened = NULL) {
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);

		ImGui::Begin(title, p_opened);
		{
			if (ImGui::Button("Clear")) Clear();
			//ImGui::SameLine();

			ImGui::Text("Filter: ");
			ImGui::SameLine();
			if (ToggleButton("Info", showInfo, ImColor(255, 255, 255), ImColor(128, 128, 128), ImColor(0, 0, 0))) ToggleInfo();
			ImGui::SameLine();
			if (ToggleButton("Warning", showWarning, LVL_WARNING.color, GetScaledColor(LVL_WARNING.color, 0.75), ImColor(0, 0, 0))) ToggleWarnings();
			ImGui::SameLine();
			if (ToggleButton("Error", showError, LVL_ERROR.color, GetScaledColor(LVL_ERROR.color, 0.75), ImColor(0, 0, 0))) ToggleErrors();
			ImGui::SameLine();
			if (ToggleButton("Debug", showDebug, LVL_DEBUG.color, GetScaledColor(LVL_DEBUG.color, 0.75), ImColor(0, 0, 0))) ToggleDebug();

			//RefreshFilters();

			ImGui::BeginChild("scrolling", ImVec2(0, 0), true);
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


	void PushIndent(string indent) {
		this->indents.push_back(indent);
	}

	void PushIndent(int spaces = 3) {
		this->indents.push_back(string(spaces, ' '));
	}

	void PopIndent() {
		this->indents.pop_back();
	}

	void ClearIndent() {
		this->indents.clear();
	}

	void SetDefaultLogLevel(LOG_LEVEL level) {
		this->standardLogLevel = level;
	}

private:
	bool scrollToBottom = false;
	LOG_LEVEL standardLogLevel = LVL_INFO;

	bool showInfo = true;
	bool showWarning = true;
	bool showError = true;
	bool showDebug = true;

	std::vector<LogLine> filteredLines;
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

	bool FilterLine(LogLine line) {
		if (line.level == LVL_INFO && !showInfo) {
			return false;
		}
		else if (line.level == LVL_WARNING && !showWarning) {
			return false;
		}
		else if (line.level == LVL_ERROR && !showError) {
			return false;
		}
		else if (line.level == LVL_DEBUG && !showDebug) {
			return false;
		}

		return true;

	}

	void RefreshFilters() {
		if (showInfo && showWarning && showError && showDebug) {
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

	void ToggleInfo() {
		showInfo = !showInfo;
		RefreshFilters();
	}

	void ToggleWarnings() {
		showWarning = !showWarning;
		RefreshFilters();
	}

	void ToggleErrors() {
		showError = !showError;
		RefreshFilters();
	}

	void ToggleDebug() {
		showDebug = !showDebug;
		RefreshFilters();
	}

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

	ImColor GetScaledColor(ImColor color, float scale) {
		return ImColor(color.Value.x * scale, color.Value.y * scale, color.Value.z * scale);
	}

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
