//
//  ModuleCommon.hpp
//  FaceOff
//
//  Created by Jonathan Cole on 11/3/15.
//
//

#ifndef ModuleCommon_h
#define ModuleCommon_h

class ModuleCommon {
public:
	virtual ~ModuleCommon() {}

	void Enable() {
		enabled = true;
	}
	void Disable() {
		enabled = false;
	}
	const bool IsEnabled() {
		return enabled;
	}
	void ToggleEnabled() {
		enabled = !enabled;
	}
	bool enabled = false;

protected:

private:


};


#endif /* ModuleCommon_h */
