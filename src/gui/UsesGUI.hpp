//
//  UsesGUI.h
//  FaceOff
//
//  Created by Jonathan Cole on 10/22/15.
//
//

#ifndef UsesGUI_h
#define UsesGUI_h

#include "GUIHandler.hpp"
#include "imgui.h"
#include <stdio.h>

class UsesGUI {
public:
    //GUIHandler& GetGUI(){
    //    return GUIHandler::GetInstance(); 
    //}
    
    virtual void DrawGUI() = 0;
    
    bool showGUI = false;

	void ToggleGUIEnabled() {
		showGUI = !showGUI;
	}
    
    static void ShowHelpMarker(const char* desc)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", desc);
    }
    
    
};


#endif /* UsesGUI_h */
