#pragma once
#include <Includes/Includes.hpp>

using namespace std;

namespace Gui {

	void Rendering( );
	void ShutdownStreamProofForUnload();
	void ApplyStreamProofAffinity(bool enable);
	inline DWORD ImGuiWindowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

}
