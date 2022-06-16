#pragma once

void drawmenu() {
	ImGui::Begin("##menu", nullptr);
	ImGuiWindow& window = *ImGui::GetCurrentWindow();

	ImGui::Text("Hello world!");

	window.DrawList->PushClipRectFullScreen();
}