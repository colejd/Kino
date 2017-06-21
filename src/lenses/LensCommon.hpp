#pragma once

//
//  LensCommon.hpp
//  FaceOff
//
//  Created by Jonathan Cole on 11/3/15.
//
//

class LensCommon {
public:
	virtual ~LensCommon() {}

    void Enable(){
        enabled = true;
    }
    void Disable(){
        enabled = false;
    }
    const bool IsEnabled(){
        return enabled;
    }
    void ToggleEnabled(){
        enabled = !enabled;
    }
    bool enabled = false;
    
protected:
    
private:
    
    
};
