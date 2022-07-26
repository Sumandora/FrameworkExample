#ifndef FEATURES_LEGIT_BHOP
#define FEATURES_LEGIT_BHOP

#include "../../SDK/CUserCmd.hpp"

namespace Features {
	namespace Legit {
		namespace Bhop {
			// Settings
			extern bool enabled;

			void CreateMove(CUserCmd* cmd);

			// GUI
			void SetupGUI();
		}
	}
}

#endif
