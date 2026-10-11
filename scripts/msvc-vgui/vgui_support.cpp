/* Minimal VGUI support API.
 *
 * HUD init calls VGui_GetPanel() and then Panel methods on the pointer.
 * Without InitAPI the engine returns NULL and the original client faults.
 * The panel is created on the first query, which is after client.dll has
 * finished its static constructors. Building it earlier makes those
 * constructors see an App and take a different path.
 */
#include "vgui_api.h"
#include "app.h"
#include "panel.h"

namespace vgui
{
class HudRootApp : public App
{
public:
	HudRootApp() : App(true) {}
	void main(int, char **) override {}
};
}

static vgui::Panel *root;

static void *GetPanel(void)
{
	if (!root) {
		if (!vgui::App::getInstance())
			new vgui::HudRootApp();
		root = new vgui::Panel(0, 0, 640, 480);
	}
	return root;
}

extern "C" void InitAPI(vguiapi_t *api)
{
	api->GetPanel = GetPanel;
}
